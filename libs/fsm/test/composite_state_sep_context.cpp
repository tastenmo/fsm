#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <fsm/composite_state.h>
#include <variant>

#include "flat_fsm.h"

using namespace escad::new_fsm;

namespace flat {

Initial::Initial(Machine &sm) noexcept
      : state(sm), count1(0), value2(0) {}

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

}

struct event1 {};

struct Initial;
struct Composite;
struct Finished;
struct Error;

using Context = flat::Context;

using States = states<Initial, Composite, Finished>;
using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

    using state<Initial, Machine>::state;
    
  void onEnter();

  auto transitionTo(const event1 &) { return sibling<Composite>(); }
};

struct Composite
    : composite_state<Composite, flat::Machine, Machine> {

  Composite(Machine &sm) noexcept;

  void onEnter(const event1 &);

  auto transitionTo(const event1 &) const -> transitions<Finished, Error> {
    if (nested_in<flat::Third>()) {
      return sibling<Finished>();
    }
    return sibling<Error>();
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
      : composite_state(flat::Machine(
                                 mpl::type_identity<flat::States>{}, flat::Context{42}), sm) {
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

TEST_CASE("composite_state", "[new_fsm]") {

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

  std::cout << "dispatch event1, --> Composite" << std::endl;

  auto result = fsm.dispatch(event1{});
  REQUIRE(result);

  // REQUIRE(result);
  REQUIRE(fsm.is_in<Composite>());

  REQUIRE(&ctx_ == &fsm.state<Composite>().context());
  REQUIRE_FALSE(&ctx_ == &fsm.state<Composite>().nested().context());

  // Context is nor copied here????
  REQUIRE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 10);

  auto nested = fsm.state<Composite>().nested();

  CHECK(nested.context().is_valid());
  CHECK(nested.context().value() == 42);

  result = fsm.dispatch(flat::event1{});
  REQUIRE(result);

  REQUIRE(fsm.state<Composite>().nested_in<flat::Second>());

  REQUIRE(ctx_.value() == 11);

  result = fsm.dispatch(flat::event2{0});
  REQUIRE(result);

  REQUIRE(fsm.state<Composite>().nested_in<flat::Second>());
  REQUIRE(ctx_.value() == 12);

  result = fsm.dispatch(flat::event2{2});
  REQUIRE(result);

  REQUIRE(fsm.state<Composite>().nested_in<flat::Third>());
  REQUIRE(ctx_.is_valid() == false);
  REQUIRE(ctx_.value() == 10);



  std::cout << "dispatch event1, --> Finished" << std::endl;

  result = fsm.dispatch(event1{});
  REQUIRE(result);

  REQUIRE(fsm.is_in<Finished>());

  std::cout << "end" << std::endl;
}
