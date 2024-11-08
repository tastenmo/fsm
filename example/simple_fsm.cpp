#include "base/type_traits.h"
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

// #include <new_fsm/machine.h>
#include <new_fsm/state.h>
#include <new_fsm/state_variant.h>

using namespace escad::new_fsm;

struct NoContext {};

struct start {};
struct pausing {
  pausing(int val) : value_(val) {}

  int value_;
};
struct stop {};

struct Initial;
struct Running;
struct Paused;
struct Stopped;

struct Initial : state<Initial, NoContext> {

  using state<Initial, NoContext>::state;

  void onEnter() { std::cout << "Initial::onEnter()" << std::endl; }

  /**
   * @brief transition to Running
   *
   * @return auto
   */
  auto transitionTo(const start &) { return sibling<Running>(); }
};

struct Running : state<Running, NoContext> {

  using state<Running, NoContext>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter(const start &) {
    std::cout << "Running::onEnter(cost start &)" << std::endl;
  }

  /**
   * @brief transition to Paused
   *
   * @return auto
   */
  auto transitionTo(const pausing &) const { return sibling<Paused>(); }

  /**
   * @brief transition to Stopped
   *
   * @return auto
   */
  auto transitionTo(const stop &) const { return sibling<Stopped>(); }
};

struct Paused : state<Paused, NoContext> {

  using state<Paused, NoContext>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter(const pausing &) {
    std::cout << "Paused::onEnter(cost pause &)" << std::endl;
  }

  /**
   * @brief transition to Paused
   *
   * @return auto
   */
  auto transitionTo(const start &) const { return sibling<Running>(); }

  /**
   * @brief transition to Stopped
   *
   * @return auto
   */
  auto transitionTo(const stop &) const { return sibling<Stopped>(); }
};

struct Stopped : state<Stopped, NoContext> {

  using state<Stopped, NoContext>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter() { std::cout << "Stopped::onEnter()" << std::endl; }
};

using States = states<Initial, Running, Paused, Stopped>;

int main() {

  std::cout << "A simple fsm..." << std::endl;

  NoContext ctx;

  StateMachine sm(mpl::type_identity<States>{}, ctx); // create a state machine

  sm.emplace<Initial>();

  sm.dispatch(start{});

  sm.dispatch(pausing{1});

  sm.dispatch(start{});

  sm.dispatch(stop{});

  // Color entries: RED = -10 BLUE = 0 GREEN = 10

  return 0;
}
