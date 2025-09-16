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

   void enter() { std::cout << "Initial" << std::endl; }
   auto transition(const event1 &) { return transition<Second>(); }
};

struct Second : state<Second, Machine> {
   using state<Second, Machine>::state;
   void enter() { std::cout << "Second" << std::endl; }
   auto transition(const event1 &) { return transition<Third>(); }
};

struct Third : state<Third, Machine> {
   using state<Third, Machine>::state;
   void enter() { std::cout << "Third" << std::endl; }
   auto transition(const event1 &) { return transition<Initial>(); }
};

TEST_CASE("state_variant no ctx", "[new_fsm]") {

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
// using MachineLCtx = StateMachine<StatesCtx, Ctx &>;

struct InitialCtx : state<InitialCtx, MachineCtx> {
   using state<InitialCtx, MachineCtx>::state;
   void enter() { std::cout << "Initial" << std::endl; }
   auto transition(const event1 &) { return transition<SecondCtx>(); }
};

struct SecondCtx : state<SecondCtx, MachineCtx> {
   using state<SecondCtx, MachineCtx>::state;
   void enter() { std::cout << "Second" << std::endl; }
   auto transition(const event1 &) { return transition<ThirdCtx>(); }
};

struct ThirdCtx : state<ThirdCtx, MachineCtx> {
   using state<ThirdCtx, MachineCtx>::state;
   void enter() { std::cout << "Third" << std::endl; }
   auto transition(const event1 &) { return transition<InitialCtx>(); }
};

TEST_CASE("state_variant rvalue ctx", "[new_fsm]") {

   auto fsm = MachineCtx(mpl::type_identity<StatesCtx>{}, Ctx{});

   auto mono = fsm.state<std::monostate>();

   STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);

   fsm.emplace<InitialCtx>();

   REQUIRE(fsm.is_in<InitialCtx>());
   auto Initial = fsm.state<InitialCtx>();

   STATIC_REQUIRE(std::is_same_v<decltype(Initial), InitialCtx>);

   STATIC_REQUIRE(
       std::is_same_v<decltype(fsm.context()), decltype(Initial.context())>);
}

TEST_CASE("state_variant lvalue ctx", "[new_fsm]") {

   Ctx ctx(1);

   auto fsm =
       MachineCtx(mpl::type_identity<StatesCtx>{}, std::forward<Ctx>(ctx));

   auto mono = fsm.state<std::monostate>();

   STATIC_REQUIRE(std::is_same_v<decltype(mono), std::monostate>);

   fsm.emplace<InitialCtx>();

   REQUIRE(fsm.is_in<InitialCtx>());
   auto Initial = fsm.state<InitialCtx>();

   STATIC_REQUIRE(std::is_same_v<decltype(Initial), InitialCtx>);

   STATIC_REQUIRE(
       std::is_same_v<decltype(fsm.context()), decltype(Initial.context())>);
}
