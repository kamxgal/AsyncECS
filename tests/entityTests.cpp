/*
 * AsyncECS
 * Copyright (c) 2018 kamxgal Kamil Galant kamil.galant@gmail.com
 *
 * MIT licence
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <gtest/gtest.h>

#include <entity.h>

#include "TestComponents.h"

TEST(EntityShould, HaveComponentAfterInsert)
{
    std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
    ecs::entity e(0);
    ASSERT_TRUE(!e.has(ecs::component::tag_t<IntComponent>()));
    e.insert(intComp);
    ASSERT_TRUE(e.has(ecs::component::tag_t<IntComponent>()));
}

TEST(EntityShould, GetComponentOfGivenTag)
{
    std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
    ecs::entity e(0);
    e.insert(intComp);

    bitflag bf(ecs::component::tag_t<IntComponent>() + 1);
    bf.set(ecs::component::tag_t<IntComponent>(), true);
    auto vec1 = e.get(bf);
    auto vec2 = e.get(bf);
    ASSERT_EQ(1, vec1.size());
    ASSERT_EQ(1, vec2.size());
}

TEST(EntityShould, UpdateComponent)
{
    std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
    ecs::entity e(0);
    e.insert(intComp);

    bitflag bf(ecs::component::tag_t<IntComponent>() + 1);
    bf.set(ecs::component::tag_t<IntComponent>(), true);
    auto vec = e.get(bf);
    ASSERT_EQ(1, vec.size());

    auto clone = std::static_pointer_cast<const IntComponent>(vec.front())->clone();
    clone.number = 10;

    ASSERT_TRUE(e.update(std::make_shared<IntComponent>(std::move(clone))));

    auto updatedVec = e.get(bf);
    ASSERT_EQ(1, updatedVec.size());
    auto updatedComp = std::static_pointer_cast<const IntComponent>(updatedVec.front());

    ASSERT_EQ(10, updatedComp->number);
}

TEST(EntityShould, NotAcceptUpdateRequestIfComponentWasAlreadyUpdatedByAnotherClient)
{
    std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
    ecs::entity e(0);
    e.insert(intComp);

    bitflag bf(ecs::component::tag_t<IntComponent>() + 1);
    bf.set(ecs::component::tag_t<IntComponent>(), true);

    // simulating client 1
    auto vec1 = e.get(bf);
    ASSERT_EQ(1, vec1.size());
    auto updated1 = std::static_pointer_cast<const IntComponent>(vec1.front())->clone();
    updated1.number = 10;

    // simulating client 2
    auto vec2 = e.get(bf);
    ASSERT_EQ(1, vec2.size());
    auto updated2 = std::static_pointer_cast<const IntComponent>(vec2.front())->clone();
    updated2.number = 20;

    ASSERT_TRUE(e.update(std::make_shared<IntComponent>(std::move(updated1))));
    // second update reqest is not handled because updated2 is created based on the same
    // "version" of component as updated1
    ASSERT_FALSE(e.update(std::make_shared<IntComponent>(std::move(updated2))));
}

TEST(EntityShould, InsertAndRemoveComponent)
{
    std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
    ecs::entity e(0);
    ASSERT_FALSE(e.has(ecs::component::tag_t<IntComponent>()));
    e.insert(intComp);
    ASSERT_TRUE(e.has(ecs::component::tag_t<IntComponent>()));
    ASSERT_TRUE(e.remove(ecs::component::tag_t<IntComponent>()));
    ASSERT_FALSE(e.has(ecs::component::tag_t<IntComponent>()));
}
