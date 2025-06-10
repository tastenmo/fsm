#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include <new_fsm/recursive_state.h>
#include <variant>

#include "flat_fsm.h"

using namespace escad::new_fsm;

struct event1 {};
struct event2 {};

struct Initial;
struct Recursive;
struct Finished;
struct Error;

using Context = flat::Context;

using States = states<Initial, Recursive, Finished>;

using MachineWithOwnContext = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  void onEnter() {
    context_.is_valid(true);
    context_.value(1);
  }

  auto transitionTo(const event1 &) { return sibling<Recursive>(); }
  auto transitionTo(const event2 &) { return sibling<Finished>(); }
};

struct Finished : state<Finished, Context> {

  void onEnter() {
    context_.is_valid(false);
    context_.value(0);
  }
};

struct Error : state<Error, Context> {

  void onEnter() {
    context_.is_valid(false);
    context_.value(0);
  }
};

struct Recursive : recursive_state<Recursive, MachineWithOwnContext, Context> {

  Recursive(Context &ctx) noexcept
      : recursive_state(ctx, MachineWithOwnContext(mpl::type_identity<States>{},
                                                   flat::Context{})) {
    nested_emplace<Initial>();
  }

  void onEnter(const event1 &) { context_.value(10); }

  auto transitionTo(const event1 &) const -> transitions<Finished, Error> {
    if (nested_in<Finished>()) {
      return sibling<Finished>();
    }
    return sibling<Error>();
  }
};

// State Constructors

TEST_CASE("basic composite with common context", "[new_fsm]") {

  std::cout << "start" << std::endl;

  Context ctx_;

  auto fsm = StateMachine(mpl::type_identity<States>{}, ctx_);

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
