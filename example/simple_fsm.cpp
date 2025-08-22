#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <fsm/version.h>
#include <fsm/state.h>
#include <fsm/state_machine.h>

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

using States = states<Initial, Running, Paused, Stopped>;

using Machine = StateMachine<States, NoContext &>;

struct Initial : state<Initial, Machine, NoContext> {

  using state<Initial, Machine, NoContext>::state;

  void onEnter() { std::cout << "Initial::onEnter()" << std::endl; }

  /**
   * @brief transition to Running
   *
   * @return auto
   */
  auto transitionTo(const start &) { return sibling<Running>(); }
};

struct Running : state<Running, Machine, NoContext> {

  using state<Running, Machine, NoContext>::state;

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

struct Paused : state<Paused, Machine, NoContext> {

  using state<Paused, Machine, NoContext>::state;

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

struct Stopped : state<Stopped, Machine, NoContext> {

  using state<Stopped, Machine, NoContext>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter() { std::cout << "Stopped::onEnter()" << std::endl; }
};



int main() {

  std::cout << "A simple fsm..." << std::endl;

  std::cout << "Build with version: " << FSM_VERSION << std::endl;

  NoContext ctx;

  //StateMachine sm(mpl::type_identity<States>{}, ctx); // create a state machine
  auto sm = Machine(mpl::type_identity<States>{}, ctx); // create a state machine with reference context
  sm.emplace<Initial>();

  sm.dispatch(start{});

  sm.dispatch(pausing{1});

  sm.dispatch(start{});

  sm.dispatch(stop{});

  // Color entries: RED = -10 BLUE = 0 GREEN = 10

  return 0;
}
