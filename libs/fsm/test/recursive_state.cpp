#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <fsm/recursive_state.h>
#include <variant>

#include "flat_fsm.h"

using namespace spie::fsm;

struct event1 {};
struct event2 {};

struct Initial;
struct Recursive;
struct Finished;
struct Error;

using Context = flat::Context;

using States = states<Initial, Recursive, Finished>;

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   void onEnter();

   auto transitionTo(const event1 &) { return transition<Recursive>(); }
   auto transitionTo(const event2 &) { return transition<Finished>(); }
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;

   void onEnter();
};

struct Error : state<Error, Machine> {

   using state<Error, Machine>::state;

   void onEnter();
};

struct Recursive : recursive_state<Recursive, Machine, Machine> {

   Recursive(Machine &sm) noexcept;

   void onEnter(const event1 &);
   auto transitionTo(const event1 &) const -> transitions<Finished, Error> {
      if (nested_in<Finished>()) {
         return transition<Finished>();
      }
      return transition<Error>();
   }
};

void Initial::onEnter() {
   context().is_valid(true);
   context().value(1);
}

Recursive::Recursive(Machine &sm) noexcept
    : recursive_state(Machine(mpl::type_identity<States>{}, Context{42}), sm) {
   nested_emplace<Initial>();
}

void Recursive::onEnter(const event1 &) { context().value(10); }

void Finished::onEnter() {
   context().is_valid(false);
   context().value(0);
}

void Error::onEnter() {
   context().is_valid(false);
   context().value(0);
}

// State Constructors

TEST_CASE("recusive_state", "[new_fsm]") {

   std::cout << "start" << std::endl;

   Context ctx_;

   auto fsm = Machine(mpl::type_identity<States>{}, ctx_);

   REQUIRE(fsm.is_in<std::monostate>());
   REQUIRE(&fsm.context() == &ctx_);

   REQUIRE(ctx_.is_valid() == false);
   REQUIRE(ctx_.value() == 0);

   fsm.emplace<Initial>();

   REQUIRE(fsm.is_in<Initial>());

   REQUIRE(ctx_.is_valid());
   REQUIRE(ctx_.value() == 1);

   std::cout << "dispatch event1, --> Recursive" << std::endl;

   auto result = fsm.dispatch(event1{});
   REQUIRE(result);

   // REQUIRE(result);
   REQUIRE(fsm.is_in<Recursive>());

   REQUIRE(ctx_.is_valid());
   REQUIRE(ctx_.value() == 10);

   REQUIRE(&ctx_ == &fsm.state<Recursive>().context());
   REQUIRE(fsm.state<Recursive>().nested()->context().is_valid());
   REQUIRE(fsm.state<Recursive>().nested()->context().value() == 1);

   auto nested = fsm.state<Recursive>().nested_state<Initial>();

   REQUIRE(nested.context().is_valid());
   REQUIRE(nested.context().value() == 1);

   result = fsm.dispatch(event2{});
   REQUIRE(result);

   REQUIRE(fsm.state<Recursive>().nested_in<Finished>());

   result = fsm.dispatch(event1{});
   REQUIRE(result);

   REQUIRE(fsm.is_in<Finished>());

   std::cout << "end" << std::endl;
}
