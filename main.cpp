#include <iostream>
#include <thread>

#include "bitflag.h"

#include "entity.h"
#include "component.h"
#include "view.h"
#include "registry.h"

using namespace std;
using namespace async_ecs;

void test_bitflag()
{
    bitflag f(15);

    bitflag sec(4); sec.set(3, true);
    sec.resize(11); sec.set(3, false); sec.set(9, true);
    std::cout << "sec = " << sec << std::endl;

    for(int i=0; i<f.size(); ++i) {
        f.set(i, true);
        bool res = (f & sec);
        std::cout << f << " " << res << std::endl;
        f.set(i, false);
    }
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

void test_entity_operations()
{
    std::shared_ptr<IntComponent> intComp = std::make_shared<IntComponent>();
    async_ecs::entity e(0);
    assert(!e.has(async_ecs::Tag<IntComponent>()));
    e.insert(async_ecs::Tag<IntComponent>(), intComp);
    assert(e.has(async_ecs::Tag<IntComponent>()));

    bitflag bf(3);
    bf.set(Tag<IntComponent>(), true);
    auto vec1 = e.get(bf);
    auto vec2 = e.get(bf);
    assert(vec1.size() == 1);
    assert(vec2.size() == 1);

    auto clone1 = std::static_pointer_cast<IntComponent>(vec1.front()->clone());
    clone1->number = 10;
    auto clone2 = std::static_pointer_cast<IntComponent>(vec1.front()->clone());
    clone2->number = 20;

    assert(e.update(Tag<IntComponent>(), clone1));
    assert(!e.update(Tag<IntComponent>(), clone2));

    auto updatedVec = e.get(bf);
    assert(updatedVec.size() == 1);
    auto updatedComp = std::static_pointer_cast<const IntComponent>(updatedVec.front());

    assert(updatedComp->number == 10);

    assert(e.remove(Tag<IntComponent>()));
    auto emptyVec = e.get(bf);
    assert(emptyVec.empty());
}

void test_view()
{
    auto strC1 = std::make_shared<StringComponent>();
    strC1->name = "AAA";

    auto strC2 = std::make_shared<StringComponent>();
    strC2->name = "BBB";

    auto intC1 = std::make_shared<IntComponent>();
    intC1->number = 10;
    auto intC2 = std::make_shared<IntComponent>();
    intC2->number = 20;

    std::vector<component_const_ptr> vec = { strC1, intC1, strC2, intC2 };

    view<StringComponent, IntComponent> myView({4,10}, std::move(vec));

    auto comp = myView.get<IntComponent>(10);
    if (comp)
    {
        std::cout << "> " << comp->number << std::endl;
    }
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

//template<int... Ks>
//struct Indexer
//{
//    template<int N>
//    struct GetIndex {
//        static int index;
//    };

//    static void setup() {
//        setupImpl<Ks...>(0);
//    }

//    template<int K>
//    static void setupImpl(int id) {
//        GetIndex<K>::index = id;
//    }

//    template<int K1, int K2, int... Rest>
//    static void setupImpl(int id) {
//        GetIndex<K1>::index = id++;
//        setupImpl<K2, Rest...>(id);
//    }
//};

//template<int... Ks> template<int N> int Indexer<Ks...>::GetIndex<N>::index = 0;

int main(int argc, char *argv[])
{
//    test_bitflag();
//    test_entity_operations();
//    test_view();

//    test_registry();

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

    return 0;
}
