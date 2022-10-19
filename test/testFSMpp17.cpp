/**
 * @file test.cpp
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2022-08-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <iostream>

//#include <catch2/catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fsmpp17/fsm.h>

struct EmptyContext
{
};

// events

struct start_event : fsm::event
{
};

struct stop_event : fsm::event
{
};

struct cont_event : fsm::event
{
};

struct abort_event : fsm::event
{
};

// States

// forward declarations
struct Running;
struct Interrupted;
struct Finished;

struct Initial : fsm::state<>
{

  Initial()
  {
    std::cout << "Entering Initial" << std::endl;
  }

  ~Initial()
  {
    std::cout << "Leaving Initial" << std::endl;
  }

  auto handle(const start_event &) const
  {
    std::cout << "start event" << std::endl;
    return transition<Running>();
  }
};

struct Running : fsm::state<>
{
  Running()
  {
    std::cout << "Entering Running" << std::endl;
  }

  ~Running()
  {
    std::cout << "Leaving Running" << std::endl;
  }

  auto handle(const stop_event &) const
  {
    std::cout << "stop event" << std::endl;
    return transition<Interrupted>();
  }
};

struct Interrupted : fsm::state<>
{

    Interrupted()
  {
    std::cout << "Entering Interrupted" << std::endl;
  }

  ~Interrupted()
  {
    std::cout << "Leaving Interrupted" << std::endl;
  }

  auto handle(const cont_event &) const
  {
    std::cout << "continue event" << std::endl;
    return transition<Running>();
  }

  auto handle(const abort_event &) const
  {
    std::cout << "abort event" << std::endl;
    return transition<Finished>();
  }
  
};

struct Finished : fsm::state<>
{

    Finished()
  {
    std::cout << "Entering Finished" << std::endl;
  }
};


TEST_CASE("Simple FSMpp17 numeric")
{

    using States = fsm::states<
        Initial,
        Running,
        Interrupted,
        Finished
    >;

    using Events = fsm::events<
        start_event,
        stop_event,
        cont_event,
        abort_event
    >;


  fsm::state_machine myfsm{States{}, Events{}, EmptyContext{}};
  

  // REQUIRE(myfsm<Initial>.is_state())
  //REQUIRE(myfsm.is_in<Initial>());

  myfsm.dispatch(start_event{});

  //REQUIRE(myfsm.is_in<Running>());
}
