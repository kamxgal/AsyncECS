#pragma once
#include <component.h>

struct StringComponent : async_ecs::component
{
	async_ecs::component_ptr clone() const override {
		auto copy = std::make_shared<StringComponent>();
		copy->name = name;
		async_ecs::component::clone_private_data(copy);
		return copy;
	}

	std::string name;
};

template<>
inline async_ecs::component_tag async_ecs::Tag<StringComponent>()
{
	return 1;
}

struct IntComponent : async_ecs::component
{
	async_ecs::component_ptr clone() const override {
		auto copy = std::make_shared<IntComponent>();
		copy->number = number;
		async_ecs::component::clone_private_data(copy);
		return copy;
	}

	int number = 0;
};

template<>
inline async_ecs::component_tag async_ecs::Tag<IntComponent>()
{
	return 2;
}
