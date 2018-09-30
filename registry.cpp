#include "registry.h"

namespace
{
async_ecs::entity_id nextAvailableEntityId = 0;
}

namespace async_ecs
{

entity_id registry::createEntity()
{
    std::lock_guard<std::mutex> lock(mMutex);
    mEntities.emplace(std::make_pair(nextAvailableEntityId,
        std::make_shared<entity>(nextAvailableEntityId)));
    return nextAvailableEntityId++;
}

bool registry::insert(entity_id id, component_tag tag, component_ptr c)
{
    mMutex.lock();
    auto iter = mEntities.find(id);
    if (iter == mEntities.end()) {
        return false;
    }

    std::shared_ptr<entity> e = iter->second;
    mMutex.unlock();

    return e->insert(tag, c);
}

bool registry::update(entity_id id, component_tag tag, component_ptr c)
{
    mMutex.lock();
    auto iter = mEntities.find(id);
    if (iter == mEntities.end()) {
        return false;
    }

    std::shared_ptr<entity> e = iter->second;
    mMutex.unlock();

    return e->update(tag, c);
}

bool registry::remove(entity_id id, component_tag tag)
{
    mMutex.lock();
    auto iter = mEntities.find(id);
    if (iter == mEntities.end()) {
        return false;
    }

    std::shared_ptr<entity> e = iter->second;
    mMutex.unlock();

    return e->remove(tag);
}

bool registry::remove(entity_id id)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mEntities.erase(mEntities.find(id));
}

} //
