#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include "base/utils.h"

// #include <new_fsm/machine.h>
#include <new_fsm/state.h>
#include <new_fsm/state_variant.h>

using namespace escad::new_fsm;

struct Context {
  bool is_valid = false;

  int value = 0;

  Context() { std::cout << "Context::Context()" << std::endl; }

  Context(int val) : value(val) {
    std::cout << "Context::Context(int val)" << std::endl;
  }

  Context(const Context &other) : is_valid(other.is_valid), value(other.value) {
    std::cout << "Context::Context(const Context &other)" << std::endl;
  }

  Context(Context &&other) noexcept
      : is_valid(std::move(other.is_valid)), value(std::move(other.value)) {
    std::cout << "Context::Context(Context &&other)" << std::endl;
  }

  ~Context() { std::cout << "Context::~Context()" << std::endl; }
};

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct StateInitial;
struct StateSecond;
struct StateThird;

using States = states<StateInitial, StateSecond, StateThird>;

using StateContainer = detail::state_variant<States, Context>;

struct StateInitial : state<StateInitial, StateContainer> {

  // using state<StateInitial, StateContainer>::state;

  StateInitial(StateContainer &state_container) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return trans<StateSecond>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateSecond : state<StateSecond, StateContainer> {

  //  using state<StateSecond, StateContainer>::state;

  StateSecond(StateContainer &state_container, Context &ctx) noexcept;

  void onEnter() {
    count1++;
    ctx_.is_valid = true;
    ctx_.value += 1;
  }

  auto transitionTo(const event2 &event) const
      -> transitions<StateInitial, StateSecond, StateThird> {
    if (event.value_ == 1) {
      return trans<StateInitial>();
    } else if (event.value_ == 2) {
      return trans<StateThird>();
    }
    // handled() does not work yet
    return trans<StateSecond>();
  }

  auto transitionTo(const event1 &) const { return handled(); }

  int count1;

  Context &ctx_;
};

struct StateThird : state<StateThird, StateContainer> {

  StateThird(StateContainer &state_container, Context &ctx) noexcept;

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid = false;
    if (ev.value_ == 2) {
      ctx_.value = 10;
      state_container_.emplace<StateInitial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // StateThird() : count1(0) {}

  int count1;
  Context &ctx_;
};

// State Constructors

auto myStatePrinter = escad::overloaded{
    [](StateInitial &) { std::cout << "StateInitial" << std::endl; },
    [](StateSecond &) { std::cout << "StateSecond" << std::endl; },
    [](StateThird &) { std::cout << "StateThird" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

StateInitial::StateInitial(StateContainer &state_container) noexcept
    : state(state_container), count1(0), value2(0) {}

StateSecond::StateSecond(StateContainer &state_container, Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

StateThird::StateThird(StateContainer &state_container, Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

TEST_CASE("with implicit Context", "[new_fsm]") {

  std::cout << "start" << std::endl;

  auto myStates = StateContainer{};

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx = myStates.context();

  std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx.is_valid == false);
  REQUIRE(ctx.value == 0);

  myStates.emplace<StateInitial>();

  REQUIRE(myStates.is_in<StateInitial>());

  auto result = myStates.dispatch(event1{});

  REQUIRE(result);
  REQUIRE(myStates.is_in<StateSecond>());

  // Context is nor copied here????
  REQUIRE(myStates.context().is_valid == true);
  REQUIRE(myStates.context().value == 1);

  auto state2 = myStates.state<StateSecond>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid == true);
  REQUIRE(state2.ctx_.value == 1);

  state2.dispatch(event2{2});

  REQUIRE(myStates.is_in<StateInitial>());
  REQUIRE(myStates.context().is_valid == false);
  REQUIRE(myStates.context().value == 10);
}

struct StateCtxInitial;
struct StateCtxSecond;
struct StateCtxThird;

using CtxStates = states<StateCtxInitial, StateCtxSecond, StateCtxThird>;

using StateCtxContainer = detail::state_variant<CtxStates, Context &>;

struct StateCtxInitial : state<StateCtxInitial, StateCtxContainer> {

  // using state<StateInitial, StateContainer>::state;

  StateCtxInitial(StateCtxContainer &state_container) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return trans<StateCtxSecond>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateCtxSecond : state<StateCtxSecond, StateCtxContainer> {

  //  using state<StateSecond, StateContainer>::state;

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

  // StateThird() : count1(0) {}

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
    : state(state_container), count1(0), value2(0) {}

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

  auto myStates = StateCtxContainer(myContext);

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx = myStates.context();

  std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx.is_valid == false);
  REQUIRE(ctx.value == 20);

  myStates.emplace<StateCtxInitial>();

  REQUIRE(myStates.is_in<StateCtxInitial>());

  auto result = myStates.dispatch(event1{});

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

using StateCtx1Container = detail::state_variant<Ctx1States, Context &&>;

struct StateCtx1Initial : state<StateCtx1Initial, StateCtx1Container> {

  // using state<StateInitial, StateContainer>::state;

  StateCtx1Initial(StateCtx1Container &state_container) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return trans<StateCtx1Second>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateCtx1Second : state<StateCtx1Second, StateCtx1Container> {

  //  using state<StateSecond, StateContainer>::state;

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

  // StateThird() : count1(0) {}

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
    : state(state_container), count1(0), value2(0) {}

StateCtx1Second::StateCtx1Second(StateCtx1Container &state_container,
                               Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

StateCtx1Third::StateCtx1Third(StateCtx1Container &state_container,
                             Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {}

TEST_CASE("with explicit&& Context", "[new_fsm]") {

  std::cout << "start" << std::endl;

  std::cout << "myContext constructed" << std::endl;

  auto myStates = StateCtx1Container(Context{30});

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx = myStates.context();

  std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx.is_valid == false);
  REQUIRE(ctx.value == 30);

  myStates.emplace<StateCtx1Initial>();

  REQUIRE(myStates.is_in<StateCtx1Initial>());

  auto result = myStates.dispatch(event1{});

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
