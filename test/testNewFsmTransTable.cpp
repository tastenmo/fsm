#include <chrono>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include "base/utils.h"

// #include <new_fsm/machine.h>
// #include <new_fsm/state.h>
#include <new_fsm/transition_table.h>

using namespace escad::new_fsm;

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct myStates {
  struct Initial : state<Initial> {

    using state<Initial>::state;

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

  struct Third : state<Third> {

    using state<Third>::state;

    // auto transitionTo(const event2 &) const { return handled(); }
    // auto transitionTo(const event1 &) const { return handled(); }

    // StateThird() : count1(0) {}

    int count1;
  };

  struct Second : state<Second> {

    using state<Second>::state;

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

      if (count1 > 2) {
        return sibling<Third>();
      }
      return none();
    }

    int count1;
  };
};

TEST_CASE("state_types", "[new_fsm]") {

  STATIC_REQUIRE(
      std::is_same_v<const transitions<detail::none, myStates::Third>,
                     internalTransitions<myStates::Second>>);

  STATIC_REQUIRE(std::is_same_v<transitions<detail::none>,
                                externalTransitions<myStates::Second, event1>>);
};
