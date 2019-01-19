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
		handleSubscriptions(operation_t::inserted, id, c);
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
		handleSubscriptions(operation_t::updated, id, c);
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

void registry::removeSubscription(subscription_id id)
{
	std::unique_lock<std::mutex> lock(mSubscriptionsMutex);
	mSubscriptions.erase(id);
}

bool registry::remove(entity_id id)
{
    std::unique_lock<std::mutex> lock(mAccessMutex);
    auto iter = mEntities.find(id);
    bool isNotExisting = iter == mEntities.end();

	if (isNotExisting) {
		return false;
	}

	bitflag bf = iter->second->get_bitflag();
    mEntities.erase(iter);
	lock.unlock();

	handleSubscriptionsOnEntityRemoval(id, bf);

    return true;
}

std::map<entity_id, entity> registry::cloneEntities() const
{
	std::map<entity_id, entity> clones;
	mAccessMutex.lock();
	for (auto it = mEntities.cbegin(); it != mEntities.end(); ++it)
	{
		clones.emplace(it->first, entity(*it->second));
	}
	mAccessMutex.unlock();
	return clones;
}

registry::Unsubscriber registry::addSubscription(std::shared_ptr<registry::Subscription> s)
{
    std::unique_lock<std::mutex> lock(mSubscriptionsMutex);
	auto subscriptionId = mNextAvailableSubscriptionId++;
    auto result = mSubscriptions.emplace(std::make_pair(subscriptionId, s));
	return [subscriptionId, this]() {
		removeSubscription(subscriptionId);
	};
}

void registry::handleSubscriptions(operation_t operation, entity_id id, component_const_ptr c) const
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

void registry::handleRemovalSubscriptions(entity_id id, component_tag tag)
{
	std::unique_lock<std::mutex> lock(mSubscriptionsMutex);
	auto copy = mSubscriptions;
	lock.unlock();

	for (auto iter = copy.begin(); iter != copy.end(); ++iter)
	{
		auto& s = iter->second;
		s->handle_removal(id, tag);
	}
}

void registry::handleSubscriptionsOnEntityRemoval(entity_id id, const bitflag& bf)
{
	if (bf.enabled_flags_count() == 0) {
		return;
	}

	std::unique_lock<std::mutex> lock(mSubscriptionsMutex);
	auto copy = mSubscriptions;
	lock.unlock();

	for (size_t tag = 0; tag < bf.size(); ++tag) {
		if (!bf.at(tag)) {
			continue;
		}

		for (auto iter = copy.begin(); iter != copy.end(); ++iter)
		{
			auto& s = iter->second;
			s->handle_removal(id, tag);
		}
	}
}

} //
