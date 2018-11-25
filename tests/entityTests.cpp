#include <gtest/gtest.h>

#include <entity.h>

#include "TestComponents.h"

TEST(EntityShould, HaveComponentAfterInsert)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	ASSERT_TRUE(!e.has(async_ecs::component::tag_t<IntComponent>()));
	e.insert(intComp);
	ASSERT_TRUE(e.has(async_ecs::component::tag_t<IntComponent>()));
}

TEST(EntityShould, GetComponentOfGivenTag)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	e.insert(intComp);

	bitflag bf(3);
	bf.set(async_ecs::component::tag_t<IntComponent>(), true);
	auto vec1 = e.get(bf);
	auto vec2 = e.get(bf);
	ASSERT_EQ(1, vec1.size());
	ASSERT_EQ(1, vec2.size());
}

TEST(EntityShould, UpdateComponent)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	e.insert(intComp);

	bitflag bf(3);
	bf.set(async_ecs::component::tag_t<IntComponent>(), true);
	auto vec = e.get(bf);
	ASSERT_EQ(1, vec.size());

	auto clone = std::static_pointer_cast<IntComponent>(vec.front()->clone());
	clone->number = 10;

	ASSERT_TRUE(e.update(clone));

	auto updatedVec = e.get(bf);
	ASSERT_EQ(1, updatedVec.size());
	auto updatedComp = std::static_pointer_cast<const IntComponent>(updatedVec.front());

	ASSERT_EQ(10, updatedComp->number);
}

TEST(EntityShould, NotAcceptUpdateRequestIfComponentWasAlreadyUpdatedByAnotherClient)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	e.insert(intComp);

	bitflag bf(3);
	bf.set(async_ecs::component::tag_t<IntComponent>(), true);

	// simulating client 1
	auto vec1 = e.get(bf);
	ASSERT_EQ(1, vec1.size());
	auto updated1 = std::static_pointer_cast<IntComponent>(vec1.front()->clone());
	updated1->number = 10;

	// simulating client 2
	auto vec2 = e.get(bf);
	ASSERT_EQ(1, vec2.size());
	auto updated2 = std::static_pointer_cast<IntComponent>(vec2.front()->clone());
	updated2->number = 20;

	ASSERT_TRUE(e.update(updated1));
	// second update reqest is not handled because updated2 is created based on the same
	// "version" of component as updated1
	ASSERT_FALSE(e.update(updated2));
}

TEST(EntityShould, InsertAndRemoveComponent)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	ASSERT_FALSE(e.has(async_ecs::component::tag_t<IntComponent>()));
	e.insert(intComp);
	ASSERT_TRUE(e.has(async_ecs::component::tag_t<IntComponent>()));
	ASSERT_TRUE(e.remove(async_ecs::component::tag_t<IntComponent>()));
	ASSERT_FALSE(e.has(async_ecs::component::tag_t<IntComponent>()));
}
