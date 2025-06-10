#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include <new_fsm/composite_state.h>
#include <variant>

#include "flat_fsm.h"
#include "new_fsm/transition.h"

using namespace escad::new_fsm;

struct event1 {};

struct Initial;
struct CompositeRef;
struct CompositeOwn;
struct Finished;
struct Error;

using Context = flat::Context;

using States = states<Initial, CompositeRef, CompositeOwn, Finished>;

struct Initial : state<Initial, Context> {

  void onEnter() {
    context_.is_valid(true);
    context_.value(1);
  }

  auto transitionTo(const event1 &) { return sibling<CompositeRef>(); }
};

struct CompositeRef
    : composite_state<CompositeRef, flat::MachineWithReferenceContext,
                      Context> {

  CompositeRef(Context &ctx) noexcept
      : composite_state(ctx, flat::MachineWithReferenceContext(
                                 mpl::type_identity<flat::States>{}, ctx)) {
    nested_emplace<flat::Initial>();
  }

  void onEnter(const event1 &) { context_.value(10); }

  auto transitionTo(const event1 &) const -> transitions<CompositeOwn, Error> {
    if (nested_in<flat::Third>()) {
      return sibling<CompositeOwn>();
    }
    return sibling<Error>();
  }
};

struct CompositeOwn
    : composite_state<CompositeOwn, flat::MachineWithOwnContext, Context> {

  CompositeOwn(Context &ctx) noexcept
      : composite_state(
            ctx, flat::MachineWithOwnContext(mpl::type_identity<flat::States>{},
                                             flat::Context{})) {
    nested_emplace<flat::Initial>();
  }

  void onEnter(const event1 &) {
    context_.is_valid(true);
    context_.value(15);
  }

  auto transitionTo(const event1 &) const -> transitions<Finished, Error> {
    if (nested_in<flat::Third>()) {
      return sibling<Finished>();
    }
    return sibling<Error>();
  }
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

  std::cout << "dispatch event1, --> CompositeRef" << std::endl;

  auto result = fsm.dispatch(event1{});
  REQUIRE(result);

  // REQUIRE(result);
  REQUIRE(fsm.is_in<CompositeRef>());

  REQUIRE(&ctx_ == &fsm.state<CompositeRef>().context());
  REQUIRE(&ctx_ == &fsm.state<CompositeRef>().nested().context());

  // Context is nor copied here????
  REQUIRE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 10);

  auto nested = fsm.state<CompositeRef>().nested_state<flat::Initial>();

  REQUIRE(nested.context().is_valid());
  REQUIRE(nested.context().value() == 10);

  result = fsm.dispatch(flat::event1{});
  REQUIRE(result);

  REQUIRE(fsm.state<CompositeRef>().nested_in<flat::Second>());

  REQUIRE(ctx_.value() == 11);

  result = fsm.dispatch(flat::event2{0});
  REQUIRE(result);

  REQUIRE(fsm.state<CompositeRef>().nested_in<flat::Second>());
  REQUIRE(ctx_.value() == 12);

  result = fsm.dispatch(flat::event2{2});
  REQUIRE(result);

  REQUIRE(fsm.state<CompositeRef>().nested_in<flat::Third>());
  REQUIRE(ctx_.is_valid() == false);
  REQUIRE(ctx_.value() == 10);

  std::cout << "dispatch event1, --> CompositeOwn" << std::endl;

  result = fsm.dispatch(event1{});
  REQUIRE(result);

  REQUIRE(fsm.is_in<CompositeOwn>());
  REQUIRE(ctx_.is_valid());
  REQUIRE(ctx_.value() == 15);

  REQUIRE(&ctx_ == &fsm.state<CompositeOwn>().context());
  REQUIRE(fsm.state<CompositeOwn>().nested().context().is_valid() == false);
  REQUIRE(fsm.state<CompositeOwn>().nested().context().value() == 0);

  result = fsm.dispatch(flat::event1{});
  REQUIRE(result);

  REQUIRE(fsm.state<CompositeOwn>().nested_in<flat::Second>());

  REQUIRE(ctx_.is_valid());
  REQUIRE(ctx_.value() == 15);

  REQUIRE(fsm.state<CompositeOwn>().nested().context().value() == 1);
  REQUIRE(fsm.state<CompositeOwn>().nested().context().is_valid());

  result = fsm.dispatch(flat::event2{2});
  REQUIRE(result);

  REQUIRE(fsm.state<CompositeOwn>().nested_in<flat::Third>());
  REQUIRE(ctx_.is_valid());
  REQUIRE(ctx_.value() == 15);

  REQUIRE(fsm.state<CompositeOwn>().nested().context().value() == 10);
  REQUIRE(fsm.state<CompositeOwn>().nested().context().is_valid() == false);

  std::cout << "dispatch event1, --> Finished" << std::endl;

  result = fsm.dispatch(event1{});
  REQUIRE(result);

  REQUIRE(fsm.is_in<Finished>());

  std::cout << "end" << std::endl;
}
