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
struct Composite;
struct Finished;
struct Error;

class MainContext {
 public:
   MainContext() : is_valid_(false), value_(0) {}
   MainContext(int val) : is_valid_(false), value_(val) {}

   MainContext(MainContext &&) = delete;
   MainContext(MainContext const &) = delete;
   MainContext &operator=(MainContext &&) = delete;
   MainContext &operator=(MainContext const &) = delete;

   bool is_valid() const { return is_valid_; }
   void is_valid(bool v) { is_valid_ = v; }

   int value() const { return value_; }
   void value(int v) { value_ = v; }

 protected:
   bool is_valid_;
   int value_;
};

using States = states<Initial, Composite, Finished>;
using Machine = StateMachine<States, MainContext>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   void onEnter();

   auto transitionTo(const event1 &) { return transition<Composite>(); }
};

struct Composite : composite_state<Composite, flat::Machine, Machine> {

   Composite(Machine &sm) noexcept;

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

Composite::Composite(Machine &sm) noexcept
    : composite_state(
          flat::Machine(mpl::type_identity<flat::States>{}, flat::Context{42}),
          sm) {
   nested_emplace<flat::Initial>();
}

void Composite::onEnter(const event1 &) { context().value(10); }

void Finished::onEnter() {
   context().is_valid(false);
   context().value(0);
}

void Error::onEnter() {
   context().is_valid(false);
   context().value(0);
}

// State Constructors

TEST_CASE("composite_state_separate_context_basic",
          "[fsm][composite][context][separate]") {

   MainContext ctx;
   auto fsm = Machine(mpl::type_identity<States>{}, ctx);
   REQUIRE(&fsm.context() == &ctx);
   REQUIRE(fsm.is_in<std::monostate>());
   fsm.emplace<Initial>();
   REQUIRE(fsm.is_in<Initial>());
   fsm.dispatch(event1{});
   REQUIRE(fsm.is_in<Composite>());
   fsm.dispatch(flat::event1{});
   REQUIRE(fsm.state<Composite>().nested_in<flat::Second>());
   fsm.dispatch(flat::event2{2});
   REQUIRE(fsm.state<Composite>().nested_in<flat::Third>());
   fsm.dispatch(event1{});
   REQUIRE(fsm.is_in<Finished>());
}
