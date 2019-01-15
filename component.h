#pragma once

#include <memory>

namespace async_ecs
{
struct component;
using component_tag = unsigned int;
using component_const_ptr = std::shared_ptr<const component>;
using component_ptr = std::shared_ptr<component>;

struct component
{
private:
	template<class T>
	struct RegisteredComponents {
		static bool is_registered;
		static component_tag tag;
	};

public:
	virtual ~component() = default;

	template<class T>
	static component_tag tag_t() {
		assert(RegisteredComponents<T>::is_registered);
		return RegisteredComponents<T>::tag;
	}

	template<class T>
	void register_t() {
		if (RegisteredComponents<T>::is_registered) {
			return;
		}
		RegisteredComponents<T>::is_registered = true;
		RegisteredComponents<T>::tag = ++mNextAvailableTag;
		RegisteredComponents<const T>::is_registered = true;
		RegisteredComponents<const T>::tag = mNextAvailableTag;
	}

	virtual component_tag tag() const = 0;

private:
	virtual component_ptr doClone() const = 0;

public:
	component_ptr clone() const { return doClone(); }

protected:
    void clone_private_data(component_ptr c) const;
private:
	static component_tag mNextAvailableTag;
    size_t mRevision = 0;
    friend struct entity;
};

template<class T> bool component::RegisteredComponents<T>::is_registered = false;
template<class T> component_tag component::RegisteredComponents<T>::tag = 0;

#define ECS_COMPONENT(NAME) \
NAME() { \
    component::register_t<NAME>(); \
} \
~NAME() override = default; \
async_ecs::component_tag tag() const override { \
	return component::tag_t<NAME>(); \
} \
private: \
async_ecs::component_ptr doClone() const override { \
		async_ecs::component_ptr cloned = std::make_shared<NAME>(*this); \
		async_ecs::component::clone_private_data(cloned); \
		return cloned; \
} \
public: \
std::shared_ptr<NAME> clone() const { \
	return std::static_pointer_cast<NAME>(doClone()); \
}

}  // namespace async_ecs
