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

TEST_CASE("composite_state separate context", "[new_fsm]") {

   std::cout << "start" << std::endl;

   MainContext ctx_;

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

   // Context is nor copied here????
   REQUIRE(fsm.context().is_valid());
   REQUIRE(fsm.context().value() == 10);

   REQUIRE(fsm.state<Composite>().nested_in<flat::Initial>());

   //  auto nested = fsm.state<Composite>().nested().context();

   CHECK(fsm.state<Composite>().nested_context().is_valid() == false);
   CHECK(fsm.state<Composite>().nested_context().value() == 42);

   result = fsm.dispatch(flat::event1{});
   REQUIRE(result);

   REQUIRE(fsm.state<Composite>().nested_in<flat::Second>());

   auto &nested_ctx = fsm.state<Composite>().nested_context();
   REQUIRE(nested_ctx.is_valid());
   REQUIRE(nested_ctx.value() == 43);

   result = fsm.dispatch(flat::event2{0});
   REQUIRE(result);

   REQUIRE(fsm.state<Composite>().nested_in<flat::Second>());

   auto &nested_ctx1 = fsm.state<Composite>().nested_context();
   REQUIRE(nested_ctx1.is_valid());
   REQUIRE(nested_ctx1.value() == 44);

   result = fsm.dispatch(flat::event2{2});
   REQUIRE(result);

   REQUIRE(fsm.state<Composite>().nested_in<flat::Third>());

   auto &nested_ctx2 = fsm.state<Composite>().nested_context();
   REQUIRE(nested_ctx2.is_valid() == false);
   REQUIRE(nested_ctx2.value() == 10);

   std::cout << "dispatch event1, --> Finished" << std::endl;

   result = fsm.dispatch(event1{});
   REQUIRE(result);

   REQUIRE(fsm.is_in<Finished>());

   std::cout << "end" << std::endl;
}
