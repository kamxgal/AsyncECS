#include "registry.h"

namespace
{
async_ecs::entity_id nextAvailableEntityId = 0;
}

namespace async_ecs
{

entity_id registry::createEntity()
{
    std::unique_lock<std::mutex> lock(mAccessMutex);
    mEntities.emplace(std::make_pair(nextAvailableEntityId,
        std::make_shared<entity>(nextAvailableEntityId)));
    return nextAvailableEntityId++;
}

bool registry::insert(entity_id id, component_ptr c)
{
    mAccessMutex.lock();
    auto iter = mEntities.find(id);
    if (iter == mEntities.end()) {
        return false;
    }

    auto e = iter->second;
    mAccessMutex.unlock();

    bool result = e->insert(c);
	if (result) {
		handleSubscription(operation_t::inserted, id, c);
	}

	return result;
}

bool registry::update(entity_id id, component_ptr c)
{
    mAccessMutex.lock();
    auto iter = mEntities.find(id);
    if (iter == mEntities.end()) {
        return false;
    }

    auto e = iter->second;
    mAccessMutex.unlock();

    bool result = e->update(c);
	if (result) {
		handleSubscription(operation_t::updated, id, c);
	}

    return result;
}

bool registry::remove(entity_id id, component_tag tag)
{
    mAccessMutex.lock();
    auto iter = mEntities.find(id);
    if (iter == mEntities.end()) {
        return false;
    }

    auto e = iter->second;
    mAccessMutex.unlock();

    return e->remove(tag);
}

bool registry::remove(entity_id id)
{
    std::unique_lock<std::mutex> lock(mAccessMutex);
    auto iter = mEntities.find(id);
    bool isRemoved = iter != mEntities.end();
    mEntities.erase(iter);
    return isRemoved;
}

void registry::addSubscription(std::shared_ptr<registry::Subscription> s)
{
    std::unique_lock<std::mutex> lock(mSubscriptionsMutex);
    mSubscriptions.emplace(std::make_pair(mNextAvailableSubscriptionId++, s));
}

void registry::handleSubscription(operation_t operation, entity_id id, component_ptr c)
{
    std::unique_lock<std::mutex> lock(mSubscriptionsMutex);
    auto copy = mSubscriptions;
    lock.unlock();

    for (auto iter = copy.begin(); iter != copy.end(); ++iter)
    {
        auto& s = iter->second;
        s->handle(operation, id, c);
    }
}

} //
