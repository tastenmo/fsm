#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

// #include <new_fsm/machine.h>
#include <new_fsm/state.h>
#include <new_fsm/state_variant.h>

using namespace escad::new_fsm;

struct start {};
struct pause {
  pause(int val) : value_(val) {}

  int value_;
};
struct stop {};

struct Initial;
struct Running;
struct Paused;
struct Stopped;

using StateContainer =
    detail::state_variant<states<Initial, Running, Paused, Stopped>>;

struct Initial : state<Initial, StateContainer> {

  using state<Initial, StateContainer>::state;

  void onEnter() { std::cout << "Initial::onEnter()" << std::endl; }

  /**
   * @brief transition to Running
   *
   * @return auto
   */
  auto transitionTo(const start &) { return trans<Running>(); }
};

struct Running : state<Running, StateContainer> {

  using state<Running, StateContainer>::state;

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
  auto transitionTo(const pause &) const { return trans<Paused>(); }

  /**
   * @brief transition to Stopped
   *
   * @return auto
   */
  auto transitionTo(const stop &) const { return trans<Stopped>(); }
};

struct Paused : state<Paused, StateContainer> {

  using state<Paused, StateContainer>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter(const pause &) {
    std::cout << "Paused::onEnter(cost pause &)" << std::endl;
  }

  /**
   * @brief transition to Paused
   *
   * @return auto
   */
  auto transitionTo(const start &) const { return trans<Running>(); }

  /**
   * @brief transition to Stopped
   *
   * @return auto
   */
  auto transitionTo(const stop &) const { return trans<Stopped>(); }
};

struct Stopped : state<Stopped, StateContainer> {

  using state<Stopped, StateContainer>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter() { std::cout << "Stopped::onEnter()" << std::endl; }
};

int main() {

  std::cout << "A simple fsm..." << std::endl;

  StateContainer sm;

  sm.emplace<Initial>();

  sm.dispatch(start{});

  sm.dispatch(pause{1});

  sm.dispatch(start{});

  sm.dispatch(stop{});

  // Color entries: RED = -10 BLUE = 0 GREEN = 10

  return 0;
}
