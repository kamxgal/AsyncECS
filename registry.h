#pragma once

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <variant>

#include "entity.h"
#include "view.h"

namespace async_ecs
{
struct registry
{
    entity_id createEntity();
    bool insert(entity_id, component_ptr);
    bool update(entity_id, component_ptr);
    bool remove(entity_id, component_tag);
    bool remove(entity_id);

    template<class... Ts>
    view<Ts...> select() const {
        bitflag bf;
        fillBitflag<Ts...>(bf);

        std::vector<entity_id> entities;
        std::vector<component_const_ptr> components;

        mAccessMutex.lock();
        auto copy = mEntities;
        mAccessMutex.unlock();

        for (auto iter = copy.begin(); iter != copy.end(); ++iter)
        {
            auto ptr = iter->second;
            if (!ptr->has(bf)) continue;
            entities.push_back(iter->first);
            collectData<Ts...>(ptr, components);
        }

        return view<Ts...>(std::move(entities), std::move(components));
    }

    template<class T>
    std::shared_ptr<const T> select(entity_id id) const
    {
        mAccessMutex.lock();
        auto iter = mEntities.find(id);
        if (iter == mEntities.end()) {
            mAccessMutex.unlock();
            return nullptr;
        }
        const auto e = iter->second;
        mAccessMutex.unlock();
        if (!e->has(component::tag_t<T>())) {
            return nullptr;
        }
        return std::static_pointer_cast<const T>(e->get<T>().front());
    }

    struct Subscription
    {
        virtual bool accepts(component_tag tag) const = 0;
        virtual void handle(entity_id id, component_const_ptr c) const = 0;
    };

    template<class T>
    using SubscriptionNotifFunc = std::function<void(entity_id, std::shared_ptr<const T>)>;

    template<class T>
    using PreconditionFunc = std::function<bool(entity_id, std::shared_ptr<const T>)>;

    template<class T>
    struct SubscriptionVariant : public Subscription
    {
        SubscriptionVariant(SubscriptionNotifFunc<T> cb, PreconditionFunc<T> prec)
            : callback(cb), precondition(prec)
        {}

        bool accepts(component_tag tag) const override { return tag == component::tag_t<T>(); }
        void handle(entity_id id, component_const_ptr c) const {
            if ( c->tag() != component::tag_t<T>()) {
                return;
            }

            std::shared_ptr<const T> comp = std::static_pointer_cast<const T>(c);
            if (precondition(id, comp)) {
                callback(id, comp);
            }
        }
        SubscriptionNotifFunc<T> callback;
        PreconditionFunc<T> precondition;
    };

    template<class T>
    void subscribe(SubscriptionNotifFunc<T> callback,
        PreconditionFunc<T> precondition = [](entity_id, std::shared_ptr<const T>) -> bool { return true; })
    {
        addSubscription(std::make_shared<SubscriptionVariant<T>>(callback, precondition));
    }

private:
    template<class T>
    static void fillBitflag(bitflag& bf) {
        if (bf.size() <= component::tag_t<T>()) {
            bf.resize(component::tag_t<T>() + 1);
        }
        bf.set(component::tag_t<T>(), true);
    }

    template<class T1, class T2, class... Rest>
    static void fillBitflag(bitflag& bf) {
        if (bf.size() <= component::tag_t<T1>()) {
            bf.resize(component::tag_t<T1>() + 1);
        }
        bf.set(component::tag_t<T1>(), true);
        fillBitflag<T2, Rest...>(bf);
    }

    template<class T>
    void collectData(std::shared_ptr<entity> entity,
        std::vector<component_const_ptr>& components) const
    {
        auto vec = entity->get<T>();
        components.insert(components.end(), vec.begin(), vec.end());
    }

    template<class T1, class T2, class... Rest>
    void collectData(std::shared_ptr<entity> entity,
        std::vector<component_const_ptr>& components) const
    {
        auto vec = entity->get<T1>();
        components.insert(components.end(), vec.begin(), vec.end());
        collectData<T2, Rest...>(entity, components);
    }

    void addSubscription(std::shared_ptr<Subscription> s);

    void handleSubscription(entity_id id, component_ptr c);

private:
    using subscription_id = size_t;

    subscription_id mNextAvailableSubscriptionId = 0;
    std::map<entity_id, std::shared_ptr<entity>> mEntities;
    std::map<subscription_id, std::shared_ptr<Subscription>> mSubscriptions;
    mutable std::mutex mAccessMutex;
    mutable std::mutex mSubscriptionsMutex;
};
} // namespace async_ecs
