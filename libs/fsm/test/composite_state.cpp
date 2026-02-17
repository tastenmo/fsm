#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <fsm/composite_state.h>
#include <variant>

#include "flat_fsm.h"

using namespace spie::fsm;

namespace flat {

Initial::Initial(Machine &sm) noexcept : state(sm), count1(0), value2(0) {}

Second::Second(Machine &sm) noexcept : state(sm), count1(0) {}

void Second::onEnter() {
   count1++;
   machine_.context().is_valid(true);
   machine_.context().value(machine_.context().value() + 1);
}

Third::Third(Machine &sm) noexcept : state(sm), count1(0) {}

void Third::onEnter(const event2 &ev) {
   count1++;
   machine_.context().is_valid(false);
   machine_.context().value(10);
}

} // namespace flat

struct event1 {};

struct Initial;
struct CompositeRef;
struct Finished;
struct Error;

using Context = flat::Context;

using States = states<Initial, CompositeRef, Finished>;
using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   void onEnter();

   auto transitionTo(const event1 &) { return transition<CompositeRef>(); }
};

struct CompositeRef : composite_state<CompositeRef, flat::Machine, Machine> {

   CompositeRef(Machine &sm) noexcept;

   void onEnter(const event1 &);

   auto transitionTo(const event1 &) const -> transitions<Finished, Error> {
      if (nested_in<flat::Third>()) {
         return transition<Finished>();
      }
      return transition<Error>();
   }
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;

   void onEnter();
};

struct Error : state<Error, Machine> {

   using state<Error, Machine>::state;

   void onEnter();
};

void Initial::onEnter() {
   context().is_valid(true);
   context().value(1);
}

CompositeRef::CompositeRef(Machine &sm) noexcept
    : composite_state(
          flat::Machine(mpl::type_identity<flat::States>{}, sm.context()), sm) {
   nested_emplace<flat::Initial>();
}

void CompositeRef::onEnter(const event1 &) { context().value(10); }

void Finished::onEnter() {
   context().is_valid(false);
   context().value(0);
}

void Error::onEnter() {
   context().is_valid(false);
   context().value(0);
}

// State Constructors

TEST_CASE("composite_state basic context sharing and transitions",
          "[composite_state][basic]") {
   Context ctx;
   auto fsm = Machine(mpl::type_identity<States>{}, ctx);

   REQUIRE(fsm.is_in<std::monostate>());
   REQUIRE(&fsm.context() == &ctx);
   REQUIRE(ctx.is_valid() == false);
   REQUIRE(ctx.value() == 0);

   fsm.emplace<Initial>();
   REQUIRE(fsm.is_in<Initial>());
   REQUIRE(ctx.is_valid());
   REQUIRE(ctx.value() == 1);

   auto result = fsm.dispatch(event1{});
   REQUIRE(result);
   REQUIRE(fsm.is_in<CompositeRef>());
   REQUIRE(&ctx == &fsm.state<CompositeRef>().context());
   REQUIRE(&ctx == &fsm.state<CompositeRef>().nested().context());
   REQUIRE(ctx.is_valid());
   REQUIRE(ctx.value() == 10);

   // Nested state transitions
   result = fsm.dispatch(flat::event1{});
   REQUIRE(result);
   REQUIRE(fsm.state<CompositeRef>().nested_in<flat::Second>());
   REQUIRE(ctx.value() == 11);

   result = fsm.dispatch(flat::event2{2});
   REQUIRE(result);
   REQUIRE(fsm.state<CompositeRef>().nested_in<flat::Third>());
   REQUIRE(ctx.is_valid() == false);
   REQUIRE(ctx.value() == 10);

   result = fsm.dispatch(event1{});
   REQUIRE(result);
   REQUIRE(fsm.is_in<Finished>());
}

struct CustomInitial;
struct CustomComposite;
struct CustomFinished;

// using Context = flat::Context;

using CustomStates = states<CustomInitial, CustomComposite, CustomFinished>;
using CustomMachine = StateMachine<CustomStates, Context>;

struct CustomInitial : state<CustomInitial, CustomMachine> {
   using state<CustomInitial, CustomMachine>::state;

   void onEnter();

   auto transitionTo(const event1 &) { return transition<CustomComposite>(); }
};

struct CustomComposite
    : composite_state<CustomComposite, flat::Machine, CustomMachine> {
   bool handled_by_composite = false;
   CustomComposite(CustomMachine &sm) noexcept;

   void onEnter(const flat::event3 &ev);
};

struct CustomFinished : state<CustomFinished, CustomMachine> {
   using state<CustomFinished, CustomMachine>::state;

   void onEnter();
};

void CustomInitial::onEnter() {
   context().is_valid(true);
   context().value(1);
}

CustomComposite::CustomComposite(CustomMachine &sm) noexcept
    : composite_state(
          flat::Machine(mpl::type_identity<flat::States>{}, sm.context()), sm) {
   nested_emplace<flat::Initial>();
}

void CustomComposite::onEnter(const flat::event3 &ev) {
   handled_by_composite = true;
}

void CustomFinished::onEnter() {
   context().is_valid(false);
   context().value(0);
}

TEST_CASE("composite_state handles event if nested does not",
          "[composite_state][event_fallback]") {

   Context ctx;

   auto fsm = CustomMachine(mpl::type_identity<CustomStates>{}, ctx);
   fsm.emplace<CustomInitial>();
   REQUIRE(fsm.is_in<CustomInitial>());
   fsm.dispatch(event1{}); // transition to CustomComposite
   REQUIRE(fsm.is_in<CustomComposite>());

   auto &composite = fsm.state<CustomComposite>();
   REQUIRE(composite.nested_in<flat::Initial>());

   // Dispatch event3, which is not handled by nested machine
   // flat::event3 ev{"test"};
   auto result = fsm.dispatch(flat::event3{});
   REQUIRE(result); // Should be handled
   REQUIRE(
       composite
           .handled_by_composite); // Composite state's handler should be called
}
