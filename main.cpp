#include <iostream>
#include <thread>
#include <sstream>

#include <gtest/gtest.h>

#include "bitflag.h"

#include "entity.h"
#include "component.h"
#include "view.h"
#include "registry.h"

using namespace std;
using namespace async_ecs;

TEST(BitflagShould, SetAndUnsetProperFlag)
{
	bitflag f(4);
	EXPECT_EQ("0000", f.str());
	f.set(3, true);
	EXPECT_EQ("0001", f.str());
	f.set(0, true);
	EXPECT_EQ("1001", f.str());
	f.set(0, false);
	EXPECT_EQ("0001", f.str());
	f.set(3, false);
	EXPECT_EQ("0000", f.str());
}

TEST(BitflagShould, SetAndUnsetProperFlagAfterResize)
{
	bitflag f(4);
	f.set(3, true);
	EXPECT_EQ("0001", f.str());

	f.resize(11);
	EXPECT_EQ("00010000000", f.str());
	f.set(3, false);
	EXPECT_EQ("00000000000", f.str());
	f.set(9, true);
	EXPECT_EQ("00000000010", f.str());
}

TEST(BitflagShould, Reverse)
{
	bitflag first(6);
	first.set(3, true);

	bitflag second = !first;
	EXPECT_EQ("111011", second.str());
}

TEST(BitflagShould, CompareWithBitAndOperationLessThanByte)
{
	bitflag first(6);
	first.set(3, true);

	bitflag second(4);
	ASSERT_FALSE(first & second);

	second.set(0, true);
	ASSERT_FALSE(first & second);
	second.set(1, true);
	ASSERT_FALSE(first & second);
	second.set(2, true);
	ASSERT_FALSE(first & second);
	second.set(3, true);
	char val = first & second;
	ASSERT_FALSE(first & second);
	second.set(0, false);
	ASSERT_FALSE(first & second);
	second.set(1, false);
	ASSERT_FALSE(first & second);
	second.set(2, false);
	ASSERT_TRUE(first & second);
}

TEST(BitflagShould, CompareWithBitAndOperationTwoBytes)
{
	bitflag first(14);
	first.set(12, true);

	bitflag second(11);
	ASSERT_TRUE(first & second);

	first.set(10, true);
	ASSERT_FALSE(first & second);

	second.set(10, true);
	ASSERT_TRUE(first & second);

	first.set(1, true);
	ASSERT_FALSE(first & second);

	second.set(1, true);
	ASSERT_TRUE(first & second);
}

struct StringComponent : async_ecs::component
{
    component_ptr clone() const override {
        auto copy = std::make_shared<StringComponent>();
        copy->name = name;
        async_ecs::component::clone_private_data(copy);
        return copy;
    }

    std::string name;
};

template<>
component_tag async_ecs::Tag<StringComponent>()
{
    return 1;
}

struct IntComponent : async_ecs::component
{
    component_ptr clone() const override {
        auto copy = std::make_shared<IntComponent>();
        copy->number = number;
        async_ecs::component::clone_private_data(copy);
        return copy;
    }

    int number = 0;
};

template<>
component_tag async_ecs::Tag<IntComponent>()
{
    return 2;
}

TEST(EntityShould, HaveComponentAfterInsert)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	ASSERT_TRUE(!e.has(async_ecs::Tag<IntComponent>()));
	e.insert(async_ecs::Tag<IntComponent>(), intComp);
	ASSERT_TRUE(e.has(async_ecs::Tag<IntComponent>()));

}

TEST(EntityShould, GetComponentOfGivenTag)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	e.insert(async_ecs::Tag<IntComponent>(), intComp);

	bitflag bf(3);
	bf.set(Tag<IntComponent>(), true);
	auto vec1 = e.get(bf);
	auto vec2 = e.get(bf);
	ASSERT_EQ(1, vec1.size());
	ASSERT_EQ(1, vec2.size());
}

TEST(EntityShould, UpdateComponent)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	e.insert(async_ecs::Tag<IntComponent>(), intComp);

	bitflag bf(3);
	bf.set(Tag<IntComponent>(), true);
	auto vec = e.get(bf);
	ASSERT_EQ(1, vec.size());

	auto clone = std::static_pointer_cast<IntComponent>(vec.front()->clone());
	clone->number = 10;

	ASSERT_TRUE(e.update(Tag<IntComponent>(), clone));

	auto updatedVec = e.get(bf);
	ASSERT_EQ(1, updatedVec.size());
	auto updatedComp = std::static_pointer_cast<const IntComponent>(updatedVec.front());

	ASSERT_EQ(10, updatedComp->number);
}

TEST(EntityShould, NotAcceptUpdateRequestIfComponentWasAlreadyUpdatedByAnotherClient)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	e.insert(async_ecs::Tag<IntComponent>(), intComp);

	bitflag bf(3);
	bf.set(Tag<IntComponent>(), true);

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

	ASSERT_TRUE(e.update(Tag<IntComponent>(), updated1));
	// second update reqest is not handled because updated2 is created based on the same
	// "version" of component as updated1
	ASSERT_FALSE(e.update(Tag<IntComponent>(), updated2));
}

TEST(EntityShould, InsertAndRemoveComponent)
{
	std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
	async_ecs::entity e(0);
	ASSERT_FALSE(e.has(async_ecs::Tag<IntComponent>()));
	e.insert(async_ecs::Tag<IntComponent>(), intComp);
	ASSERT_TRUE(e.has(async_ecs::Tag<IntComponent>()));
	ASSERT_TRUE(e.remove(Tag<IntComponent>()));
	ASSERT_FALSE(e.has(async_ecs::Tag<IntComponent>()));
}

TEST(ViewShould, BeCreatedAndHandleGetRequests)
{
	std::vector<entity_id> entities;
	std::vector<component_const_ptr> components;

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

    view<StringComponent, IntComponent> myView(std::move(entities), std::move(components));

    ASSERT_NE(nullptr, myView.get<IntComponent>(4));
	EXPECT_EQ(10, myView.get<IntComponent>(4)->number);
	ASSERT_NE(nullptr, myView.get<StringComponent>(4));
	EXPECT_EQ("AAA", myView.get<StringComponent>(4)->name);

	ASSERT_NE(nullptr, myView.get<IntComponent>(10));
	EXPECT_EQ(20, myView.get<IntComponent>(10)->number);
	ASSERT_NE(nullptr, myView.get<StringComponent>(10));
	EXPECT_EQ("BBB", myView.get<StringComponent>(10)->name);
}

void test_registry()
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
    std::cout << myView.get<StringComponent>(e1)->name << std::endl;
    std::cout << myView.get<StringComponent>(e2)->name << std::endl;

    auto myView2 = reg.get<StringComponent, IntComponent>();
    std::cout << myView2.get<IntComponent>(e1)->number << std::endl;
    std::cout << myView2.get<IntComponent>(e2)->number << std::endl;
    std::cout << myView2.get<StringComponent>(e1)->name << std::endl;
    std::cout << myView2.get<StringComponent>(e2)->name << std::endl;

    auto strC1_update = myView2.get<StringComponent>(e1)->clone();
    std::static_pointer_cast<StringComponent>(strC1_update)->name = "UPDATED";
    auto strC1_faulty_update = myView2.get<StringComponent>(e1)->clone();
    std::static_pointer_cast<StringComponent>(strC1_faulty_update)->name = "XXXXXXXXXXX";

    assert(reg.update(e1, Tag<StringComponent>(), strC1_update));
    assert(!reg.update(e1, Tag<StringComponent>(), strC1_faulty_update));

    auto myView3 = reg.get<StringComponent, IntComponent>();
    std::cout << myView3.get<IntComponent>(e1)->number << std::endl;
    std::cout << myView3.get<IntComponent>(e2)->number << std::endl;
    std::cout << myView3.get<StringComponent>(e1)->name << std::endl;
    std::cout << myView3.get<StringComponent>(e2)->name << std::endl;
}

void test_async_access()
{
    registry reg;
    entity_id e = reg.createEntity();

    auto strC1 = std::make_shared<StringComponent>();
    strC1->name = "AAA";
    reg.insert(e, Tag<StringComponent>(), strC1);

    std::vector<std::string> logs;
    std::mutex logMutex;

    std::thread logger([&logs, &logMutex](){
        while(true)
        {
            logMutex.lock();
            if (logs.size() < 10) { logMutex.unlock(); continue; }
            std::string p = logs.front();
            logs.erase(logs.begin());

            std::cout << ">" << p << std::endl;
            logMutex.unlock();

        }
    });

    std::thread th1([&reg, &logMutex, &logs](){
        while(true)
        {
            auto myView = reg.get<StringComponent>();
            auto comp = myView.get<StringComponent>(myView.entities().front());

            auto update = std::static_pointer_cast<StringComponent>(comp->clone());
            update->name = "THREAD 1";

            bool result = reg.update(myView.entities().front(), Tag<StringComponent>(), update);
            std::string print = "TH1> " + comp->name + " " + (result ? "1" : "0");
            logMutex.lock();
            logs.emplace_back(std::move(print));
            logMutex.unlock();
        }
    });

    std::thread th2([&reg, &logMutex, &logs](){
        while(true)
        {
            auto myView = reg.get<StringComponent>();
            auto comp = myView.get<StringComponent>(myView.entities().front());

            auto update = std::static_pointer_cast<StringComponent>(comp->clone());
            update->name = "THREAD 2";

            bool result = reg.update(myView.entities().front(), Tag<StringComponent>(), update);
            std::string print = "TH2> " + comp->name + " " + (result ? "1" : "0");
            logMutex.lock();
            logs.emplace_back(std::move(print));
            logMutex.unlock();
        }
    });

    std::thread th3([&reg, &logMutex, &logs](){
        while(true)
        {
            auto myView = reg.get<StringComponent>();
            auto comp = myView.get<StringComponent>(myView.entities().front());

            auto update = std::static_pointer_cast<StringComponent>(comp->clone());
            update->name = "THREAD 3";

            bool result = reg.update(myView.entities().front(), Tag<StringComponent>(), update);
            std::string print = "TH3> " + comp->name + " " + (result ? "1" : "0");
            logMutex.lock();
            logs.push_back(print);
            logMutex.unlock();
        }
    });

    th1.join();
    th2.join();
    th3.join();
    logger.join();
}

void test_subscribing_and_getting_notification_of_async_update()
{
    registry reg;

    entity_id e1 = reg.createEntity();
    auto strC1 = std::make_shared<StringComponent>();
    strC1->name = "E1";
    reg.insert(e1, Tag<StringComponent>(), strC1);

    entity_id e2 = reg.createEntity();
    auto strC2 = std::make_shared<StringComponent>();
    strC2->name = "E2";
    reg.insert(e2, Tag<StringComponent>(), strC2);

    std::mutex logMutex;

    bool isFinished = false;

    reg.subscribe<StringComponent>([&logMutex](entity_id id, std::shared_ptr<const StringComponent> c){
        std::unique_lock<std::mutex> lock(logMutex);
        std::cout << "E1> " << c->name << std::endl;
    }, [e1](entity_id id, std::shared_ptr<const StringComponent>) {
        return id == e1;
    });

    reg.subscribe<StringComponent>([&logMutex](entity_id id, std::shared_ptr<const StringComponent> c){
        std::unique_lock<std::mutex> lock(logMutex);
        std::cout << "E2> " << c->name << std::endl;
    }, [e2](entity_id id, std::shared_ptr<const StringComponent>) {
        return id == e2;
    });

    reg.subscribe<StringComponent>([&isFinished](entity_id id, std::shared_ptr<const StringComponent> c){
        isFinished = true;
    }, [e1](entity_id id, std::shared_ptr<const StringComponent> c) {
        return c->name.find("10000") != std::string::npos;
    });

    std::thread th1([&reg, &isFinished, e1](){
        unsigned count = 0;

        while (!isFinished)
        {
            auto comp = reg.get<StringComponent>(e1);
            auto update = std::static_pointer_cast<StringComponent>(comp->clone());
            std::stringstream ss;
            ss << "E1 - ver " << count++;
            update->name = ss.str();
            reg.update(e1, Tag<StringComponent>(), update);
        }
    });

    std::thread th2([&reg, &isFinished, e2](){
        unsigned count = 0;

        while (!isFinished)
        {
            auto comp = reg.get<StringComponent>(e2);
            auto update = std::static_pointer_cast<StringComponent>(comp->clone());
            std::stringstream ss;
            ss << "E2 - ver " << count++;
            update->name = ss.str();
            reg.update(e2, Tag<StringComponent>(), update);
        }
    });

    th1.join();
    th2.join();
}

void test_getting_single_component()
{
    registry reg;
    entity_id e = reg.createEntity();

    auto strC1 = std::make_shared<StringComponent>();
    strC1->name = "AAA";
    reg.insert(e, Tag<StringComponent>(), strC1);

    std::cout << reg.get<StringComponent>(e)->name << std::endl;
    assert(reg.get<StringComponent>(e)->name == strC1->name);
}

void test_subscibing_and_getting_nottification()
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
    reg.subscribe<StringComponent>([&isUpdateNotifReceived](entity_id, std::shared_ptr<const StringComponent>){
        std::cout << "isUpdateNotifReceived = true" << std::endl;
        isUpdateNotifReceived = true;
    });

    reg.update(e, Tag<StringComponent>(), update);

    assert(isUpdateNotifReceived);
    std::cout << "isUpdateNotifReceived = " << isUpdateNotifReceived << std::endl;
}


/**
int main(int argc, char *argv[])
{
//    test_bitflag();
//    test_entity_operations();
//    test_view();

//    test_registry();

//    test_async_access();

//    test_getting_single_component();

//    test_subscibing_and_getting_nottification();

    //test_subscribing_and_getting_notification_of_async_update();

    return 0;
}
*/

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	std::getchar(); // keep console window open until Return keystroke
}