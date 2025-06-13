#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/number.h>

using namespace escad::json;

using namespace std::literals;

TEST_CASE("Number unsigned", "[json]") {

  auto fsm = StateMachine(mpl::type_identity<number::States>{},
                          number::Context(view{"123"sv}));

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(fsm.context().value() == "123"sv);

  REQUIRE(fsm.context().size() == 3);

  auto number = fsm.context().number.get<unsigned>();
  REQUIRE(number);
  REQUIRE(*number == 123u);

  REQUIRE_FALSE(fsm.context().number.get<int>());
}

TEST_CASE("Number negative integer", "[json]") {

  view v("-54321"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "-54321"sv);

  REQUIRE(ctx.size() == 6);

  auto number = ctx.number.get<int>();
  REQUIRE(number);
  REQUIRE(*number == -54321);

  REQUIRE_FALSE(ctx.number.get<unsigned>());
}

TEST_CASE("Number positive integer", "[json]") {

  view v("+54321"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "+54321"sv);

  REQUIRE(ctx.size() == 6);

  auto number = ctx.number.get<int>();
  REQUIRE(number);
  REQUIRE(*number == 54321);

  REQUIRE_FALSE(ctx.number.get<unsigned>());
}

TEST_CASE("Number very large unsigned", "[json]") {

  std::cout << "unsigned max: " << std::numeric_limits<unsigned>::max()
            << std::endl;

  // max of unsigned is 4294967295
  view v("5294967295"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "5294967295"sv);

  REQUIRE(ctx.size() == 10);

  auto number = ctx.number.get<uint64_t>();
  REQUIRE(number);
  REQUIRE(*number == UINT64_C(5294967295));

  REQUIRE_FALSE(ctx.number.get<unsigned>());
}

TEST_CASE("Number very large signed", "[json]") {

  std::cout << "int min: " << std::numeric_limits<int>::min() << std::endl;

  // min of int is -2147483648
  view v("-3147483648"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "-3147483648"sv);

  REQUIRE(ctx.size() == 11);

  auto number = ctx.number.get<int64_t>();
  REQUIRE(number);
  REQUIRE(*number == INT64_C(-3147483648));

  REQUIRE_FALSE(ctx.number.get<int>());
}

TEST_CASE("Number double", "[json]") {

  view v("123.3756"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "123.3756"sv);

  REQUIRE(ctx.size() == 8);

  auto number = ctx.number.get<double>();
  REQUIRE(number);
  REQUIRE(*number == 123.3756);

  REQUIRE_FALSE(ctx.number.get<int>());
}

TEST_CASE("Number scientific1", "[json]") {

  view v("-1.234e06"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "-1.234e06"sv);

  REQUIRE(ctx.size() == 9);

  auto number = ctx.number.get<double>();
  REQUIRE(number);
  REQUIRE(*number == -1.234e6);

  REQUIRE_FALSE(ctx.number.get<int>());
}

TEST_CASE("Number scientific2", "[json]") {

  view v("-1.234e-06"sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "-1.234e-06"sv);

  REQUIRE(ctx.size() == 10);

  auto number = ctx.number.get<double>();
  REQUIRE(number);
  REQUIRE(*number == -1.234e-6);

  REQUIRE_FALSE(ctx.number.get<int>());
}

TEST_CASE("Number 1.", "[json]") {

  view v("1."sv);

  number::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<number::States>{}, ctx);

  fsm.emplace<number::Initial>();

  REQUIRE(fsm.is_in<number::Finished>());

  REQUIRE(ctx.value() == "1."sv);

  REQUIRE(ctx.size() == 2);

  auto number = ctx.number.get<double>();
  REQUIRE(number);
  REQUIRE(*number == 1.0);

  REQUIRE_FALSE(ctx.number.get<int>());
}