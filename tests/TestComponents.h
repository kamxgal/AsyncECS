#pragma once
#include <component.h>

struct StringComponent : async_ecs::component
{
	ECS_COMPONENT(StringComponent)
	
	std::string name;
};

struct IntComponent : async_ecs::component
{
	ECS_COMPONENT(IntComponent)

	int number = 0;
};
