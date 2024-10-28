#include "json/tokenizer.h"
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/string.h>

using namespace escad::json;

using namespace std::literals;

struct start {};

TEST_CASE("simple strings", "[json]") {

  tokenizer t("\"This is a simple string.\" this should be ignored."sv);

  string::Context ctx(t);

  auto fsm = string::Initial::create(ctx);

  REQUIRE(fsm.is_in<string::Finished>());

  REQUIRE(ctx.value() == "This is a simple string."sv);

  REQUIRE(ctx.size() == 24);
}
