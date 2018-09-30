#pragma once

#include <iostream>

#include <list>
#include <map>
#include <memory>
#include <mutex>

#include "entity.h"
#include "view.h"

namespace async_ecs
{
struct registry
{
    entity_id createEntity();
    bool insert(entity_id, component_tag, component_ptr);
    bool update(entity_id, component_tag, component_ptr);
    bool remove(entity_id, component_tag);
    bool remove(entity_id);

    template<class... Ts>
    view<Ts...> get() const {
        bitflag bf;
        fillBitflag<Ts...>(bf);

        std::vector<entity_id> entities;
        std::vector<component_const_ptr> components;

        mMutex.lock();
        auto copy = mEntities;
        mMutex.unlock();

        for (auto iter = copy.begin(); iter != copy.end(); ++iter)
        {
            std::shared_ptr<entity> ptr = iter->second;
            if (!ptr->has(bf)) continue;
            entities.push_back(iter->first);
            collectData<Ts...>(ptr, components);
        }

        return view<Ts...>(std::move(entities), std::move(components));
    }

private:
    template<class T>
    static void fillBitflag(bitflag& bf) {
        if (bf.size() <= Tag<T>()) {
            bf.resize(Tag<T>() + 1);
        }
        bf.set(Tag<T>(), true);
    }

    template<class T1, class T2, class... Rest>
    static void fillBitflag(bitflag& bf) {
        if (bf.size() <= Tag<T1>()) {
            bf.resize(Tag<T1>() + 1);
        }
        bf.set(Tag<T1>(), true);
        fillBitflag<T2, Rest...>(bf);
    }

    template<class T>
    void collectData(std::shared_ptr<entity>& entity,
        std::vector<component_const_ptr>& components) const
    {
        auto vec = entity->get<T>();
        components.insert(components.end(), vec.begin(), vec.end());
    }

    template<class T1, class T2, class... Rest>
    void collectData(std::shared_ptr<entity>& entity,
        std::vector<component_const_ptr>& components) const
    {
        auto vec = entity->get<T1>();
        components.insert(components.end(), vec.begin(), vec.end());
        collectData<T2, Rest...>(entity, components);
    }

private:
    std::map<entity_id, std::shared_ptr<entity>> mEntities;
    mutable std::mutex mMutex;
};
} //
