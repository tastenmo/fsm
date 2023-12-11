#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include "base/utils.h"

// #include <new_fsm/machine.h>
#include <new_fsm/state.h>
#include <new_fsm/state_variant.h>

using namespace escad::new_fsm;

struct Context {};

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct StateInitial;
struct StateSecond;
struct StateThird;

using States = states<StateInitial, StateSecond, StateThird>;

using StateContainer =
    detail::state_variant<States, Context>;

struct StateInitial : state<StateInitial, StateContainer> {

  // using state<StateInitial, StateContainer>::state;

  StateInitial(StateContainer &state_container) noexcept;
  // count1(0), value2(0) {}
  ~StateInitial() { std::cout << "StateInitial::~StateInitial()" << std::endl; }

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return trans<StateSecond>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateSecond : state<StateSecond, StateContainer> {

  //  using state<StateSecond, StateContainer>::state;

  StateSecond(StateContainer &state_container) noexcept;

  ~StateSecond() { std::cout << "StateSecond::~StateSecond()" << std::endl; }

  void onEnter() { count1++; }

  auto transitionTo(const event2 &event) const
      -> transitions<StateInitial, StateSecond, StateThird> {
    if (event.value_ == 1) {
      return trans<StateInitial>();
    } else if (event.value_ == 2) {
      return trans<StateThird>();
    }
    // handled() does not work yet
    return trans<StateSecond>();
  }

  auto transitionTo(const event1 &) const { return handled(); }

  int count1;
};

struct StateThird : state<StateThird, StateContainer> {

  using state<StateThird, StateContainer>::state;

  StateThird(StateContainer &state_container) noexcept;
  ~StateThird() { std::cout << "StateThird::~StateThird()" << std::endl; }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // StateThird() : count1(0) {}

  int count1;
};

// State Constructors

auto myStates = StateContainer{};

auto myStatePrinter = escad::overloaded{
    [](StateInitial &) { std::cout << "StateInitial" << std::endl; },
    [](StateSecond &) { std::cout << "StateSecond" << std::endl; },
    [](StateThird &) { std::cout << "StateThird" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

StateInitial::StateInitial(StateContainer &state_container) noexcept
    : state(state_container), count1(0), value2(0) {

  std::cout << "StateInitial::StateInitial()" << std::endl;
}

StateSecond::StateSecond(StateContainer &state_container) noexcept
    : state(state_container), count1(0) {

  std::cout << "StateSecond::StateSecond()" << std::endl;
}

StateThird::StateThird(StateContainer &state_container) noexcept
    : state(state_container), count1(0) {

  std::cout << "StateThird::StateThird()" << std::endl;
}


TEST_CASE("state_onEnter", "[new_fsm]") {

  StateInitial first{myStates};
  StateSecond second{myStates};
  StateThird third{myStates};

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

  first.enter(event3{"test"});
  second.enter(event3{"test"});
  third.enter(event3{"test"});

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

TEST_CASE("state_transitionTo", "[new_fsm]") {

  StateInitial first{myStates};

  auto result = first.transitionTo(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<StateSecond>>);
  REQUIRE(result.is_transition());

  auto result1 = first.transition(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result1), transitions<StateSecond>>);
  REQUIRE(result1.is_transition());

  StateSecond second{myStates};

  auto result2 = second.transition(event1{});
  CHECK(result2.is_handled());

  auto result3 = second.transition(event2(1));
  STATIC_REQUIRE(
      std::is_same_v<decltype(result3),
                     transitions<StateInitial, StateSecond, StateThird>>);
  REQUIRE(result3.is_transition());
  REQUIRE(result3.idx == 0);

  auto result4 = second.transition(event2(2));
  STATIC_REQUIRE(
      std::is_same_v<decltype(result4),
                     transitions<StateInitial, StateSecond, StateThird>>);
  REQUIRE(result4.is_transition());
  REQUIRE(result4.idx == 2);

  auto result5 = second.transition(event2(3));
  STATIC_REQUIRE(
      std::is_same_v<decltype(result5),
                     transitions<StateInitial, StateSecond, StateThird>>);
  REQUIRE(result5.is_transition());
  REQUIRE(result5.idx == 1);

  
}

TEST_CASE("state_dispatch", "[new_fsm]") {

  std::cout << "before emplace: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  myStates.emplace<StateInitial>();

  REQUIRE(myStates.is_in<StateInitial>());

  std::cout << "after emplace: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  std::cout << "before dispatch event1: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  auto result = myStates.dispatch(event1{});

  REQUIRE(result);
  REQUIRE(myStates.is_in<StateSecond>());

  std::cout << "after dispatch event1: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  std::cout << "before dispatch event1: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  auto result2 = myStates.dispatch(event1{});
  REQUIRE(result2);
  REQUIRE(myStates.is_in<StateSecond>());

  std::cout << "before dispatch event2: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  auto result3 = myStates.dispatch(event2{1});
  // auto result3 = state2_1.dispatch(event2{1});

  REQUIRE(result3);
  REQUIRE(myStates.is_in<StateInitial>());

  std::cout << "after dispatch event2: ";

  myStates.visit(myStatePrinter);

  std::cout << std::endl;

  auto state3 = myStates.state<StateInitial>();
  REQUIRE(state3.value2 == 1);

  REQUIRE(myStates.is_in<StateInitial>());
}
