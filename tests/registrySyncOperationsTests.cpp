#include <gtest/gtest.h>

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

	reg.insert(e1, Tag<StringComponent>(), strC1);
	reg.insert(e1, Tag<IntComponent>(), intC1);
	reg.insert(e2, Tag<StringComponent>(), strC2);
	reg.insert(e2, Tag<IntComponent>(), intC2);

	auto myView = reg.get<StringComponent>();
	EXPECT_EQ(strC1->name, myView.get<StringComponent>(e1)->name);
	EXPECT_EQ(strC2->name, myView.get<StringComponent>(e2)->name);

	auto myView2 = reg.get<StringComponent, IntComponent>();
	EXPECT_EQ(intC1->number, myView2.get<IntComponent>(e1)->number);
	EXPECT_EQ(intC2->number, myView2.get<IntComponent>(e2)->number);
	EXPECT_EQ(strC1->name, myView2.get<StringComponent>(e1)->name);
	EXPECT_EQ(strC2->name, myView2.get<StringComponent>(e2)->name);

	auto strC1_update = myView2.get<StringComponent>(e1)->clone();
	std::static_pointer_cast<StringComponent>(strC1_update)->name = "UPDATED";
	auto strC1_faulty_update = myView2.get<StringComponent>(e1)->clone();
	std::static_pointer_cast<StringComponent>(strC1_faulty_update)->name = "XXXXXXXXXXX";

	ASSERT_TRUE(reg.update(e1, Tag<StringComponent>(), strC1_update));
	ASSERT_FALSE(reg.update(e1, Tag<StringComponent>(), strC1_faulty_update));

	auto myView3 = reg.get<StringComponent, IntComponent>();
	EXPECT_EQ(intC1->number, myView3.get<IntComponent>(e1)->number);
	EXPECT_EQ(intC2->number, myView3.get<IntComponent>(e2)->number);
	EXPECT_EQ(std::static_pointer_cast<StringComponent>(strC1_update)->name,
		myView3.get<StringComponent>(e1)->name);
	EXPECT_EQ(strC2->name, myView3.get<StringComponent>(e2)->name);
}

TEST(RegistryShould, GetSingleComponentWithoutView)
{
	registry reg;
	entity_id e = reg.createEntity();

	auto strC1 = std::make_shared<StringComponent>();
	strC1->name = "AAA";
	reg.insert(e, Tag<StringComponent>(), strC1);

	EXPECT_EQ(reg.get<StringComponent>(e)->name, strC1->name);
}

TEST(RegistryShould, SubscribeForUpdateAndGetNotification)
{
	registry reg;
	entity_id e = reg.createEntity();

	auto strC1 = std::make_shared<StringComponent>();
	strC1->name = "AAA";
	reg.insert(e, Tag<StringComponent>(), strC1);

	auto myView = reg.get<StringComponent>();
	auto comp = myView.get<StringComponent>(myView.entities().front());
	auto update = std::static_pointer_cast<StringComponent>(comp->clone());
	update->name = "UPDATE";


	bool isUpdateNotifReceived = false;
	reg.subscribe<StringComponent>([&isUpdateNotifReceived](entity_id, std::shared_ptr<const StringComponent>) {
		isUpdateNotifReceived = true;
	});

	reg.update(e, Tag<StringComponent>(), update);

	EXPECT_TRUE(isUpdateNotifReceived);
}
