/*
 * AsyncECS
 * Copyright (c) 2018 kamxgal Kamil Galant kamil.galant@gmail.com
 *
 * MIT licence
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "entity.h"

namespace ecs
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
    assert(bf.size() <= mBitflag.size());
    std::vector<component_const_ptr> result;
    mMutex.lock();
    auto resources = mResources;
    mMutex.unlock();

    for (size_t i=0; i<bf.size(); ++i)
    {
        if (bf.at(i)) {
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
} // namespace ecs
