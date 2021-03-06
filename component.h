#pragma once

#include <memory>

namespace ecs
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
ecs::component_tag tag() const override { \
	return component::tag_t<NAME>(); \
} \
public: \
NAME clone() const { \
	return *this; \
}

}  // namespace ecs
