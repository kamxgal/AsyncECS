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
#include <gmock/gmock.h>

#include <registry.h>

#include "TestComponents.h"

#include <reactive_system.h>

using namespace ecs;

struct AsyncInsertIntComponent : public command
{
    virtual ~AsyncInsertIntComponent() = default;
    AsyncInsertIntComponent(registry& reg, entity_id eId)
        : mRegistry(reg), eId(eId) {
    }
    void execute() override {
        auto intComponent = mRegistry.select<IntComponent>(eId);
        if (!intComponent) {
            IntComponent newIntComponent;
            newIntComponent.number = 999;
            mRegistry.insert(eId, std::move(newIntComponent));
        }
    }
    registry& mRegistry;
    entity_id eId;
};

struct AsyncUpdateIntComponent : public command
{
    virtual ~AsyncUpdateIntComponent() = default;
    AsyncUpdateIntComponent(registry& reg, std::string strValue, entity_id eId)
        : mRegistry(reg), toConvert(std::move(strValue)), eId(eId) {
    }
    void execute() override {
        auto intComponent = mRegistry.select<IntComponent>(eId);
        if (intComponent) {
            auto updated = intComponent->clone();
            updated.number = atoi(toConvert.c_str());
            mRegistry.update(eId, std::move(updated));
        }
    }
    registry& mRegistry;
    std::string toConvert;
    entity_id eId;
};

struct StringInsertionListener : public reactive_system
{
    StringInsertionListener(registry& reg) : mRegistry(reg) {
    }

    void initialize() {
        mRegistry.subscribe<StringComponent>([&] (const Notification<StringComponent>& nn) {
            handleStringComponentInsert(*nn.component, nn.entityId);
        }, [] (const Notification<StringComponent>& nn) -> bool {
            return nn.operation == operation_t::inserted;
        });
    }

    void handleStringComponentInsert(const StringComponent& cc, entity_id eId) {
        add_task(std::make_unique<AsyncInsertIntComponent>(mRegistry, eId));
    }

    registry& mRegistry;
};

struct StringUpdateListener : public reactive_system
{
    StringUpdateListener(registry& reg) : mRegistry(reg) {
    }

    void initialize() {
        mRegistry.subscribe<StringComponent>([&] (const Notification<StringComponent>& nn) {
            handleStringComponentUpdate(*nn.component, nn.entityId);
        }, [] (const Notification<StringComponent>& nn) -> bool {
            return nn.operation == operation_t::updated;
        });
    }

    void handleStringComponentUpdate(const StringComponent& cc, entity_id eId) {
        add_task(std::make_unique<AsyncUpdateIntComponent>(mRegistry, cc.name, eId));
    }

    registry& mRegistry;
};

TEST(RegistryShould, HandleReactiveSystemRequestToInsertIntComponentOnStringComponentInsertion)
{
    registry reg;
    StringInsertionListener system(reg);
    system.initialize();
    system.start();

    entity_id eid = reg.createEntity();
    StringComponent stringComponent;
    stringComponent.name = "AAA";

    reg.insert(eid, std::move(stringComponent));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto intComponent = reg.select<IntComponent>(eid);
    ASSERT_NE(nullptr, intComponent);
    EXPECT_EQ(999, intComponent->number);
}

TEST(RegistryShould, HandleReactiveSystemRequestToUpdateIntComponentOnStringComponentUpdate)
{
    registry reg;
    StringUpdateListener systemA(reg);
    systemA.initialize();
    systemA.start();

    entity_id eid = reg.createEntity();

    {
        StringComponent stringComponent;
        stringComponent.name = "AAA";
        reg.insert(eid, std::move(stringComponent));
        IntComponent intComponent;
        intComponent.number = 999;
        reg.insert(eid, std::move(intComponent));

        auto updated = stringComponent.clone();
        updated.name = "12345";
        reg.update(eid, std::move(updated));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto intComponent = reg.select<IntComponent>(eid);
    ASSERT_NE(nullptr, intComponent);
    EXPECT_EQ(12345, intComponent->number);
}

struct IntComponentUpdater : public reactive_system
{
    IntComponentUpdater(registry& reg, entity_id eid) : mRegistry(reg), eid(eid) {
    }

    void initialize() {
        struct update : public command {
            update(registry& r, entity_id eid) : r(r), eid(eid) {}
            void execute() override {
                while (!is_stopped)
                {
                    auto c = r.select<IntComponent>(eid)->clone();
                    if (c.number >= 10000) {
                        return;
                    }
                    ++c.number;
                    bool res = r.update(eid, std::move(c));
                }
            }
            registry& r;
            entity_id eid;
        };

        add_task(std::make_unique<update>(mRegistry, eid));
        add_task(std::make_unique<ecs::stop>(*this));
    }

    registry& mRegistry;
    entity_id eid;
};

TEST(RegistryShould, HandleTwoAsyncSystemsUpdatingTheSameComponentAtOnce)
{
    registry reg;
    entity_id ee = reg.createEntity();
    IntComponent intComponent;
    intComponent.number = 0;
    reg.insert(ee, std::move(intComponent));

    IntComponentUpdater updater1(reg, ee);
    IntComponentUpdater updater2(reg, ee);
    updater1.initialize();
    updater2.initialize();
    // there might be drops in updating but at the end both updaters should finish their work
    updater1.start();
    updater2.start();
    updater1.join();
    updater2.join();
    EXPECT_EQ(10000, reg.select<IntComponent>(ee)->number);
}
