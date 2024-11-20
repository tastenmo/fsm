#include "json/number.h"
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/object.h>

using namespace escad::json;

using namespace std::literals;

TEST_CASE("Json_object - simple", "[json]") {

  view v(
      "{\"unsigned\":1234,\"string\":\"Das ist ein Test\", \"isValid\":true, \"show Details\":false}"sv);

  object::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<object::States>{}, ctx);

  fsm.emplace<object::Initial>();

  REQUIRE(fsm.is_in<object::Finished>());

  auto theObject = ctx.values();

  auto value = std::get<3>(theObject.getValue("unsigned"));

  REQUIRE(value.get<unsigned>() == 1234);

  auto thestring = std::get<2>(theObject.getValue("string"));

  REQUIRE(thestring == "Das ist ein Test");

  auto isValid = std::get<1>(theObject.getValue("isValid"));

  REQUIRE(isValid == true);

  auto showDetails = std::get<1>(theObject.getValue("show Details"));

  REQUIRE(showDetails == false);
}

TEST_CASE("Json_object - nested", "[json]") {

  std::string_view input = "{\n"
                           "\"unsigned\":1234, \n"
                           "\"string\":\"Das ist ein Test\",\n"
                           "\"numbers\": {\n"
                            "  \"unsigned\":1234, \n"
                            "  \"real\": 1234.5678\n"
                            "},\n"
                            "\"isValid\":true, \n"
                            "\"show Details\":false\n"
                            "}"sv;

      view v(input);

  object::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<object::States>{}, ctx);

  fsm.emplace<object::Initial>();

  REQUIRE(fsm.is_in<object::Finished>());

  auto theObject = ctx.values();

  auto value = std::get<3>(theObject.getValue("unsigned"));

  REQUIRE(value.get<unsigned>() == 1234);

  auto thestring = std::get<2>(theObject.getValue("string"));

  REQUIRE(thestring == "Das ist ein Test");

  auto isValid = std::get<1>(theObject.getValue("isValid"));

  REQUIRE(isValid == true);

  auto showDetails = std::get<1>(theObject.getValue("show Details"));

  REQUIRE(showDetails == false);

  auto numbers = std::get<4>(theObject.getValue("numbers"));

  auto numbers_unsigned = std::get<3>(numbers.getValue("unsigned"));

  REQUIRE(numbers_unsigned.get<unsigned>() == 1234);

  auto numbers_real = std::get<3>(numbers.getValue("real"));

  REQUIRE(numbers_real.get<double>() == 1234.5678);
}