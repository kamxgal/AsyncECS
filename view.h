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
#include <map>
#include <algorithm>
#include "component.h"

namespace ecs
{

using entity_id = size_t;

template<typename... Ts>
struct view
{
    view(std::vector<entity_id>&& entities, std::vector<component_const_ptr>&& resources)
        : mEntities(std::move(entities))
        , mNumOfComponentsPerEntity(sizeof...(Ts))
        , mResources(std::move(resources))
    {
        setupComponentIndexing();
    }

    view(view&& other)
        : mEntities(std::move(other.mEntities))
        , mNumOfComponentsPerEntity(std::move(other.mNumOfComponentsPerEntity))
        , mResources(std::move(other.mResources))
    {}

    template<class T>
    std::shared_ptr<const T> select(entity_id id)
    {
        size_t entityIndex = 0;
        bool isEntityIndexFound = false;
        for (; entityIndex < mEntities.size(); ++entityIndex) {
            if (mEntities.at(entityIndex) == id) {
                isEntityIndexFound = true;
                break;
            }
        }

        if (!isEntityIndexFound) {
            return std::shared_ptr<const T>();
        }

        size_t offset = entityIndex * mNumOfComponentsPerEntity + GetComponentIndex<T>::index;
        return std::dynamic_pointer_cast<const T>(mResources[offset]);
    }


    template<class T>
    std::map<entity_id, std::shared_ptr<const T>> select(std::function<bool(std::shared_ptr<const T>)> predicate) {
        std::map<entity_id, std::shared_ptr<const T>> result;
        for (size_t entityIndex = 0; entityIndex < mEntities.size(); ++entityIndex) {
            size_t offset = entityIndex * mNumOfComponentsPerEntity + GetComponentIndex<T>::index;
            assert(offset < mResources.size());
            const auto component = std::dynamic_pointer_cast<const T>(mResources[offset]);
            if (!predicate(component)) {
                continue;
            }
            result.insert(std::make_pair(mEntities.at(entityIndex), component));
        }
        return result;
    }

    const std::vector<entity_id>& entities() { return mEntities; }

private:
    template<class T>
    struct GetComponentIndex
    {
        static int index;
    };

    static void setupComponentIndexing() {
        setupComponentIndexingImpl<Ts...>(0);
    }

    template<class T>
    static void setupComponentIndexingImpl(size_t id) {
        GetComponentIndex<T>::index = id;
    }

    template<class T1, class T2, class... Rest>
    static void setupComponentIndexingImpl(size_t id) {
        GetComponentIndex<T1>::index = id;
        setupComponentIndexingImpl<T2, Rest...>(id + 1);
    }

private:
    std::vector<entity_id> mEntities;
    size_t mNumOfComponentsPerEntity;
    std::vector<component_const_ptr> mResources;
};

template<class... Ts> template<class T> int view<Ts...>::GetComponentIndex<T>::index = 0;
} //
