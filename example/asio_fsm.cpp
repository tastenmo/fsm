#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <date/date.h>

#include <boost/asio.hpp>

#include <fsm/version.h>
#include <fsm/state.h>
#include <fsm/state_machine.h>

namespace io = boost::asio;

using namespace date;
using namespace std::chrono;

using error_code = boost::system::error_code;

using namespace escad::new_fsm;

struct Context {


   Context(io::io_context &io_context)
      : deadline(io_context) {}
   
   io::steady_timer deadline;
};


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
using Machine = StateMachine<States, Context &>;


struct Initial : state<Initial, Machine, Context> {

  using state<Initial, Machine, Context>::state;

  void onEnter() { std::cout << "Initial::onEnter()" << std::endl; }

  /**
   * @brief transition to Running
   *
   * @return auto
   */
  auto transitionTo(const start &) { return sibling<Running>(); }
};

struct Running : state<Running, Machine, Context> {

  using state<Running, Machine, Context>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter(const start &);

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

struct Paused : state<Paused, Machine, Context> {

  using state<Paused, Machine, Context>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter(const pausing &);

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

struct Stopped : state<Stopped, Machine, Context> {

  using state<Stopped, Machine, Context>::state;

  /**
   * @brief onEnter
   *
   * @param start
   */
  void onEnter() { 
   
         auto tp = system_clock::now();

     std::cout << "Stopped::onEnter(), timestamp: " << tp << std::endl;

}
};


/**
 * @brief onEnter method for Running state.
 *
 * This method is called when the state machine enters the Running state.
 * It sets a deadline for 5 seconds and waits asynchronously for it to expire.
 * If the deadline expires, it transitions to the Stopped state.
 *
 * @param start The start event that triggers the transition to Running state.
 *
 * @note This method has to be defined outside the class definition
 * because it uses the `async_wait` method of the `deadline` timer, which
 * requires a complete definition of the `Running` class.
 */

void Running::onEnter(const start &) {

   auto tp = system_clock::now();

    std::cout << "Running::onEnter(cost start &), timestamp: " << tp << std::endl;

    context_.deadline.expires_after(std::chrono::seconds(5));

    context_.deadline.async_wait([&sm = this->machine_](error_code ec) {

      if constexpr (std::is_lvalue_reference_v<decltype(sm)>) {
        std::cout << "sm is a lvalue_reference" << std::endl;
      } 
      
      if (!ec) {
        std::cout << "Running::deadline expired, transitioning to Stopped." << std::endl;
         sm.dispatch(stop{}); // Dispatch stop event to transition to Stopped state
      } 
      else if (ec == boost::asio::error::operation_aborted) {
        std::cout << "Running::deadline was cancelled." << std::endl;

      }
    
      else {
        std::cout << "Running::deadline error: " << ec.message() << std::endl;
      }
   }


    );
  }

void Paused::onEnter(const pausing &pause) {
      auto tp = system_clock::now();

     std::cout << "Paused::onEnter(cost pausing &), timestamp: " << tp << " value: " << pause.value_ << std::endl;

    context_.deadline.expires_after(std::chrono::seconds(pause.value_));

    context_.deadline.async_wait([&sm = this->machine_](error_code ec) {
      
      if (!ec) {
        std::cout << "Paused::deadline expired, transitioning back to Running." << std::endl;
         sm.dispatch(start{}); // Dispatch stop event to transition to Stopped state
      } 
      else if (ec == boost::asio::error::operation_aborted) {
        std::cout << "Paused::deadline was cancelled." << std::endl;

      }
    
      else {
        std::cout << "Paused::deadline error: " << ec.message() << std::endl;
      }
   }


    );
  }


int main() {

  std::cout << "A simple fsm..." << std::endl;

  std::cout << "Build with version: " << FSM_VERSION << std::endl;

  io::io_context io_context; // create an io_context for asynchronous operations

  Context ctx(io_context); // create a context for the state machine

  //StateMachine sm(mpl::type_identity<States>{}, ctx); // create a state machine
  auto sm = Machine(mpl::type_identity<States>{}, ctx); // create a state machine with reference context

  sm.emplace<Initial>();

  sm.dispatch(start{});

  sm.dispatch(pausing{2});

  
  std::cout << "Running the asynchronous io_context..." << std::endl;
  io_context.run(); // run the io_context to process asynchronous operations

  std::cout << "Asynchronous io_context finished." << std::endl;

  // Color entries: RED = -10 BLUE = 0 GREEN = 10

  return 0;
}
