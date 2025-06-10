#include <chrono>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include "base/type_traits.h"
#include "base/utils.h"

// #include <new_fsm/machine.h>
// #include <new_fsm/state.h>
#include <new_fsm/state_machine.h>

#include <variant>

using namespace escad::new_fsm;

class theContext {

public:
  theContext() : PlaySequence_(false), value_(0), msg_("") {

    std::cout << "theContext()" << std::endl;
  };

  ~theContext() { std::cout << "~theContext()" << std::endl; }

  void PlaySequence() { PlaySequence_ = true; }

  bool GetPlaySequence() const { return PlaySequence_; }

  void Value(int val) { value_ = val; }

  int Value() const { return value_; }

private:
  bool PlaySequence_;
  int value_;

  std::string msg_;
};

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct myStates {

  struct Initial : state<Initial, theContext> {

    using state<Initial, theContext>::state;

    void onEnter(const event1 &) { count1++; }

    void onEnter(const event2 &ev) { value2 += ev.value_; }

    /**
     * @brief simple transition
     *
     * @return auto
     */
    auto transitionTo(const event1 &) { return sibling<Second>(); }
    // auto transitionTo(const event2 &) const { return not_handled(); }

    // template<>
    // auto transitionTo<StateSecond>(const event1 &);

    int count1;
    int value2;
  };

  struct Third : state<Third, theContext> {

    using state<Third, theContext>::state;

    // auto transitionTo(const event2 &) const { return handled(); }
    // auto transitionTo(const event1 &) const { return handled(); }

    // StateThird() : count1(0) {}

    int count1;
  };

  struct Second : state<Second, theContext> {

    using state<Second, theContext>::state;

    void onEnter() { count1++; }

    auto transitionTo(const event2 &event) const
        -> transitions<detail::none, Initial, Second, Third> {
      if (event.value_ == 1) {
        return sibling<Initial>();
      } else if (event.value_ == 2) {
        return sibling<Third>();
      }
      // handled() does not work yet
      return none();
    }

    auto transitionTo(const event1 &) const { return none(); }

    auto transitionInternalTo() -> transitions<detail::none, Third> const {

      if (context_.GetPlaySequence()) {
        return sibling<Third>();
      }
      return none();
    }

    int count1;
  };
};

using States = states<myStates::Initial, myStates::Second, myStates::Third>;

// State Constructors

auto myStatePrinter = escad::overloaded{
    [](myStates::Initial &) { std::cout << "Initial" << std::endl; },
    [](myStates::Second &) { std::cout << "Second" << std::endl; },
    [](myStates::Third &) { std::cout << "Third" << std::endl; },
    [](std::monostate) { std::cout << "std::monostate" << std::endl; },
    [](auto) { std::cout << "unknown" << std::endl; },
};

TEST_CASE("state_types", "[new_fsm]") {

  STATIC_REQUIRE(std::is_copy_constructible_v<std::monostate>);
  STATIC_REQUIRE(std::is_copy_constructible_v<myStates::Initial>);
  STATIC_REQUIRE(std::is_copy_constructible_v<myStates::Second>);
  STATIC_REQUIRE(std::is_copy_constructible_v<myStates::Third>);

  STATIC_REQUIRE(std::is_move_constructible_v<myStates::Initial>);
  STATIC_REQUIRE(std::is_move_constructible_v<myStates::Second>);
  STATIC_REQUIRE(std::is_move_constructible_v<myStates::Third>);
}

TEST_CASE("state_onEnter", "[new_fsm]") {

  theContext ctx;

  STATIC_REQUIRE(std::is_lvalue_reference_v<theContext &>);

  StateMachine fsm(mpl::type_identity<States>{}, ctx);

  fsm.emplace<myStates::Initial>();

  REQUIRE(fsm.is_in<myStates::Initial>());

  fsm.dispatch(event1{});

  REQUIRE(fsm.is_in<myStates::Second>());

  fsm.context().GetPlaySequence();

  fsm.dispatch(event1{});

  REQUIRE(fsm.is_in<myStates::Third>());
}
