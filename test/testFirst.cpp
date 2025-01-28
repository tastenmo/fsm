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

// #include <catch2/catch.hpp>
#include <base/utils.h>
#include <catch2/catch_test_macros.hpp>
#include <fsm/fsm.h>


// events

struct start_event {
  std::string msg;
};

struct stop_event {};

struct cont_event {};

struct abort_event {};

// States
struct Running;
struct Interrupted;

struct Initial {

  void onEnter();

  auto transitionTo(const start_event &);
};

struct Running {
  void onEnter(const start_event &);

  auto transitionTo(const stop_event &);
};

struct Interrupted {
  void onEnter();

  auto transitionTo(const cont_event &);

  auto transitionTo(const abort_event &);
};

// Initial
void Initial::onEnter() { std::cout << "Entered Initial." << std::endl; }

auto Initial::transitionTo(const start_event &) { return Running{}; }

// Running
void Running::onEnter(const start_event &event) {
  std::cout << "Entered Running: " << event.msg << std::endl;
}

auto Running::transitionTo(const stop_event &) { return Interrupted{}; }

// Interrupted
void Interrupted::onEnter() {
  std::cout << "Entered Interrupted." << std::endl;
}

auto Interrupted::transitionTo(const cont_event &) { return Running{}; }

auto Interrupted::transitionTo(const abort_event &) { return Initial{}; }

using state = std::variant<Initial, Running, Interrupted>;

class Fsm : public escad::fsm::fsm<state> {};

class StateHandler {

public:
  StateHandler(Fsm *fsm) : fsm_(fsm) {}

  void OnEvent(const state &state_variant) {
    std::visit(escad::overloaded{
                   [&](const Running &) { fsm_->dispatch(stop_event{}); },
                   [&](const Interrupted &) { fsm_->dispatch(abort_event{}); },
                   [&](auto) {}},
               state_variant);
  }

private:
  Fsm *fsm_;
};

TEST_CASE("Simple FSM numeric") {
  Fsm myfsm;

  /*myfsm.NewState.connect([&](const state &state_variant)
                         { std::visit(
                               Overload{
                                   [&](const Running &state)
                                   {
                                     myfsm.dispatch(stop_event{});
                                   },
                                   [&](const Interrupted &state)
                                   {
                                     myfsm.dispatch(abort_event{});
                                   },
                                   [&](auto) {}},
                               state_variant); });*/

  StateHandler handler(&myfsm);

  // escad::fsm::slot sink{myfsm.NewState};

  // sink.connect(&handler::OnEvent);

  myfsm.init(Initial{});

  // REQUIRE(myfsm<Initial>.is_state())
  REQUIRE(myfsm.is_state<Initial>());

  myfsm.dispatch(start_event{"Hello!!!"});

  REQUIRE(myfsm.is_state<Running>());

  myfsm.dispatch(stop_event{});

  REQUIRE(myfsm.is_state<Interrupted>());

  auto conn = myfsm.NewState.connect<&StateHandler::OnEvent>(&handler);

  REQUIRE(conn);

  myfsm.dispatch(cont_event{});

  REQUIRE(myfsm.is_state<Initial>());

  conn.release();

  REQUIRE(myfsm.NewState.empty());
  REQUIRE_FALSE(conn);
}
