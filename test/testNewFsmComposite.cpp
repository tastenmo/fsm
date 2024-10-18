#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include "base/utils.h"

#include <new_fsm/composite_state.h>

#include "flat_fsm.h"

using namespace escad::new_fsm;

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct Initial;
struct First;
struct Second;
struct Composite;

using Context = flat::Context;

using States = states<Initial, First, Second, Composite>;

using StateContainer = state_variant<States, Context &>;

struct Initial : initial_state<Initial, StateContainer, Context> {

  // using state<Initial, StateContainer>::state;

  Initial(Context &ctx) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return sibling<First>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<First>(const event1 &);

  int count1;
  int value2;

  Context &ctx_;
};

struct First : state<First, Context> {

  //  using state<First, StateContainer>::state;

  First(Context &ctx) noexcept;

  void onEnter() {
    count1++;
    ctx_.is_valid(true);
    ctx_.value(ctx_.value() + 1);
  }

  auto transitionTo(const event1 &) { return sibling<Composite>(); }

  auto transitionTo(const event2 &event) const
      -> transitions<Initial, First, Second> {
    if (event.value_ == 1) {
      return sibling<Initial>();
    } else if (event.value_ == 2) {
      return sibling<Second>();
    }
    // handled() does not work yet
    return sibling<First>();
  }

  // auto transitionTo(const event1 &) const { return handled(); }

  int count1;

  Context &ctx_;
};

struct Second : state<Second, Context> {

  Second(Context &ctx) noexcept;

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid(false);
    if (ev.value_ == 2) {
      ctx_.value(10);
      // state_container_.emplace<Initial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // Second() : count1(0) {}

  int count1;
  Context &ctx_;
};

struct Composite : composite_state<Composite, flat::StateInitial, Context> {

  Composite(Context &ctx) noexcept
      : composite_state(ctx), count1(0), ctx_(ctx) {
    std::cout << "Composite::Composite(Context &ctx)" << std::endl;
  }

  Composite(const Composite &other)
      : composite_state(other.ctx_), count1(other.count1), ctx_(other.ctx_) {
    std::cout << "Composite::Composite(const Composite &other)" << std::endl;
  }

  Composite(Composite &&other) noexcept
      : composite_state(other.ctx_), count1(std::move(other.count1)),
        ctx_(other.ctx_) {
    std::cout << "Composite::Composite(Composite &&other)" << std::endl;
  }

  ~Composite() { std::cout << "Composite::~Composite()" << std::endl; }

  // Composite(Context &ctx) noexcept;

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid(false);
    if (ev.value_ == 2) {
      ctx_.value(10);
      // state_container_.emplace<Initial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // Second() : count1(0) {}

  int count1;
  Context &ctx_;
};

// State Constructors

auto myStatePrinter = escad::overloaded{
    [](Initial &) { std::cout << "Initial" << std::endl; },
    [](First &) { std::cout << "First" << std::endl; },
    [](Second &) { std::cout << "Second" << std::endl; },
    [](Composite &) { std::cout << "Composite" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

Initial::Initial(Context &ctx) noexcept
    : initial_state(ctx), count1(0), value2(0), ctx_(ctx) {}

First::First(Context &ctx) noexcept : state(ctx), count1(0), ctx_(ctx) {}

Second::Second(Context &ctx) noexcept : state(ctx), count1(0), ctx_(ctx) {}

TEST_CASE("basic composite", "[new_fsm]") {

  std::cout << "start" << std::endl;

  Context ctx_;

  auto myStates = Initial::create(ctx_);

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  // auto ctx = myStates.context();

  // std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx_.is_valid() == false);
  REQUIRE(ctx_.value() == 0);

  // myStates.emplace<Initial>();

  REQUIRE(myStates.is_in<Initial>());

  auto result = myStates.dispatch(event1{});

  // REQUIRE(result);
  REQUIRE(myStates.is_in<First>());

  // Context is nor copied here????
  REQUIRE(myStates.context().is_valid() == true);
  REQUIRE(myStates.context().value() == 1);

  auto state2 = myStates.state<First>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(ctx_.is_valid() == true);
  REQUIRE(ctx_.value() == 1);

  std::cout << "before myStates.dispatch(event1{})" << std::endl;

  // state2.dispatch(event2{2});
  auto result2 = myStates.dispatch(event1{});

  REQUIRE(result2);

  REQUIRE(myStates.is_in<Composite>());

  std::cout << "before myStates.state<Composite>()" << std::endl;

  // auto comp1 = myStates.state<Composite>();

  REQUIRE(myStates.state<Composite>().nested_in<flat::StateInitial>());

  auto result3 = myStates.dispatch(flat::event1{});

  REQUIRE(result3);

  REQUIRE(myStates.is_in<Composite>());

  std::cout << "before myStates.state<Composite>()" << std::endl;

  // auto comp2 = myStates.state<Composite>();

  REQUIRE(myStates.state<Composite>().nested_in<flat::StateSecond>());
}

/*
struct StateCtxInitial;
struct StateCtxSecond;
struct StateCtxThird;

using CtxStates = states<StateCtxInitial, StateCtxSecond, StateCtxThird>;

using StateCtxContainer = state_variant<CtxStates, Context &>;

struct StateCtxInitial : initial_state<StateCtxInitial, StateCtxContainer> {

  // using state<Initial, StateContainer>::state;

  StateCtxInitial(StateCtxContainer &state_container) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }


  auto transitionTo(const event1 &) { return trans<StateCtxSecond>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<First>(const event1 &);

  int count1;
  int value2;
};

struct StateCtxSecond : state<StateCtxSecond, StateCtxContainer> {

  //  using state<First, StateContainer>::state;

  StateCtxSecond(StateCtxContainer &state_container, Context &ctx) noexcept;

  void onEnter() {
    count1++;
    ctx_.is_valid = true;
    ctx_.value += 1;
  }

  auto transitionTo(const event2 &event) const
      -> transitions<StateCtxInitial, StateCtxSecond, StateCtxThird> {
    if (event.value_ == 1) {
      return trans<StateCtxInitial>();
    } else if (event.value_ == 2) {
      return trans<StateCtxThird>();
    }
    // handled() does not work yet
    return trans<StateCtxSecond>();
  }

  auto transitionTo(const event1 &) const { return handled(); }

  int count1;

  Context &ctx_;
};

struct StateCtxThird : state<StateCtxThird, StateCtxContainer> {

  StateCtxThird(StateCtxContainer &state_container, Context &ctx) noexcept;

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid = false;
    if (ev.value_ == 2) {
      ctx_.value = 10;
      state_container_.emplace<StateCtxInitial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // Second() : count1(0) {}

  int count1;
  Context &ctx_;
};

// State Constructors

auto myStateCtxPrinter = escad::overloaded{
    [](StateCtxInitial &) { std::cout << "StateCtxInitial" << std::endl; },
    [](StateCtxSecond &) { std::cout << "StateCtxSecond" << std::endl; },
    [](StateCtxThird &) { std::cout << "StateCtxThird" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

StateCtxInitial::StateCtxInitial(StateCtxContainer &state_container) noexcept
    : initial_state(state_container), count1(0), value2(0) {}

StateCtxSecond::StateCtxSecond(StateCtxContainer &state_container,
                               Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

StateCtxThird::StateCtxThird(StateCtxContainer &state_container,
                             Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

TEST_CASE("with explicit Context", "[new_fsm]") {

  std::cout << "start" << std::endl;

  Context myContext(20);

  std::cout << "myContext constructed" << std::endl;

  auto myStates = StateCtxInitial::create(myContext);

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx = myStates.context();

  std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx.is_valid == false);
  REQUIRE(ctx.value == 20);

  myStates.emplace<StateCtxInitial>();

  REQUIRE(myStates.is_in<StateCtxInitial>());

  auto result = myStates.handle(event1{});

  REQUIRE(result);
  REQUIRE(myStates.is_in<StateCtxSecond>());

  // Context is nor copied here????
  REQUIRE(myStates.context().is_valid == true);
  REQUIRE(myStates.context().value == 21);

  auto state2 = myStates.state<StateCtxSecond>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid == true);
  REQUIRE(state2.ctx_.value == 21);
}

struct StateCtx1Initial;
struct StateCtx1Second;
struct StateCtx1Third;

using Ctx1States = states<StateCtx1Initial, StateCtx1Second, StateCtx1Third>;

using StateCtx1Container = state_variant<Ctx1States, Context &&>;

struct StateCtx1Initial : initial_state<StateCtx1Initial, StateCtx1Container> {

  // using state<Initial, StateContainer>::state;

  StateCtx1Initial(StateCtx1Container &state_container) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }


  auto transitionTo(const event1 &) { return trans<StateCtx1Second>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<First>(const event1 &);

  int count1;
  int value2;
};

struct StateCtx1Second : state<StateCtx1Second, StateCtx1Container> {

  //  using state<First, StateContainer>::state;

  StateCtx1Second(StateCtx1Container &state_container, Context &ctx) noexcept;

  void onEnter() {
    count1++;
    ctx_.is_valid = true;
    ctx_.value += 1;
  }

  auto transitionTo(const event2 &event) const
      -> transitions<StateCtx1Initial, StateCtx1Second, StateCtx1Third> {
    if (event.value_ == 1) {
      return trans<StateCtx1Initial>();
    } else if (event.value_ == 2) {
      return trans<StateCtx1Third>();
    }
    // handled() does not work yet
    return trans<StateCtx1Second>();
  }

  auto transitionTo(const event1 &) const { return handled(); }

  int count1;

  Context &ctx_;
};

struct StateCtx1Third : state<StateCtx1Third, StateCtx1Container> {

  StateCtx1Third(StateCtx1Container &state_container, Context &ctx) noexcept;

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid = false;
    if (ev.value_ == 2) {
      ctx_.value = 10;
      state_container_.emplace<StateCtx1Initial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // Second() : count1(0) {}

  int count1;
  Context &ctx_;
};

// State Constructors

auto myStateCtx1Printer = escad::overloaded{
    [](StateCtx1Initial &) { std::cout << "StateCtx1Initial" << std::endl; },
    [](StateCtx1Second &) { std::cout << "StateCtx1Second" << std::endl; },
    [](StateCtx1Third &) { std::cout << "StateCtx1Third" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

StateCtx1Initial::StateCtx1Initial(StateCtx1Container &state_container) noexcept
    : initial_state(state_container), count1(0), value2(0) {}

StateCtx1Second::StateCtx1Second(StateCtx1Container &state_container,
                                 Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

StateCtx1Third::StateCtx1Third(StateCtx1Container &state_container,
                               Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

TEST_CASE("with explicit&& Context", "[new_fsm]") {

  std::cout << "start" << std::endl;

  std::cout << "myContext constructed" << std::endl;

  auto myStates = StateCtx1Initial::create(Context{30});

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx = myStates.context();

  std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx.is_valid == false);
  REQUIRE(ctx.value == 30);

  myStates.emplace<StateCtx1Initial>();

  REQUIRE(myStates.is_in<StateCtx1Initial>());

  auto result = myStates.handle(event1{});

  REQUIRE(result);
  REQUIRE(myStates.is_in<StateCtx1Second>());

  // Context is nor copied here????
  REQUIRE(myStates.context().is_valid == true);
  REQUIRE(myStates.context().value == 31);

  auto state2 = myStates.state<StateCtx1Second>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid == true);
  REQUIRE(state2.ctx_.value == 31);
}
*/