#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <fsm/state_machine.h>
#include <variant>

using namespace escad::new_fsm;

struct event1 {};

struct myStates {

  struct Initial : state<Initial> {
    void enter() { std::cout << "Initial" << std::endl; }
    auto transition(const event1 &) { return sibling<Second>(); }
  };

  struct Second : state<Second> {
    void enter() { std::cout << "Second" << std::endl; }
    auto transition(const event1 &) { return sibling<Third>(); }
  };

  struct Third : state<Third> {
    void enter() { std::cout << "Third" << std::endl; }
    auto transition(const event1 &) { return sibling<Initial>(); }
  };
};

using States = states<myStates::Initial, myStates::Second, myStates::Third>;

TEST_CASE("state_variant no ctx", "[new_fsm]") {

  auto fsm = StateMachine(mpl::type_identity<States>{}, {});

  auto mono = fsm.state<std::monostate>();

  STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);

  fsm.emplace<myStates::Initial>();

  REQUIRE(fsm.is_in<myStates::Initial>());
  auto Initial = fsm.state<myStates::Initial>();

  STATIC_REQUIRE(std::is_same_v<decltype(Initial), myStates::Initial>);
  REQUIRE(fsm.valueless_by_exception() == false);
}

struct Ctx {
  int i = 0;
};

struct myStatesCtx {

  struct Initial : state<Initial, Ctx> {
    void enter() { std::cout << "Initial" << std::endl; }
    auto transition(const event1 &) { return sibling<Second>(); }
  };

  struct Second : state<Second, Ctx> {
    void enter() { std::cout << "Second" << std::endl; }
    auto transition(const event1 &) { return sibling<Third>(); }
  };

  struct Third : state<Third> {
    void enter() { std::cout << "Third" << std::endl; }
    auto transition(const event1 &) { return sibling<Initial>(); }
  };
};

using StatesCtx =
    states<myStatesCtx::Initial, myStatesCtx::Second, myStatesCtx::Third>;

TEST_CASE("state_variant rvalue ctx", "[new_fsm]") {

  auto fsm = StateMachine(mpl::type_identity<StatesCtx>{}, Ctx{});

  auto mono = fsm.state<std::monostate>();

  STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);

  fsm.emplace<myStatesCtx::Initial>();

  REQUIRE(fsm.is_in<myStatesCtx::Initial>());
  auto Initial = fsm.state<myStatesCtx::Initial>();

  STATIC_REQUIRE(std::is_same_v<decltype(Initial), myStatesCtx::Initial>);

  STATIC_REQUIRE(
      std::is_same_v<decltype(fsm.context()), decltype(Initial.context())>);
}

TEST_CASE("state_variant lvalue ctx", "[new_fsm]") {

  Ctx ctx(1);

  auto fsm = StateMachine(mpl::type_identity<StatesCtx>{}, ctx);

  auto mono = fsm.state<std::monostate>();

  STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);

  fsm.emplace<myStatesCtx::Initial>();

  REQUIRE(fsm.is_in<myStatesCtx::Initial>());
  auto Initial = fsm.state<myStatesCtx::Initial>();

  STATIC_REQUIRE(std::is_same_v<decltype(Initial), myStatesCtx::Initial>);

  STATIC_REQUIRE(
      std::is_same_v<decltype(fsm.context()), decltype(Initial.context())>);
}
