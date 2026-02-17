#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <fsm/state_machine.h>
#include <variant>

using namespace spie::fsm;

struct noContext {};

struct event1 {};

// Forward declarations for states
struct Initial;
struct Second;
struct Third;

using States = states<Initial, Second, Third>;
using Machine = StateMachine<States, noContext>;

struct Initial : state<Initial, Machine> {
   using state<Initial, Machine>::state;

   void onEnter() { std::cout << "Initial" << std::endl; }
   auto transitionTo(const event1 &) { return transition<Second>(); }
};

struct Second : state<Second, Machine> {
   using state<Second, Machine>::state;
   void onEnter() { std::cout << "Second" << std::endl; }
   auto transitionTo(const event1 &) { return transition<Third>(); }
};

struct Third : state<Third, Machine> {
   using state<Third, Machine>::state;
   void onEnter() { std::cout << "Third" << std::endl; }
   auto transitionTo(const event1 &) { return transition<Initial>(); }
};

TEST_CASE("fsm_no_context_basic", "[fsm][no_context]") {
   auto fsm = Machine(mpl::type_identity<States>{}, noContext{});
   auto mono = fsm.state<std::monostate>();
   STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);
   fsm.emplace<Initial>();
   REQUIRE(fsm.is_in<Initial>());
   auto initial = fsm.state<Initial>();
   STATIC_REQUIRE(std::is_same_v<decltype(initial), Initial>);
   REQUIRE(fsm.valueless_by_exception() == false);
}

struct Ctx {
   int i = 0;
};

// Forward declarations for states
struct InitialCtx;
struct SecondCtx;
struct ThirdCtx;

using StatesCtx = states<InitialCtx, SecondCtx, ThirdCtx>;
using MachineCtx = StateMachine<StatesCtx, Ctx>;

struct InitialCtx : state<InitialCtx, MachineCtx> {
   using state<InitialCtx, MachineCtx>::state;
   void onEnter() { std::cout << "Initial" << std::endl; }
   auto transitionTo(const event1 &) { return transition<SecondCtx>(); }
};

struct SecondCtx : state<SecondCtx, MachineCtx> {
   using state<SecondCtx, MachineCtx>::state;
   void onEnter() { std::cout << "Second" << std::endl; }
   auto transitionTo(const event1 &) { return transition<ThirdCtx>(); }
};

struct ThirdCtx : state<ThirdCtx, MachineCtx> {
   using state<ThirdCtx, MachineCtx>::state;
   void onEnter() { std::cout << "Third" << std::endl; }
   auto transitionTo(const event1 &) { return transition<InitialCtx>(); }
};

TEST_CASE("fsm_stateful_rvalue_context", "[fsm][stateful][rvalue]") {
   auto fsm = MachineCtx(mpl::type_identity<StatesCtx>{}, Ctx{});
   auto mono = fsm.state<std::monostate>();
   STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);
   fsm.emplace<InitialCtx>();
   REQUIRE(fsm.is_in<InitialCtx>());
   auto initial = fsm.state<InitialCtx>();
   STATIC_REQUIRE(std::is_same_v<decltype(initial), InitialCtx>);
   STATIC_REQUIRE(
       std::is_same_v<decltype(fsm.context()), decltype(initial.context())>);
}

TEST_CASE("fsm_stateful_lvalue_context", "[fsm][stateful][lvalue]") {
   Ctx ctx{1};
   auto fsm =
       MachineCtx(mpl::type_identity<StatesCtx>{}, std::forward<Ctx>(ctx));
   auto mono = fsm.state<std::monostate>();
   STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);
   fsm.emplace<InitialCtx>();
   REQUIRE(fsm.is_in<InitialCtx>());
   auto initial = fsm.state<InitialCtx>();
   STATIC_REQUIRE(std::is_same_v<decltype(initial), InitialCtx>);
   STATIC_REQUIRE(
       std::is_same_v<decltype(fsm.context()), decltype(initial.context())>);
}

TEST_CASE("fsm_event_driven_cycle", "[fsm][event][cycle]") {
   auto fsm = Machine(mpl::type_identity<States>{}, noContext{});
   fsm.emplace<Initial>();
   REQUIRE(fsm.is_in<Initial>());
   // Dispatch event1, should transition to Second
   fsm.dispatch(event1{});
   REQUIRE(fsm.is_in<Second>());
   // Dispatch event1, should transition to Third
   fsm.dispatch(event1{});
   REQUIRE(fsm.is_in<Third>());
   // Dispatch event1, should transition back to Initial
   fsm.dispatch(event1{});
   REQUIRE(fsm.is_in<Initial>());
}
