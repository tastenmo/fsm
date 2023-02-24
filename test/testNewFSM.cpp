#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <new_fsm/state.h>

using namespace escad::new_fsm;

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {};

using StateVariant = fsm::StateVariant;

struct StateSecond;
struct StateThird;


struct StateFirst : state<StateFirst> {

  StateFirst() : count1(0), value2(0) {}

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }


  // Think about this, should not be here
  //template <typename Event, typename NewState>
  //std::optional<NewState> transitionTo(const Event &) {

  //  return std::nullopt;
  //}

  
  std::optional<StateVariant> StateFirst::transitionTo(const event1 &){

    return StateSecond{};
  }




  //template<>
  //auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateSecond : state<StateSecond> {

  StateSecond() : count1(0) {}

  void onEnter() { count1++; }

  int count1;
};

struct StateThird : state<StateThird> {

  StateThird() : count1(0) {}

  int count1;
};

TEST_CASE("state_onEnter", "[new_fsm]") {

  StateFirst first;
  StateSecond second;
  StateThird third;

  REQUIRE(first.count1 == 0);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 0);
  REQUIRE(third.count1 == 0);

  first.enter();
  second.enter();
  third.enter();

  REQUIRE(first.count1 == 0);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);

  first.enter(event3{});
  second.enter(event3{});
  third.enter(event3{});

  REQUIRE(first.count1 == 0);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);

  first.enter(event1{});
  second.enter(event1{});
  third.enter(event1{});

  REQUIRE(first.count1 == 1);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);

  first.enter(event2{42});
  second.enter(event2{42});
  third.enter(event2{42});

  REQUIRE(first.count1 == 1);
  REQUIRE(first.value2 == 42);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);
}

TEST_CASE("state_transitionTo", "[new_fsm]"){

    StateFirst first;


    auto second = first.transitionTo(event1{});


    
    
    
    
    std::optional<StateSecond> second = first.transition(event1{});

    REQUIRE(second);

    REQUIRE(first.count1 == 1);







}