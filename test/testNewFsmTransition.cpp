#include "fsmpp17/transitions.h"
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

// #include <new_fsm/state.h>
#include <new_fsm/transition.h>

using namespace escad::new_fsm;

struct event1 {};
struct event2 {};
struct event3 {};
struct event4 {

  event4(int val) : value_(val) {}

  int value_;
};

struct event5 {};

struct StateFirst {};
struct StateSecond {};
struct StateThird {};

auto transitionTo(const event1 &) { return sibling<StateSecond>(); }

auto transitionTo(const event2 &) { return none(); }

auto transitionTo(const event3 &) { return inner<StateThird>(); }

auto transitionTo(const event5 &) { return inner_entry<StateThird>(); }

auto transitionTo(const event4 &event)
    -> transitions<StateFirst, StateSecond, StateThird, detail::none> {
  if (event.value_ == 1) {
    return sibling<StateFirst>();
  } else if (event.value_ == 2) {
    return inner<StateSecond>();
  } else if (event.value_ == 3) {
    return inner_entry<StateThird>();
  }
  return none(); // does not work yet
  // return trans<StateSecond>();
}

TEST_CASE("transition_typelist", "[new_fsm]") {

  using tran = transitions<StateFirst, StateSecond, StateThird, detail::none>;

  STATIC_REQUIRE(std::is_same_v<transition_t<0u, tran>, StateFirst>);
  STATIC_REQUIRE(std::is_same_v<transition_t<1u, tran>, StateSecond>);
  STATIC_REQUIRE(std::is_same_v<transition_t<2u, tran>, StateThird>);
}

TEST_CASE("transition", "[new_fsm]") {

  auto result = transitionTo(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<StateSecond>>);
  REQUIRE(result.is_sibling());
  REQUIRE(result.idx == 0);

  auto result2 = transitionTo(event3{});
  STATIC_REQUIRE(std::is_same_v<decltype(result2), transitions<StateThird>>);
  REQUIRE(result2.is_inner());
  REQUIRE(result.idx == 0);

  auto result3 = transitionTo(event5{});
  STATIC_REQUIRE(std::is_same_v<decltype(result3), transitions<StateThird>>);
  REQUIRE(result3.is_inner_entry());
  REQUIRE(result.idx == 0);
}

TEST_CASE("none", "[new_fsm]") {

  auto result = transitionTo(event2{});

  STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<detail::none>>);
  REQUIRE(result.is_none());
  REQUIRE_FALSE(result.is_transition());
  CHECK(result.idx == 0); // fails is 1, I dont know why
}

TEST_CASE("multiple transition path", "[new_fsm]") {

  auto result1 = transitionTo(event4{1});

  STATIC_REQUIRE(
      std::is_same_v<decltype(result1), transitions<StateFirst, StateSecond,
                                                    StateThird, detail::none>>);
  REQUIRE(result1.is_sibling());
  REQUIRE(result1.idx == 0);

  auto result2 = transitionTo(event4{2});
  STATIC_REQUIRE(
      std::is_same_v<decltype(result2), transitions<StateFirst, StateSecond,
                                                    StateThird, detail::none>>);
  REQUIRE(result2.is_inner());
  REQUIRE(result2.idx == 1);

  auto result3 = transitionTo(event4{3});
  STATIC_REQUIRE(
      std::is_same_v<decltype(result3), transitions<StateFirst, StateSecond,
                                                    StateThird, detail::none>>);
  REQUIRE(result3.is_inner_entry()); // Is true???
  REQUIRE(result3.idx == 2);

  auto result4 = transitionTo(event4{4});
  STATIC_REQUIRE(
      std::is_same_v<decltype(result4), transitions<StateFirst, StateSecond,
                                                    StateThird, detail::none>>);
  REQUIRE_FALSE(result4.is_sibling());

  REQUIRE(result4.is_none());
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

