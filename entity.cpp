#include "entity.h"

namespace async_ecs
{
entity::entity(entity_id id) : mId(id)
{
}

bool entity::has(component_tag t) const
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() < t) {
        return false;
    }

    return mBitflag.at(t);
}

bool entity::has(const bitflag &bf) const
{
    std::unique_lock<std::mutex> lock(mMutex);
    return mBitflag & bf;
}

std::vector<component_const_ptr> entity::get(const bitflag &bf) const
{
    std::vector<component_const_ptr> result;
    mMutex.lock();
    auto resources = mResources;
    mMutex.unlock();

//    auto comp = result.begin();
    for (size_t i=0; i<bf.size(); ++i)
    {
        if (bf.at(i) && mBitflag.at(i)) {
//            *comp++ = resources.at(i);
            result.push_back(resources.at(i));
        }
    }
    return result;
}

bool entity::insert(component_tag tag, component_ptr comp)
{
    entity_id myId = id();
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() <= tag) {
        mBitflag.resize(tag+1);
        mResources.resize(tag+1);
    }

    if (mBitflag.at(tag)) {
        return false;
    }

    mResources[tag] = comp;
    mBitflag.set(tag, true);
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

bool entity::update(component_tag tag, component_ptr comp)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mBitflag.size() <= tag) {
        return false;
    }
    if (!mBitflag.at(tag)) {
        return false;
    }
    if (comp->mRevision != mResources.at(tag)->mRevision) {
        return false;
    }

    ++comp->mRevision;
    mResources[tag] = comp;
}
} // namespace async_ecs
