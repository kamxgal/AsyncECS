#include "entity.h"

namespace async_ecs
{
entity::entity(entity_id id) : mId(id), mBitflag(0)
{
}

entity::entity(const entity &other)
    : mId(other.mId)
    , mBitflag(other.mBitflag)
    , mResources(other.mResources)
{
}

bool entity::has(component_tag t) const
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() <= t) {
        return false;
    }

    return mBitflag.at(t);
}

bool entity::has(const bitflag &bf) const
{
    std::unique_lock<std::mutex> lock(mMutex);
    return mBitflag.has(bf);
}

std::vector<component_const_ptr> entity::get(const bitflag &bf) const
{
    std::vector<component_const_ptr> result;
    mMutex.lock();
    auto resources = mResources;
    mMutex.unlock();

    for (size_t i=0; i<bf.size(); ++i)
    {
        if (bf.at(i) && mBitflag.at(i)) {
            result.push_back(resources.at(i));
        }
    }
    return result;
}

bool entity::insert(component_ptr comp)
{
    entity_id myId = id();
	size_t s = mBitflag.size();
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() <= comp->tag()) {
        mBitflag.resize(comp->tag()+1);
        mResources.resize(comp->tag() +1);
    }

	size_t s2 = mBitflag.size();
	size_t s3 = mResources.size();

    if (mBitflag.at(comp->tag())) {
        return false;
    }

    mResources[comp->tag()] = comp;
    mBitflag.set(comp->tag(), true);
    return true;
}

bool entity::remove(component_tag tag)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() <= tag) {
        return false;
    }
    if (!mBitflag.at(tag)) {
        return false;
    }

    mResources[tag] = component_ptr();
    mBitflag.set(tag, false);
    return true;
}

bool entity::update(component_ptr comp)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() <= comp->tag()) {
        return false;
    }
    if (!mBitflag.at(comp->tag())) {
        return false;
    }
    if (comp->mRevision != mResources.at(comp->tag())->mRevision) {
        return false;
    }

    ++comp->mRevision;
    mResources[comp->tag()] = comp;
    return true;
}
} // namespace async_ecs
