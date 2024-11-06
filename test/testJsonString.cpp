
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/tokenizer.h>
#include <json/string.h>

using namespace escad::json;

using namespace std::literals;

struct start {};

TEST_CASE("Json_simple strings", "[json]") {

  view v{"\"This is a simple string.\" this should be ignored."sv};

  string::Context ctx(v);

  auto fsm = string::Initial::create(ctx);

  REQUIRE(fsm.is_in<string::Finished>());

  REQUIRE(ctx.value() == "This is a simple string."sv);

  REQUIRE(ctx.size() == 24);
}

TEST_CASE("Json_strings with special characters", "[json]") {


  string::Context ctx(view{
      "\"This is a more complicated string:\\n"
      "\\tIt contains special character,\\n"
      "\\tcompiles in 10 \\u00B5s,\\n"
      "\\tand only needs \\u00BD the time.\" this should be ignored."sv});

  auto fsm = string::Initial::create(ctx);

  REQUIRE(fsm.is_in<string::Finished>());

  //  REQUIRE(ctx.value() == "This is a simple string."sv);

  REQUIRE(ctx.size() == 130);
  auto value = ctx.value();
  CAPTURE(value);
}
