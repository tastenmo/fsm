#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

//#include <new_fsm/machine.h>
#include <new_fsm/state.h>

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

struct Initial : state<Initial> {

  void onEnter() { std::cout << "Initial::onEnter()" << std::endl; }

  /**
   * @brief transition to Running
   *
   * @return auto
   */
  auto transitionTo(const start &)   { return trans<Running>(); }
};

struct Running : state<Running> {

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

int main() {

  std::cout << "A simple fsm..." << std::endl;
  // Color entries: RED = -10 BLUE = 0 GREEN = 10

  return 0;
}
