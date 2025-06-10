#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <ctre.hpp>

#include <json/tokenizer.h>

using namespace escad::json;
using namespace std::literals;

TEST_CASE("static tokens", "[json]") {

  // STATIC_REQUIRE(jsonTokenType == 13);

  REQUIRE(magic_enum::enum_value<jsonTokenType>(0) == jsonTokenType::WS);

  STATIC_REQUIRE(ctre::multiline_starts_with<jsonTokenRegex>(" test"));

  STATIC_REQUIRE(ctre::multiline_starts_with<jsonTokenRegex>("{optional}"));

  auto match = ctre::multiline_starts_with<jsonTokenRegex>("{optional}");

  REQUIRE(match);

  REQUIRE(match.template get<2>());
  REQUIRE(match.template get<2>().to_view() == "{");

  STATIC_REQUIRE(ctre::multiline_starts_with<stringTokenRegex>(" test"));
  STATIC_REQUIRE(ctre::multiline_starts_with<stringTokenRegex>("\u00B5"));
  STATIC_REQUIRE(ctre::multiline_starts_with<stringTokenRegex>("\\n"));
}

TEST_CASE("simple tokens", "[json]") {

  view v{"  {  }\\u1234"sv};

  REQUIRE(v.pos_ == 0);

  jsonTokenizer t(v);

  REQUIRE(t.next() == jsonTokenizer::Token::WS);
  auto result = t.consume(jsonTokenizer::Token::WS);
  REQUIRE(result);
  REQUIRE(*result == "  ");
  REQUIRE(v.pos_ == 2);

  result = t.consume(jsonTokenizer::Token::OPEN_BRACE);
  REQUIRE(result);
  REQUIRE(*result == "{");

  REQUIRE(t.consume(jsonTokenizer::Token::WS) == "  "sv);

  REQUIRE_FALSE(t.consume(jsonTokenizer::Token::WS));

  REQUIRE(t.consume(jsonTokenizer::Token::CLOSE_BRACE) == "}"sv);

  // REQUIRE(t.consume(jsonTokenizer::Token::HEX) == "\\u1234"sv);

  REQUIRE(t.next() == std::nullopt);
}

TEST_CASE("Json_String", "[json]") {

  stringTokenizer t(view{"\"1234 \\n\\u00B5\""sv});

  REQUIRE(t.consume(stringTokenizer::Token::DOUBLE_QUOTE) == "\""sv);

  REQUIRE(t.consume(stringTokenizer::Token::CHARS) == "1234 "sv);

  REQUIRE(t.consume(stringTokenizer::Token::ESCAPE) == "\\n"sv);

  REQUIRE(t.consume(stringTokenizer::Token::HEX) == "\\u00B5"sv);

  REQUIRE(t.consume(stringTokenizer::Token::DOUBLE_QUOTE) == "\""sv);

  REQUIRE(t.next() == std::nullopt);
}