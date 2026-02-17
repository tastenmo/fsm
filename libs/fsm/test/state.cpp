#include <catch2/catch_test_macros.hpp>
#include <fsm/state.h>

using namespace spie::fsm;

struct DummyMachine {
   int context_ = 42;
   int &context() { return context_; }
   template <class Event> void dispatch(const Event &) {}
};

struct StateNoMethods : state<StateNoMethods, DummyMachine> {
   using state<StateNoMethods, DummyMachine>::state;
};

struct StateWithOnEnter : state<StateWithOnEnter, DummyMachine> {
   using state<StateWithOnEnter, DummyMachine>::state;
   bool entered = false;
   void onEnter() { entered = true; }
};

struct StateWithOnExit : state<StateWithOnExit, DummyMachine> {
   using state<StateWithOnExit, DummyMachine>::state;
   bool exited = false;
   void onExit() { exited = true; }
};

struct StateWithTransition : state<StateWithTransition, DummyMachine> {
   using state<StateWithTransition, DummyMachine>::state;
   bool transitioned = false;
   struct DummyEvent {};
   auto transitionTo(const DummyEvent &) {
      transitioned = true;
      return transition<StateNoMethods>();
   }
};

struct StateWithTransitionInternal
    : state<StateWithTransitionInternal, DummyMachine> {
   using state<StateWithTransitionInternal, DummyMachine>::state;
   bool called = false;
   auto transitionInternalTo() {
      called = true;
      return transition<StateNoMethods>();
   }
};

struct StateWithOnEnterEvent : state<StateWithOnEnterEvent, DummyMachine> {
   using state<StateWithOnEnterEvent, DummyMachine>::state;
   bool entered_event = false;
   struct DummyEvent {};
   void onEnter(const DummyEvent &) { entered_event = true; }
};

struct StateWithBothOnEnter : state<StateWithBothOnEnter, DummyMachine> {
   using state<StateWithBothOnEnter, DummyMachine>::state;
   bool entered = false;
   bool entered_event = false;
   void onEnter() { entered = true; }
   struct DummyEvent {};
   void onEnter(const DummyEvent &) { entered_event = true; }
};

TEST_CASE("state_no_methods_basic", "[fsm][state]") {
   DummyMachine m;
   StateNoMethods s(m);
   REQUIRE_FALSE(s.enter());
   REQUIRE_FALSE(s.exit());
}

TEST_CASE("state_on_enter_called", "[fsm][state][onEnter]") {
   DummyMachine m;
   StateWithOnEnter s(m);
   REQUIRE(s.enter());
   REQUIRE(s.entered);
}

TEST_CASE("state_on_exit_called", "[fsm][state][onExit]") {
   DummyMachine m;
   StateWithOnExit s(m);
   REQUIRE(s.exit());
   REQUIRE(s.exited);
}

TEST_CASE("state_transition_to_event", "[fsm][state][transitionTo]") {
   DummyMachine m;
   StateWithTransition s(m);
   StateWithTransition::DummyEvent e;
   auto result = s.trans(e);
   REQUIRE(s.transitioned);
   REQUIRE(result.is_transition());
}

TEST_CASE("state_transition_internal_to",
          "[fsm][state][transitionInternalTo]") {
   DummyMachine m;
   StateWithTransitionInternal s(m);
   auto result = s.transInternal();
   REQUIRE(s.called);
   REQUIRE(result.is_transition());
}

TEST_CASE("state_context_and_machine_access", "[fsm][state][context]") {
   DummyMachine m;
   StateNoMethods s(m);
   REQUIRE(s.context() == 42);
   REQUIRE(&s.machine() == &m);
}

TEST_CASE("state_on_enter_event_called", "[fsm][state][onEnter][event]") {
   DummyMachine m;
   StateWithOnEnterEvent s(m);
   StateWithOnEnterEvent::DummyEvent e;
   REQUIRE(s.enter(e));
   REQUIRE(s.entered_event);
}

TEST_CASE("state_on_enter_both_called", "[fsm][state][onEnter][both]") {
   DummyMachine m;
   StateWithBothOnEnter s(m);
   StateWithBothOnEnter::DummyEvent e;
   REQUIRE(s.enter());
   REQUIRE(s.entered);
   REQUIRE(s.enter(e));
   REQUIRE(s.entered_event);
}
