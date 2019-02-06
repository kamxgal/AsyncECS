#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <registry.h>

#include "TestComponents.h"

#include <reactive_system.h>

using namespace async_ecs;

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
