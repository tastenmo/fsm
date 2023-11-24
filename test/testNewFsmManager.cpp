#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

// #include <new_fsm/machine.h>
#include <new_fsm/manager.h>

using namespace escad::new_fsm;

struct AContext {
  int shared_value = 0;
  std::string message;
};

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct StateSecond;
struct StateThird;

struct StateInitial : state<StateInitial> {

  StateInitial() : count1(0), value2(0) {}

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return trans<StateSecond>(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateSecond : state<StateSecond> {

  StateSecond(AContext &ctx) : ctx_(ctx), count1(0) {}

  void onEnter() {
    count1++;
    ctx_.message = "StateSecond";
  }

  auto transitionTo(const event2 &event) const
      -> transitions<StateInitial, StateSecond, StateThird, detail::handled> {
    if (event.value_ == 1) {
      return trans<StateInitial>();
    } else if (event.value_ == 2) {
      return trans<StateThird>();
    }
    // handled() does not work yet
    return not_handled();
    // return trans<StateSecond>();
  }

  auto transitionTo(const event1 &) const {
    ctx_.shared_value++;
    return handled();
  }

  AContext &ctx_;
  int count1;
};

struct StateThird : state<StateThird> {

  StateThird(AContext &ctx) : ctx_(ctx), count1(0) {}

  void onEnter() {
    count1++;
    ctx_.message = "StateThird";
  }

  AContext &ctx_;
  int count1;

  auto transitionTo(const event1 &) { return trans<StateSecond>(); }
};

using sm_states = std::variant<StateInitial, StateSecond, StateThird>;

TEST_CASE("state_Manager", "[new_fsm]") {

  AContext ctx;
  ctx.message = "Initial";

  manager<sm_states, AContext &> sm{ctx};

  REQUIRE(sm.is_in<StateInitial>());

  sm.dispatch(event1{});

  REQUIRE(sm.is_in<StateSecond>());

  auto &actualState = sm.state<StateSecond>();

  REQUIRE(actualState.ctx_.shared_value == 0);
  REQUIRE(actualState.ctx_.message == "StateSecond");

  sm.dispatch(event1{});

  REQUIRE(sm.is_in<StateSecond>());
  REQUIRE(actualState.ctx_.shared_value == 1);
}
