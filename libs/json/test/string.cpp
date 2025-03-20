
#include <core/type_traits.h>

#include <catch2/catch_test_macros.hpp>

#include <json/string.h>
#include <json/tokenizer.h>

using namespace escad::json;

using namespace std::literals;

TEST_CASE("Json_simple strings", "[json]") {

  auto fsm = StateMachine(
      mpl::type_identity<string::States>{},
      string::Context(
          view{"\"This is a simple string.\" this should be ignored."sv}));

  fsm.emplace<string::Initial>();

  REQUIRE(fsm.is_in<string::Finished>());

  REQUIRE(fsm.context().value() == "This is a simple string."sv);

  REQUIRE(fsm.context().size() == 24);
}

TEST_CASE("Json_strings with special characters", "[json]") {

  string::Context ctx(
      view{"\"This is a more complicated string:\\n"
           "\\tIt contains special character,\\n"
           "\\tcompiles in 10 \\u00B5s,\\n"
           "\\tand only needs \\u00BD the time.\" this should be ignored."sv});

  auto fsm = StateMachine(mpl::type_identity<string::States>{}, ctx);

  fsm.emplace<string::Initial>();

  REQUIRE(fsm.is_in<string::Finished>());

  //  REQUIRE(ctx.value() == "This is a simple string."sv);

  REQUIRE(ctx.size() == 130);
  auto value = ctx.value();
  CAPTURE(value);
}
