#pragma once

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <variant>

#include "entity.h"
#include "view.h"
#include "notification.h"

namespace async_ecs
{
struct registry
{
    entity_id createEntity();

    bool remove(entity_id);

    template<class T>
    bool insert(entity_id eid, T&& component) {
        std::shared_ptr<T> cptr = std::make_shared<T>(std::move(component));
        return insertComponent(eid, cptr);
    }

    template<class T>
    bool update(entity_id eid, T&& component) {
        std::shared_ptr<T> cptr = std::make_shared<T>(std::move(component));
        return updateComponent(eid, cptr);
    }

    template<class T>
    bool remove(entity_id eid) {
        bool result = remove(eid, component::tag_t<T>());
        if (result) {
            handleRemovalSubscriptions(eid, component::tag_t<T>());
        }
        return result;
    };

    template<class... Ts>
    view<Ts...> select() const {
        bitflag bf;
        fillBitflag<Ts...>(bf);

        std::vector<entity_id> entities;
        std::vector<component_const_ptr> components;

        auto clones = cloneEntities();
        for (auto iter = clones.begin(); iter != clones.end(); ++iter)
        {
            const auto& e = iter->second;
            if (!e.has(bf)) continue;
            entities.push_back(iter->first);
            collectData<Ts...>(e, components);
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
		const auto e = *iter->second;
		mAccessMutex.unlock();
		if (!e.has(component::tag_t<T>())) {
			return nullptr;
		}
		return std::static_pointer_cast<const T>(e.get<T>().front());
    }

	// synchronization should be guaranteed by a user
	// @see: finish_unsafe_update
	template<class T>
	std::shared_ptr<T> select_unsafely(entity_id id) const
	{
		return std::const_pointer_cast<T>(select<T>(id));
	}

	// should be called by a user when update granted by
	// select_unsafely method is done and the user wants to
	// notify subscribers about that
	template<class T>
	void finish_unsafe_update(entity_id id) const
	{
		mAccessMutex.lock();
		auto iter = mEntities.find(id);
		if (iter == mEntities.end()) {
			return;
		}

		auto e = iter->second;
		auto c = e->get<T>().front();
		mAccessMutex.unlock();

		if (c == nullptr) {
			return;
		}

		handleSubscriptions(operation_t::updated, id, c);
	}

    struct Subscription
    {
        virtual void handle(operation_t operation, entity_id id, component_const_ptr c) const = 0;
        virtual void handle_removal(entity_id id, component_tag tag) const = 0;
    };

    template<class T>
    using SubscriptionNotifFunc = std::function<void(const Notification<T>&)>;

    template<class T>
    using PreconditionFunc = std::function<bool(const Notification<T>&)>;

    template<class T>
    struct SubscriptionVariant : public Subscription
    {
        SubscriptionVariant(SubscriptionNotifFunc<T> cb, PreconditionFunc<T> prec)
            : callback(cb), precondition(prec)
        {}

        void handle(operation_t operation, entity_id id, component_const_ptr c) const {
            if ( c->tag() != component::tag_t<T>() ) {
                return;
            }

            Notification<T> notification {
                operation,
                id,
                std::static_pointer_cast<const T>(c)
            };

            if (precondition(notification)) {
                callback(notification);
            }
        }

        void handle_removal(entity_id id, component_tag tag) const {
            if (tag != component::tag_t<T>()) {
                return;
            }

            Notification<T> notification{
                operation_t::removed,
                id,
                nullptr
            };

            if (precondition(notification)) {
                callback(notification);
            }
        }

        SubscriptionNotifFunc<T> callback;
        PreconditionFunc<T> precondition;
    };

	using Unsubscriber = std::function<void()>;

    template<class T>
	Unsubscriber subscribe(SubscriptionNotifFunc<T> callback,
        PreconditionFunc<T> precondition = [](const Notification<T>&) -> bool { return true; })
    {
        static_assert( !std::is_same<async_ecs::entity, T>::value );
        static_assert( std::is_base_of<async_ecs::component, T>::value );
        return addSubscription(std::make_shared<SubscriptionVariant<T>>(callback, precondition));
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
    void collectData(const entity& entity,
        std::vector<component_const_ptr>& components) const
    {
        auto vec = entity.get<T>();
        components.insert(components.end(), vec.begin(), vec.end());
    }

    template<class T1, class T2, class... Rest>
    void collectData(const entity& entity,
        std::vector<component_const_ptr>& components) const
    {
        auto vec = entity.get<T1>();
        components.insert(components.end(), vec.begin(), vec.end());
        collectData<T2, Rest...>(entity, components);
    }

	std::map<entity_id, entity> cloneEntities() const;

    bool insertComponent(entity_id, component_ptr);
    bool updateComponent(entity_id, component_ptr);
	Unsubscriber addSubscription(std::shared_ptr<Subscription> s);
    void handleSubscriptions(operation_t operation, entity_id id, component_const_ptr c) const;
    void handleRemovalSubscriptions(entity_id id, component_tag tag);
    void handleSubscriptionsOnEntityRemoval(entity_id id, const bitflag& bf);

    bool remove(entity_id, component_tag);

	using subscription_id = size_t;
	void removeSubscription(subscription_id);

private:
    subscription_id mNextAvailableSubscriptionId = 0;
    std::map<entity_id, std::shared_ptr<entity>> mEntities;
    std::map<subscription_id, std::shared_ptr<Subscription>> mSubscriptions;
    mutable std::mutex mAccessMutex;
    mutable std::mutex mSubscriptionsMutex;
};
} // namespace async_ecs
