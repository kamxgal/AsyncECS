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

#pragma once

#include <vector>
#include <mutex>
#include "bitflag.h"
#include "component.h"

namespace ecs
{
using entity_id = size_t;
struct entity
{
    entity(entity_id id);
    entity(const entity& other);

    entity_id id() const { return mId; }
    bool has(component_tag t) const;
    bool has(const bitflag& bf) const;
    std::vector<component_const_ptr> get(const bitflag& bf) const;
    bool insert(component_ptr comp);
    bool remove(component_tag tag);
    bool update(component_ptr comp);
    const bitflag& get_bitflag() { return mBitflag; }

    template<class... Ts>
    std::vector<component_const_ptr> get() const {
        std::vector<component_const_ptr> result;
        mMutex.lock();
        auto resources = mResources;
        mMutex.unlock();
        get_components<Ts...>(resources, result);
        return result;
    }

private:
    template<class T>
    static void get_components(std::vector<component_ptr> source, std::vector<component_const_ptr>& result)
    {
        result.push_back(source.at(component::tag_t<T>()));
    }

    template<class T1, class T2, class... Ts>
    static void get_components(std::vector<component_ptr> source, std::vector<component_const_ptr>& result)
    {
        result.push_back(source.at(component::tag_t<T1>()));
        get_components<T2, Ts...>(source, result);
    }

private:
    entity_id mId;
    bitflag mBitflag;
    std::vector<component_ptr> mResources;
    mutable std::mutex mMutex;
};
}
