#include "json/number.h"
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/object.h>

using namespace spie::json;

using namespace std::literals;

TEST_CASE("Object - simple", "[json]") {

   view v(
       "{\"unsigned\":1234,\"string\":\"Das ist ein Test\", \"isValid\":true, \"showDetails\":false}"sv);

   object::Context ctx(v);

   auto fsm = object::Machine(mpl::type_identity<object::States>{}, ctx);

   fsm.emplace<object::Initial>();

   REQUIRE(fsm.is_in<object::Finished>());

   auto theObject = ctx.values();

   auto value = theObject.getValue("unsigned");

   REQUIRE(value);

   REQUIRE(value->get<number::JsonNumber>()->get<unsigned>() == 1234);

   auto thestring = theObject.getValue("string");

   REQUIRE(thestring);

   REQUIRE(thestring->get<std::string>() == "Das ist ein Test");

   auto isValid = theObject.getValue("isValid");

   REQUIRE(isValid);

   REQUIRE(isValid->get<bool>() == true);

   auto showDetails = theObject.getValue("showDetails");

   REQUIRE(showDetails);

   CHECK(showDetails->get<bool>() == false);
}

TEST_CASE("Object - nested", "[json]") {

   std::string_view input = "{\n"
                            "\"unsigned\":1234, \n"
                            "\"string\":\"Das ist ein Test\",\n"
                            "\"numbers\": {\n"
                            "  \"unsigned\":1234, \n"
                            "  \"real\": 1234.5678\n"
                            "},\n"
                            "\"isValid\":true, \n"
                            "\"showDetails\":false\n"
                            "}"sv;

   view v(input);

   object::Context ctx(v);

   auto fsm = object::Machine(mpl::type_identity<object::States>{}, ctx);

   fsm.emplace<object::Initial>();

   REQUIRE(fsm.is_in<object::Finished>());

   auto theObject = ctx.values();

   auto value = theObject.getValue("unsigned");

   REQUIRE(value);

   // REQUIRE(value->get<unsigned>() == 1234);

   auto thestring = theObject.getValue("string");

   REQUIRE(thestring);

   REQUIRE(thestring->get<std::string>() == "Das ist ein Test");

   auto isValid = theObject.getValue("isValid");

   REQUIRE(isValid);

   REQUIRE(isValid->get<bool>() == true);

   auto showDetails = theObject.getValue("showDetails");

   REQUIRE(showDetails);

   CHECK(showDetails->get<bool>() == false);

   auto numbers = theObject.getValue("numbers");

   REQUIRE(numbers);

   auto numbers_object = numbers->get<jsonObject>();
   REQUIRE(numbers_object);

   auto numbers_unsigned = numbers_object->getValue("unsigned");

   REQUIRE(numbers_unsigned->get<number::JsonNumber>()->get<unsigned>() ==
           1234);

   auto numbers_real = numbers_object->getValue("real");

   REQUIRE(numbers_real);

   REQUIRE(numbers_real->get<number::JsonNumber>()->get<double>() == 1234.5678);
}