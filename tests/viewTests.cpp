#include <gtest/gtest.h>

#include <entity.h>
#include <component.h>
#include <view.h>

#include "TestComponents.h"

struct ViewShould : public ::testing::Test
{
	void SetUp() {
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

	std::vector<async_ecs::entity_id> entities;
	std::vector<async_ecs::component_const_ptr> components;
};

TEST_F(ViewShould, BeCreatedAndHandleGetRequests)
{
	async_ecs::view<StringComponent, IntComponent> myView(std::move(entities), std::move(components));

	ASSERT_NE(nullptr, myView.select<IntComponent>(4));
	EXPECT_EQ(10, myView.select<IntComponent>(4)->number);
	ASSERT_NE(nullptr, myView.select<StringComponent>(4));
	EXPECT_EQ("AAA", myView.select<StringComponent>(4)->name);

	ASSERT_NE(nullptr, myView.select<IntComponent>(10));
	EXPECT_EQ(20, myView.select<IntComponent>(10)->number);
	ASSERT_NE(nullptr, myView.select<StringComponent>(10));
	EXPECT_EQ("BBB", myView.select<StringComponent>(10)->name);
}

TEST_F(ViewShould, BeCreatedAndHandleSelectRequestWithPredicate)
{
	async_ecs::view<StringComponent, IntComponent> myView(std::move(entities), std::move(components));

	auto mapOfFounds = myView.select<IntComponent>([](auto& ptr) {
		return ptr->number < 15;
	});

	ASSERT_EQ(1, mapOfFounds.size());
	auto& front = *mapOfFounds.begin();
	ASSERT_EQ(4, front.first);
	ASSERT_EQ(10, front.second->number);
}

