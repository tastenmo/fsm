#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include "base/utils.h"

// #include <new_fsm/machine.h>
#include <new_fsm/initial_state.h>

using namespace escad::new_fsm;

struct Context {
  bool is_valid = false;

  int value = 0;

  std::string msg;

  Context() : msg("empty") { std::cout << "Context::Context()" << std::endl; }

  Context(int val) : value(val), msg("empty") {
    std::cout << "Context::Context(int val)" << std::endl;
  }

  Context(const Context &other)
      : is_valid(other.is_valid), value(other.value), msg(other.msg) {
    std::cout << "Context::Context(const Context &other)" << std::endl;
  }

  Context(Context &&other) noexcept
      : is_valid(std::move(other.is_valid)), value(std::move(other.value)),
        msg(std::move(other.msg)) {
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


struct Initial;
struct StateSecond;
struct StateThird;

using States = states<Initial, StateSecond, StateThird>;

using StateContainer = state_variant<States, Context>;

struct Initial : initial_state<Initial, StateContainer> {

  Initial(StateContainer &state_container) noexcept;

  ~Initial() { std::cout << "Initial::~Initial()" << std::endl; }

  void onEnter();

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

  ~StateSecond() { std::cout << "StateSecond::~StateSecond()" << std::endl; }

void onEnter() {

    count1++;
    ctx_.is_valid = true;
    ctx_.value += 1;
  }
  auto transitionTo(const event2 &event) const
      -> transitions<Initial, StateSecond, StateThird> {
    if (event.value_ == 1) {
      return trans<Initial>();
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

  ~StateThird() { std::cout << "StateThird::~StateThird()" << std::endl; }

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid = false;
    if (ev.value_ == 2) {
      ctx_.value = 10;
      state_container_.emplace<Initial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // StateThird() : count1(0) {}

  int count1;
  Context &ctx_;
};

// State Constructors


Initial::Initial(StateContainer &state_container) noexcept
    : initial_state(state_container), count1(0), value2(0) {
      std::cout << "Initial::Initial()" << std::endl;
    }

void Initial::onEnter() {
  ;
}

StateSecond::StateSecond(StateContainer &state_container, Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {
      std::cout << "StateSecond::StateSecond()" << std::endl;
    }


StateThird::StateThird(StateContainer &state_container, Context &ctx) noexcept
    : state(state_container), count1(0), ctx_(ctx) {
      std::cout << "StateThird::StateThird()" << std::endl;
    }


class StateHandler {

public:

  void OnStateChange(const StateContainer::states_variant &state_variant) {
    std::visit(escad::overloaded{
                   [&](const Initial &) { std::cout << "Initial" << std::endl; },
                   [&](const StateSecond &) { std::cout << "StateSecond" << std::endl; },
                   [&](const StateThird &) { std::cout << "StateThird" << std::endl; },
                   [&](auto) {}},
               state_variant);
  }

};

auto myStatePrinter = escad::overloaded{
    [](Initial &) { std::cout << "Initial" << std::endl; },
    [](StateSecond &) { std::cout << "StateSecond" << std::endl; },
    [](StateThird &) { std::cout << "StateThird" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

TEST_CASE("with implicit Context", "[new_fsm]") {

  std::cout << "start" << std::endl;

  StateHandler handler;

  auto myStates = Initial::create();


  myStates.NewState.connect<&StateHandler::OnStateChange>(&handler);

  std::cout << "myStates constructed" << std::endl;
  // Context is copied here!!!!
  auto ctx = myStates.context();

  std::cout << "after myStates.context()" << std::endl;

  REQUIRE(ctx.is_valid == false);
  REQUIRE(ctx.value == 0);

  //myStates.emplace<Initial>();

  REQUIRE(myStates.is_in<Initial>());

  auto result = myStates.handle(event1{});

  REQUIRE(result);
  REQUIRE(myStates.is_in<StateSecond>());

  // Context is nor copied here????
  REQUIRE(myStates.context().is_valid == true);
  REQUIRE(myStates.context().value == 1);

  auto state2 = myStates.state<StateSecond>();

  REQUIRE(state2.count1 == 1);
  REQUIRE(state2.ctx_.is_valid == true);
  REQUIRE(state2.ctx_.value == 1);

  state2.handle(event2{2});

  REQUIRE(myStates.is_in<Initial>());
  REQUIRE(myStates.context().is_valid == false);
  REQUIRE(myStates.context().value == 10);
}
