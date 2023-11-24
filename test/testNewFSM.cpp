#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

// #include <new_fsm/machine.h>
#include <new_fsm/state.h>

using namespace escad::new_fsm;

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct StateSecond;
struct StateThird;

struct StateInitial : state<StateInitial> {

  StateInitial() : count1(0), value2(0) {}

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return trans<StateSecond>(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;
};

struct StateSecond : state<StateSecond> {

  StateSecond() : count1(0) {}

  void onEnter() { count1++; }

  auto transitionTo(const event2 &event) const
      -> transitions<StateInitial, StateSecond, StateThird> {
    if (event.value_ == 1) {
      return trans<StateInitial>();
    } else if (event.value_ == 2) {
      return trans<StateThird>();
    }
    // handled() does not work yet
    return trans<StateSecond>();
  }

  auto transitionTo(const event1 &) const { return handled(); }

  int count1;
};

struct StateThird : state<StateThird> {

  StateThird() : count1(0) {}

  int count1;
};

constexpr auto number_rx =
    ctll::fixed_string{"(-?[0-9]*\\.[0-9]*([eE]-?[0-9]+)?|-?[1-9][0-9]*)"};

template <ctll::fixed_string regex> struct matcher {

  auto match(std::string_view sv) const { return ctre::match<regex>(sv); }

  int value_;
};

using number = matcher<number_rx>;

struct StateCtre : state<StateCtre> {

  StateCtre() : num1("") {}

  void onEnter(const number &num) {

    if (auto [m, integer, number] = num.match(std::string_view{"1.234"}); m) {
      num1 = m.to_view();
    }
  }

  std::string num1;

  auto transitionTo(const number &num)
      -> transitions<StateInitial, StateSecond> const {

    auto m = num.match(std::string_view{"1.234"});
    if (m) {
      std::cout << "Transition:" << m.to_view() << std::endl;
      return trans<StateInitial>();
    }

    return trans<StateSecond>();
  }
};

TEST_CASE("state_onEnter", "[new_fsm]") {

  StateInitial first;
  StateSecond second;
  StateThird third;

  REQUIRE(first.count1 == 0);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 0);
  REQUIRE(third.count1 == 0);

  first.enter();
  second.enter();
  third.enter();

  REQUIRE(first.count1 == 0);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);

  first.enter(event3{"test"});
  second.enter(event3{"test"});
  third.enter(event3{"test"});

  REQUIRE(first.count1 == 0);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);

  first.enter(event1{});
  second.enter(event1{});
  third.enter(event1{});

  REQUIRE(first.count1 == 1);
  REQUIRE(first.value2 == 0);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);

  first.enter(event2{42});
  second.enter(event2{42});
  third.enter(event2{42});

  REQUIRE(first.count1 == 1);
  REQUIRE(first.value2 == 42);
  REQUIRE(second.count1 == 1);
  REQUIRE(third.count1 == 0);
}

TEST_CASE("state_onEnterCTRE", "[new_fsm]") {

  StateCtre first;

  first.enter(number{});
  REQUIRE(first.num1 == "1.234");
};

TEST_CASE("state_transitionTo", "[new_fsm]") {

  StateInitial first;

  auto result = first.transitionTo(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<StateSecond>>);
  REQUIRE(result.is_transition());

  auto result1 = first.transition(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result1), transitions<StateSecond>>);
  REQUIRE(result1.is_transition());

  StateSecond second;

  auto result2 = second.transition(event1{});
  CHECK(result2.is_handled());

  auto result3 = second.transition(event2(1));
  STATIC_REQUIRE(
      std::is_same_v<decltype(result3),
                     transitions<StateInitial, StateSecond, StateThird>>);
  REQUIRE(result3.is_transition());
  REQUIRE(result3.idx == 0);

  auto result4 = second.transition(event2(2));
  STATIC_REQUIRE(
      std::is_same_v<decltype(result4),
                     transitions<StateInitial, StateSecond, StateThird>>);
  REQUIRE(result4.is_transition());
  REQUIRE(result4.idx == 2);

  auto result5 = second.transition(event2(3));
  STATIC_REQUIRE(
      std::is_same_v<decltype(result5),
                     transitions<StateInitial, StateSecond, StateThird>>);
  REQUIRE(result5.is_transition());
  REQUIRE(result5.idx == 1);

  StateCtre ctre;

  auto result6 = ctre.transition(number{});
  STATIC_REQUIRE(std::is_same_v<decltype(result6),
                                transitions<StateInitial, StateSecond>>);
  REQUIRE(result6.is_transition());

  // REQUIRE(first.count1 == 1);
}