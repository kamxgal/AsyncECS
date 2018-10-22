#pragma once

#include <vector>
#include <mutex>
#include "bitflag.h"
#include "component.h"

namespace async_ecs
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
    bool insert(component_tag tag, component_ptr comp);
    bool remove(component_tag tag);
    bool update(component_tag tag, component_ptr comp);

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
        result.push_back(source.at(Tag<T>()));
    }

    template<class T1, class T2, class... Ts>
    static void get_components(std::vector<component_ptr> source, std::vector<component_const_ptr>& result)
    {
        result.push_back(source.at(Tag<T1>()));
        get_components<T2, Ts...>(source, result);
    }

private:
    entity_id mId;
    bitflag mBitflag;
    std::vector<component_ptr> mResources;
    mutable std::mutex mMutex;
};
}
