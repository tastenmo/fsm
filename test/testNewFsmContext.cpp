#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include "base/utils.h"

#include <new_fsm/initial_state.h>

#include "flat_fsm.h"

using namespace escad::new_fsm;

// State Constructors

auto myStatePrinter = escad::overloaded{
    [](flat::Initial &) { std::cout << "flat::Initial" << std::endl; },
    [](flat::Second &) { std::cout << "flat::Second" << std::endl; },
    [](flat::Third &) { std::cout << "flat::Third" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

TEST_CASE("Context reference", "[new_fsm]") {

  std::cout << "start" << std::endl;

  flat::Context ctx_;

  auto fsm = flat::Initial::create(ctx_);

  REQUIRE(&ctx_ == &fsm.context());

  std::cout << "fsm constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx1 = fsm.context();

  REQUIRE(&ctx_ != &ctx1);

  flat::Context &ctx2 = fsm.context();
  REQUIRE(&ctx_ == &ctx2);

  auto &ctx = fsm.context();

  REQUIRE(&ctx == &ctx_);

  std::cout << "after fsm.context()" << std::endl;

  REQUIRE_FALSE(ctx.is_valid());
  REQUIRE(ctx.value() == 0);

  REQUIRE(fsm.is_in<flat::Initial>());

  REQUIRE(&ctx == &fsm.state<flat::Initial>().ctx_);
  REQUIRE_FALSE(fsm.state<flat::Initial>().ctx_.is_valid());
  REQUIRE(fsm.state<flat::Initial>().ctx_.value() == 0);

  auto result = fsm.dispatch(flat::event1{});

  // REQUIRE(result);
  REQUIRE(fsm.is_in<flat::Second>());

  // Context is nor copied here????
  REQUIRE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 1);

  auto state2 = fsm.state<flat::Second>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid());
  REQUIRE(state2.ctx_.value() == 1);

  // state2.dispatch(event2{2});
  auto result2 = fsm.dispatch(flat::event2{2});

  REQUIRE(fsm.is_in<flat::Third>());
  REQUIRE_FALSE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 10);
}

TEST_CASE("Context instantiated reference", "[new_fsm]") {

  flat::Context ctx_(42);

  auto fsm = flat::Initial::create(ctx_);

  REQUIRE(&ctx_ == &fsm.context());

  auto &ctx = fsm.context();

  REQUIRE(&ctx == &ctx_);

  REQUIRE_FALSE(ctx.is_valid());
  REQUIRE(ctx.value() == 42);

  REQUIRE(fsm.is_in<flat::Initial>());

  REQUIRE(&ctx == &fsm.state<flat::Initial>().ctx_);
  REQUIRE_FALSE(fsm.state<flat::Initial>().ctx_.is_valid());
  REQUIRE(fsm.state<flat::Initial>().ctx_.value() == 42);

  auto result = fsm.dispatch(flat::event1{});

  // REQUIRE(result);
  REQUIRE(fsm.is_in<flat::Second>());

  // Context is nor copied here????
  REQUIRE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 43);

  auto state2 = fsm.state<flat::Second>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid());
  REQUIRE(state2.ctx_.value() == 43);

  // state2.dispatch(event2{2});
  auto result2 = fsm.dispatch(flat::event2{2});

  REQUIRE(fsm.is_in<flat::Third>());
  REQUIRE_FALSE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 10);
}

TEST_CASE("Context implicit", "[new_fsm]") {

  auto fsm = flat::Initial::create(flat::Context{42});

  auto &ctx = fsm.context();

  REQUIRE_FALSE(ctx.is_valid());
  REQUIRE(ctx.value() == 42);

  REQUIRE(fsm.is_in<flat::Initial>());

  REQUIRE(&ctx == &fsm.state<flat::Initial>().ctx_);
  REQUIRE_FALSE(fsm.state<flat::Initial>().ctx_.is_valid());
  REQUIRE(fsm.state<flat::Initial>().ctx_.value() == 42);

  auto result = fsm.dispatch(flat::event1{});

  // REQUIRE(result);
  REQUIRE(fsm.is_in<flat::Second>());

  // Context is nor copied here????
  REQUIRE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 43);

  auto state2 = fsm.state<flat::Second>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid());
  REQUIRE(state2.ctx_.value() == 43);

  // state2.dispatch(event2{2});
  auto result2 = fsm.dispatch(flat::event2{2});

  REQUIRE(fsm.is_in<flat::Third>());
  REQUIRE_FALSE(fsm.context().is_valid());
  REQUIRE(fsm.context().value() == 10);
}
