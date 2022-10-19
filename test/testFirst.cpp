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
#include <fsm/fsm.h>


// events

struct start_event
{
};

struct stop_event
{
};

struct cont_event
{
};

struct abort_event
{
};

// States
struct Running;
struct Interrupted;

struct Initial
{

  void onEnter();

  auto transitionTo(const start_event &);
};

struct Running
{
  void onEnter();

  auto transitionTo(const stop_event &);
};

struct Interrupted
{
  void onEnter();

  auto transitionTo(const cont_event &);

  auto transitionTo(const abort_event &);
};

// Initial
void Initial::onEnter()
{
  std::cout << "Entered Initial." << std::endl;
}

auto Initial::transitionTo(const start_event &start)
{

  return Running{};
}

// Running
void Running::onEnter()
{
  std::cout << "Entered Running." << std::endl;
}

auto Running::transitionTo(const stop_event &stop)
{

  return Interrupted{};
}

// Interrupted
void Interrupted::onEnter()
{
  std::cout << "Entered Interrupted." << std::endl;
}

auto Interrupted::transitionTo(const cont_event &ev)
{

  return Running{};
}

auto Interrupted::transitionTo(const abort_event &ev)
{

  return Initial{};
}

using state = std::variant<Initial, Running, Interrupted>;

class Fsm : public fsm::fsm<state>
{
};

TEST_CASE("Simple FSM numeric")
{
  Fsm myfsm;

  myfsm.init(Initial{});

  // REQUIRE(myfsm<Initial>.is_state())
  REQUIRE(myfsm.is_state<Initial>());

  myfsm.dispatch(start_event{});

  REQUIRE(myfsm.is_state<Running>());
}
