#include "fsmpp17/transitions.h"
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include <new_fsm/state.h>
#include <new_fsm/transition.h>

using namespace escad::new_fsm;

struct event1 {};
struct event2 {};
struct event3 {};
struct event4 {

  event4(int val) : value_(val) {}

  int value_;
};

struct StateFirst {};
struct StateSecond {};
struct StateThird {};

auto transitionTo(const event1 &) { return trans<StateSecond>(); }

auto transitionTo(const event2 &) { return handled(); }

auto transitionTo(const event3 &) { return not_handled(); }

auto transitionTo(const event4 &event)
    -> transitions<StateFirst, StateSecond, StateThird, detail::not_handled,
                   detail::handled> {
  if (event.value_ == 1) {
    return trans<StateFirst>();
  } else if (event.value_ == 2) {
    return trans<StateSecond>();
  } else if (event.value_ == 3) {
    return handled();
  }
  return not_handled(); // does not work yet
  // return trans<StateSecond>();
}

TEST_CASE("transition_typelist", "[new_fsm]") {

  using tran = transitions<StateFirst, StateSecond, StateThird,
                           detail::not_handled, detail::handled>;

  STATIC_REQUIRE(std::is_same_v<transition_t<0u, tran>, StateFirst>);
  STATIC_REQUIRE(std::is_same_v<transition_t<1u, tran>, StateSecond>);
  STATIC_REQUIRE(std::is_same_v<transition_t<2u, tran>, StateThird>);
}

TEST_CASE("transition", "[new_fsm]") {

  auto result = transitionTo(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<StateSecond>>);
  REQUIRE(result.is_transition());
  REQUIRE(result.idx == 0);
}

TEST_CASE("handled", "[new_fsm]") {

  auto result = transitionTo(event2{});

  STATIC_REQUIRE(
      std::is_same_v<decltype(result), transitions<detail::handled>>);
  REQUIRE(result.is_handled());
  REQUIRE_FALSE(result.is_transition());
  CHECK(result.idx == 0); // fails is 1, I dont know why
}

TEST_CASE("not_handled", "[new_fsm]") {

  auto result = transitionTo(event3{});

  STATIC_REQUIRE(
      std::is_same_v<decltype(result), transitions<detail::not_handled>>);
  REQUIRE_FALSE(result.is_handled());
  REQUIRE_FALSE(result.is_transition());
  CHECK(result.idx == 0); // fails is 1, I dont know why
}

TEST_CASE("multiple transition path", "[new_fsm]") {

  auto result1 = transitionTo(event4{1});

  STATIC_REQUIRE(
      std::is_same_v<decltype(result1),
                     transitions<StateFirst, StateSecond, StateThird,
                                 detail::not_handled, detail::handled>>);
  REQUIRE(result1.is_transition());
  REQUIRE(result1.idx == 0);

  auto result2 = transitionTo(event4{2});
  STATIC_REQUIRE(
      std::is_same_v<decltype(result2),
                     transitions<StateFirst, StateSecond, StateThird,
                                 detail::not_handled, detail::handled>>);
  REQUIRE(result2.is_transition());
  REQUIRE(result2.idx == 1);

  auto result3 = transitionTo(event4{3});
  STATIC_REQUIRE(
      std::is_same_v<decltype(result3),
                     transitions<StateFirst, StateSecond, StateThird,
                                 detail::not_handled, detail::handled>>);
  CHECK_FALSE(result3.is_transition()); // Is true???
  CHECK(result3.is_handled());
  REQUIRE(result3.idx == 4);

  auto result4 = transitionTo(event4{4});
  STATIC_REQUIRE(
      std::is_same_v<decltype(result4),
                     transitions<StateFirst, StateSecond, StateThird,
                                 detail::not_handled, detail::handled>>);
  CHECK_FALSE(result4.is_transition());
  CHECK_FALSE(result4.is_handled());
  REQUIRE(result4.idx == 3);

  // REQUIRE(first.count1 == 1);
}

TEST_CASE("transition_for", "[new_fsm]") {

  auto trans1 = transitionTo(event1{});

  bool result1 = false;

  for_each_transition(trans1, [&](auto i, auto t) {
    std::cout << "i: " << i << " t: " << t.idx << std::endl;

    if (i == t.idx) {
      result1 = true;
    }
  });

  REQUIRE(result1);

  result1 = false;
  auto trans2 = transitionTo(event4{1});

  for_each_transition(trans2, [&](auto i, auto t) {
    std::cout << "i: " << i << " t: " << t.idx;

    if (i == t.idx) {
      std::cout << " matched";
      result1 = true;
    }
    std::cout << std::endl;
  });

  REQUIRE(result1);

  result1 = false;
  auto trans3 = transitionTo(event4{2});

  for_each_transition(trans3, [&](auto i, auto t) {
    std::cout << "i: " << i << " t: " << t.idx;

    if (i == t.idx) {
      std::cout << " matched";
      result1 = true;
    }
    std::cout << std::endl;
  });

  REQUIRE(result1);
}
