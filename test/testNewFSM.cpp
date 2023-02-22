#include <functional>
#include <utility>
#include <iostream>
#include <chrono>

#include <catch2/catch_test_macros.hpp>

#include <new_fsm/state.h>


using namespace escad::new_fsm;


struct event1 {};
struct event2 {

    event2(int val): value_(val) {}

    int value_;
};
struct event3 {};

struct StateFirst : state<StateFirst>{

    StateFirst() : count1(0), value2(0) {}

    void onEnter();

    void onEnter(const event1&){

        count1++;

    } 

    void onEnter(const event2& ev){

        value2 += ev.value_;

    } 

    int count1;
    int value2;

};

struct StateSecond : state<StateSecond>{

    StateSecond() : count1(0) {}

    void onEnter(){

        count1++;

    } 

    int count1;

};




TEST_CASE("state_onEnter", "[new_fsm]"){


    StateFirst first;

    REQUIRE(first.count1 == 0);
    REQUIRE(first.value2 == 0);

    first.enter(event3{});

    REQUIRE(first.count1 == 0);
    REQUIRE(first.value2 == 0);

    first.enter(event1{});

    REQUIRE(first.count1 == 1);
    REQUIRE(first.value2 == 0);

    first.enter(event2{42});

    REQUIRE(first.count1 == 1);
    REQUIRE(first.value2 == 42);






}