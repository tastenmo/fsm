#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "core/utils.h"

#include <fsm/state_machine.h>

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

void Third::onEnter(const event2 &) {
   count1++;
   machine_.context().is_valid(false);
   machine_.context().value(10);
}

} // namespace flat

// State Constructors

TEST_CASE("fsm_context_basic_lvalue", "[fsm][context][basic]") {

   flat::Context ctx;
   auto fsm = flat::Machine(mpl::type_identity<flat::States>{}, ctx);
   fsm.emplace<flat::Initial>();
   REQUIRE(&ctx == &fsm.context());
   REQUIRE(fsm.is_in<flat::Initial>());
   fsm.dispatch(flat::event1{});
   REQUIRE(fsm.is_in<flat::Second>());
   fsm.dispatch(flat::event2{2});
   REQUIRE(fsm.is_in<flat::Third>());
}

TEST_CASE("fsm_context_basic_rvalue", "[fsm][context][basic][rvalue]") {

   auto fsm =
       flat::Machine(mpl::type_identity<flat::States>{}, flat::Context{42});
   fsm.emplace<flat::Initial>();
   REQUIRE(fsm.context().value() == 42);
   REQUIRE(fsm.is_in<flat::Initial>());
   fsm.dispatch(flat::event1{});
   REQUIRE(fsm.is_in<flat::Second>());
   fsm.dispatch(flat::event2{2});
   REQUIRE(fsm.is_in<flat::Third>());
}
