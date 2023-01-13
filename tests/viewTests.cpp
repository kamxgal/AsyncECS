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

            auto strComp3 = std::make_shared<StringComponent>();
            strComp3->name = "CCC";
            auto intComp3 = std::make_shared<IntComponent>();
            intComp3->number = 30;

            entities = { 4, 10, 17 };
            components = { strComp1, intComp1, strComp2, intComp2, strComp3, intComp3 };
    }

    std::vector<ecs::entity_id> entities;
    std::vector<ecs::component_const_ptr> components;
};

TEST_F(ViewShould, BeCreatedAndHandleGetRequests)
{
    ecs::view<StringComponent, IntComponent> myView(std::move(entities), std::move(components));

    ASSERT_NE(nullptr, myView.select<IntComponent>(4));
    EXPECT_EQ(10, myView.select<IntComponent>(4)->number);
    ASSERT_NE(nullptr, myView.select<StringComponent>(4));
    EXPECT_EQ("AAA", myView.select<StringComponent>(4)->name);

    ASSERT_NE(nullptr, myView.select<IntComponent>(10));
    EXPECT_EQ(20, myView.select<IntComponent>(10)->number);
    ASSERT_NE(nullptr, myView.select<StringComponent>(10));
    EXPECT_EQ("BBB", myView.select<StringComponent>(10)->name);

    ASSERT_NE(nullptr, myView.select<IntComponent>(17));
    EXPECT_EQ(30, myView.select<IntComponent>(17)->number);
    ASSERT_NE(nullptr, myView.select<StringComponent>(17));
    EXPECT_EQ("CCC", myView.select<StringComponent>(17)->name);
}

TEST_F(ViewShould, BeCreatedAndHandleSelectRequestWithPredicate)
{
    ecs::view<StringComponent, IntComponent> myView(std::move(entities), std::move(components));

    auto mapOfFounds = myView.select<IntComponent>([](auto& ptr) {
        return ptr->number < 15 || ptr->number > 25;
    });

    ASSERT_EQ(2, mapOfFounds.size());
    auto iter = mapOfFounds.begin();
    auto& firstEl = *iter;
    ASSERT_EQ(4, firstEl.first);
    ASSERT_EQ(10, firstEl.second->number);
    ++iter;

    auto& thirdEl = *iter;
    ASSERT_EQ(17, thirdEl.first);
    ASSERT_EQ(30, thirdEl.second->number);
}
