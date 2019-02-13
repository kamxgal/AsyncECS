#pragma once
#include <component.h>

struct StringComponent : ecs::component
{
	ECS_COMPONENT(StringComponent)
	
	std::string name;
};

struct IntComponent : ecs::component
{
	ECS_COMPONENT(IntComponent)

	int number = 0;
};
