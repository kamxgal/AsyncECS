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

    StringComponent strC1;
	strC1.name = "AAA";
    StringComponent strC2;
	strC2.name = "BBB";

    IntComponent intC1;
	intC1.number = 10;
    IntComponent intC2;
	intC2.number = 20;

	reg.insert(e1, StringComponent(strC1));
	reg.insert(e1, IntComponent(intC1));
	reg.insert(e2, StringComponent(strC2));
	reg.insert(e2, IntComponent(intC2));
    
	auto myView = reg.select<StringComponent>();
	EXPECT_EQ(strC1.name, myView.select<StringComponent>(e1)->name);
	EXPECT_EQ(strC2.name, myView.select<StringComponent>(e2)->name);

	auto myView2 = reg.select<StringComponent, IntComponent>();
	EXPECT_EQ(intC1.number, myView2.select<IntComponent>(e1)->number);
	EXPECT_EQ(intC2.number, myView2.select<IntComponent>(e2)->number);
	EXPECT_EQ(strC1.name, myView2.select<StringComponent>(e1)->name);
	EXPECT_EQ(strC2.name, myView2.select<StringComponent>(e2)->name);

	auto strC1_update = myView2.select<StringComponent>(e1)->clone();
	strC1_update->name = "UPDATED";
	auto strC1_faulty_update = myView2.select<StringComponent>(e1)->clone();
	strC1_faulty_update->name = "XXXXXXXXXXX";

	ASSERT_TRUE(reg.update(e1, strC1_update));
	ASSERT_FALSE(reg.update(e1, strC1_faulty_update));

	auto myView3 = reg.select<StringComponent, IntComponent>();
	EXPECT_EQ(intC1.number, myView3.select<IntComponent>(e1)->number);
	EXPECT_EQ(intC2.number, myView3.select<IntComponent>(e2)->number);
	EXPECT_EQ(strC1_update->name, myView3.select<StringComponent>(e1)->name);
	EXPECT_EQ(strC2.name, myView3.select<StringComponent>(e2)->name);
}

TEST(RegistryShould, GetSingleComponentWithoutView)
{
	registry reg;
	entity_id e = reg.createEntity();

    StringComponent strC1;
	strC1.name = "AAA";
	reg.insert(e, StringComponent(strC1));

	EXPECT_EQ(reg.select<StringComponent>(e)->name, strC1.name);
}

TEST(RegistryShould, SubscribeForUpdateAndGetNotification)
{
	registry reg;
	entity_id e = reg.createEntity();

    StringComponent strC1;
	strC1.name = "AAA";
	reg.insert(e, std::move(strC1));

	auto myView = reg.select<StringComponent>();
	auto comp = myView.select<StringComponent>(myView.entities().front());
	auto update = comp->clone();
	update->name = "UPDATE";

	bool isUpdateNotifReceived = false;
	reg.subscribe<StringComponent>([&isUpdateNotifReceived](const Notification<StringComponent>&) {
		isUpdateNotifReceived = true;
	}, [e](const Notification<StringComponent>& notif) -> bool {
		return notif.operation == operation_t::updated
			&& notif.component != nullptr
			&& notif.entityId == e;
	});

	reg.update(e, update);

	EXPECT_TRUE(isUpdateNotifReceived);
}

TEST(RegistryShould, SubscribeForInsertAndGetNotification)
{
	registry reg;
	entity_id e = reg.createEntity();

    StringComponent strC1;
	strC1.name = "AAA";

	bool isInsertNotifReceived = false;
	reg.subscribe<StringComponent>([&isInsertNotifReceived](const Notification<StringComponent>&) {
		isInsertNotifReceived = true;
	}, [e](const Notification<StringComponent>& notif) -> bool {
		return notif.operation == operation_t::inserted
			&& notif.component != nullptr
			&& notif.entityId == e;
	});

	reg.insert(e, std::move(strC1));

	EXPECT_TRUE(isInsertNotifReceived);
}

TEST(RegistryShould, SubscribeForRemovalAndGetNotification)
{
	registry reg;
	entity_id e = reg.createEntity();

    StringComponent strC1;
	strC1.name = "AAA";
	reg.insert(e, std::move(strC1));

	bool isRemovedNotifReceived = false;
	reg.subscribe<StringComponent>([&isRemovedNotifReceived](const Notification<StringComponent>&) {
		isRemovedNotifReceived = true;
	}, [e](const Notification<StringComponent>& notif) -> bool {
		return notif.operation == operation_t::removed
			&& notif.entityId == e
			&& notif.component == nullptr;
	});

	reg.remove<StringComponent>(e);

	EXPECT_TRUE(isRemovedNotifReceived);
}

TEST(RegistryShould, SubscribeForRemovalAndGetNotificationWhenEntityIsRemoved)
{
	registry reg;
	entity_id e = reg.createEntity();

    StringComponent strC1;
	strC1.name = "AAA";
	reg.insert(e, std::move(strC1));

	bool isRemovedNotifReceived = false;
	reg.subscribe<StringComponent>([&isRemovedNotifReceived](const Notification<StringComponent>&) {
		isRemovedNotifReceived = true;
	}, [e](const Notification<StringComponent>& notif) -> bool {
		return notif.operation == operation_t::removed
			&& notif.entityId == e
			&& notif.component == nullptr;
	});

	reg.remove(e);

	EXPECT_TRUE(isRemovedNotifReceived);
}

TEST(RegistryShould, GiveUnsafeAccessToComponentAndNotifyUpdateOnTriggerFromUser)
{
	registry reg;
	entity_id e = reg.createEntity();

	const std::string FIRST_NAME = "AAA";
	const std::string SECOND_NAME = "BBB";

	{
        StringComponent strC1;
		strC1.name = FIRST_NAME;
		reg.insert(e, std::move(strC1));
	}

	auto cc = reg.select_unsafely<StringComponent>(e);
	cc->name = SECOND_NAME;

	bool isUpdateNotifReceived = false;
	reg.subscribe<StringComponent>([&isUpdateNotifReceived](const Notification<StringComponent>&) {
		isUpdateNotifReceived = true;
	}, [e, &SECOND_NAME](const Notification<StringComponent>& notif) -> bool {
		return notif.operation == operation_t::updated
			&& notif.component != nullptr
			&& notif.component->name == SECOND_NAME
			&& notif.entityId == e;
	});

	reg.finish_unsafe_update<StringComponent>(e);

	EXPECT_TRUE(isUpdateNotifReceived);
}

TEST(RegistryShould, SubscribeAndUnsubscribe)
{
	registry reg;
	entity_id e = reg.createEntity();

    StringComponent strC1;
	strC1.name = "AAA";
	reg.insert(e, std::move(strC1));

	bool isRemovedNotifReceived = false;
	auto unsubscribe = reg.subscribe<StringComponent>([&isRemovedNotifReceived](const Notification<StringComponent>&) {
		isRemovedNotifReceived = true;
	}, [e](const Notification<StringComponent>& notif) -> bool {
		return notif.operation == operation_t::removed
			&& notif.entityId == e
			&& notif.component == nullptr;
	});

	unsubscribe();

	reg.remove<StringComponent>(e);

	EXPECT_FALSE(isRemovedNotifReceived);
}
