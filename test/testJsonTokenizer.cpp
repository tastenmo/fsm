#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include <json/tokenizer.h>

using namespace escad::json;

TEST_CASE("static tokens", "[json]") {

  STATIC_REQUIRE(tokenTypeSize == 13);

  REQUIRE(magic_enum::enum_value<tokenType>(0) == tokenType::WS);

  STATIC_REQUIRE(ctre::multiline_starts_with<tokenRegEx>(" test"));

  STATIC_REQUIRE(ctre::multiline_starts_with<tokenRegEx>("{optional}"));

  auto match = ctre::multiline_starts_with<tokenRegEx>("{optional}");

  REQUIRE(match);

  REQUIRE(match.template get<2>());
  REQUIRE(match.template get<2>().to_view() == "{");
}

TEST_CASE("simple tokens", "[json]") {

  tokenizer t(" {  }\\u1234");

  REQUIRE(t.next() == tokenType::WS);
  REQUIRE(t.consume() == " ");

  REQUIRE(t.next() == tokenType::OPEN_BRACE);
  REQUIRE(t.consume() == "{");

  REQUIRE(t.next() == tokenType::WS);
  REQUIRE(t.consume() == "  ");

  REQUIRE(t.next() == tokenType::CLOSE_BRACE);
  REQUIRE(t.consume() == "}");

  REQUIRE(t.next() == tokenType::HEX);
  REQUIRE(t.consume() == "\\u1234");

  REQUIRE(t.next() == std::nullopt);
}