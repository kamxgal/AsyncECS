#include <gtest/gtest.h>

#include <entity.h>
#include <component.h>
#include <view.h>

#include "TestComponents.h"

TEST(ViewShould, BeCreatedAndHandleGetRequests)
{
	std::vector<async_ecs::entity_id> entities;
	std::vector<async_ecs::component_const_ptr> components;

	{ // creating dummy entities and components
		auto strComp1 = std::make_shared<StringComponent>();
		strComp1->name = "AAA";
		auto intComp1 = std::make_shared<IntComponent>();
		intComp1->number = 10;

		auto strComp2 = std::make_shared<StringComponent>();
		strComp2->name = "BBB";
		auto intComp2 = std::make_shared<IntComponent>();
		intComp2->number = 20;

		entities = { 4, 10 };
		components = { strComp1, intComp1, strComp2, intComp2 };
	}

	async_ecs::view<StringComponent, IntComponent> myView(std::move(entities), std::move(components));

	ASSERT_NE(nullptr, myView.get<IntComponent>(4));
	EXPECT_EQ(10, myView.get<IntComponent>(4)->number);
	ASSERT_NE(nullptr, myView.get<StringComponent>(4));
	EXPECT_EQ("AAA", myView.get<StringComponent>(4)->name);

	ASSERT_NE(nullptr, myView.get<IntComponent>(10));
	EXPECT_EQ(20, myView.get<IntComponent>(10)->number);
	ASSERT_NE(nullptr, myView.get<StringComponent>(10));
	EXPECT_EQ("BBB", myView.get<StringComponent>(10)->name);
}