#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include "component.h"

namespace async_ecs
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
