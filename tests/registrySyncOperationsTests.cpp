#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <registry.h>

#include "TestComponents.h"

using namespace async_ecs;

TEST(RegistryShould, CreateAndUpdateComponentsAndCollectView)
{
	registry reg;
	entity_id e1 = reg.createEntity();
	entity_id e2 = reg.createEntity();

	auto strC1 = std::make_shared<StringComponent>();
	strC1->name = "AAA";
	auto strC2 = std::make_shared<StringComponent>();
	strC2->name = "BBB";

	auto intC1 = std::make_shared<IntComponent>();
	intC1->number = 10;
	auto intC2 = std::make_shared<IntComponent>();
	intC2->number = 20;

	reg.insert(e1, strC1);
	reg.insert(e1, intC1);
	reg.insert(e2, strC2);
	reg.insert(e2, intC2);

	auto myView = reg.select<StringComponent>();
	EXPECT_EQ(strC1->name, myView.select<StringComponent>(e1)->name);
	EXPECT_EQ(strC2->name, myView.select<StringComponent>(e2)->name);

	auto myView2 = reg.select<StringComponent, IntComponent>();
	EXPECT_EQ(intC1->number, myView2.select<IntComponent>(e1)->number);
	EXPECT_EQ(intC2->number, myView2.select<IntComponent>(e2)->number);
	EXPECT_EQ(strC1->name, myView2.select<StringComponent>(e1)->name);
	EXPECT_EQ(strC2->name, myView2.select<StringComponent>(e2)->name);

	auto strC1_update = myView2.select<StringComponent>(e1)->clone();
	strC1_update->name = "UPDATED";
	auto strC1_faulty_update = myView2.select<StringComponent>(e1)->clone();
	strC1_faulty_update->name = "XXXXXXXXXXX";

	ASSERT_TRUE(reg.update(e1, strC1_update));
	ASSERT_FALSE(reg.update(e1, strC1_faulty_update));

	auto myView3 = reg.select<StringComponent, IntComponent>();
	EXPECT_EQ(intC1->number, myView3.select<IntComponent>(e1)->number);
	EXPECT_EQ(intC2->number, myView3.select<IntComponent>(e2)->number);
	EXPECT_EQ(strC1_update->name, myView3.select<StringComponent>(e1)->name);
	EXPECT_EQ(strC2->name, myView3.select<StringComponent>(e2)->name);
}

TEST(RegistryShould, GetSingleComponentWithoutView)
{
	registry reg;
	entity_id e = reg.createEntity();

	auto strC1 = std::make_shared<StringComponent>();
	strC1->name = "AAA";
	reg.insert(e, strC1);

	EXPECT_EQ(reg.select<StringComponent>(e)->name, strC1->name);
}

TEST(RegistryShould, SubscribeForUpdateAndGetNotification)
{
	registry reg;
	entity_id e = reg.createEntity();

	auto strC1 = std::make_shared<StringComponent>();
	strC1->name = "AAA";
	reg.insert(e, strC1);

	auto myView = reg.select<StringComponent>();
	auto comp = myView.select<StringComponent>(myView.entities().front());
	auto update = comp->clone();
	update->name = "UPDATE";

	bool isUpdateNotifReceived = false;
	reg.subscribe<StringComponent>([&isUpdateNotifReceived](const Notification<StringComponent>&) {
		isUpdateNotifReceived = true;
	});

	reg.update(e, update);

	EXPECT_TRUE(isUpdateNotifReceived);
}
