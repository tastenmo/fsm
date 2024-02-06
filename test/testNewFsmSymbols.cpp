
#include <chrono>
#include <ctre.hpp>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <new_fsm/state.h>
#include <new_fsm/symbol.h>
#include <new_fsm/transition.h>

using namespace escad::new_fsm;

struct event1 {};

struct event2 {};

static constexpr auto number_regex =
    ctll::fixed_string{"[\\-\\+]?[0-9]+[.,][0-9]+"};

auto match = ctre::match<number_regex>("2.321");

struct isNumber {

  constexpr bool operator()(const std::string_view &sv) const {
    return ctre::match<number_regex>(sv);
  }
};

template <class TEvent> struct isEvent {
  static constexpr bool value = false;

  template <class U> constexpr bool operator()(const U &) const {

    return std::is_same_v<TEvent, U>;
  }
};

using symbol = result_type<isEvent<event1>, isNumber>;

struct StateFirst {};
struct StateSecond {};
struct StateThird {};

auto transitionTo(const event1 &) { return trans<StateSecond>(); }

template <class TSymbols, class... Values>
auto transitionTo(const Values &...values)
    -> transitions<StateSecond, detail::not_handled> {

  if (TSymbols{}(values...)) {
    return trans<StateSecond>();
  } else {
    return not_handled();
  }
}

TEST_CASE("symbol_typelist", "[new_fsm]") {

  STATIC_REQUIRE(isEvent<event1>{}(event1{}));

  STATIC_REQUIRE(isNumber{}("2.321"));

  using symbol = result_type<isEvent<event1>, isNumber>;

  STATIC_REQUIRE(symbol::symbol_list::size == 2);

  STATIC_REQUIRE(symbol{}(event1{}, "2.321"));

  STATIC_REQUIRE_FALSE(symbol{}(event2{}, "2.321"));

  STATIC_REQUIRE_FALSE(symbol{}(event1{}, "Martin"));

  STATIC_REQUIRE_FALSE(symbol{}(event1{}));

  event1 *e1 = new event1();

  std::string test = "2.321";

  REQUIRE(symbol{}(*e1, std::string_view{test}));

  test.append("Martin");

  REQUIRE_FALSE(symbol{}(*e1, std::string_view{test}));
}

TEST_CASE("symbol_transition", "[new_fsm]") {

  auto result = transitionTo<symbol>(event1{});

  STATIC_REQUIRE(std::is_same_v<decltype(result),
                                transitions<StateSecond, detail::not_handled>>);

  REQUIRE_FALSE(result.is_handled());

  auto result1 = transitionTo<symbol>(event1{}, "2.321");

  REQUIRE(result1.is_transition());
  REQUIRE(result1.idx == 0);
}