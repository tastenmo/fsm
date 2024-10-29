#pragma once

#include <bits/utility.h>
#include <cstddef>
#include <ctre.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <magic_enum.hpp>

#include "base/type_traits.h"

using namespace std::literals;

namespace escad::json {

template <class TokenType, ctll::fixed_string Regex> class tokenizer {

public:
  using Token = TokenType;

  static constexpr auto TokenTypeSize = magic_enum::enum_count<TokenType>();

  tokenizer(std::string_view input, std::size_t pos = 0)
      : input_(input), pos_(pos) {}

  // Get the next token
  std::optional<TokenType> next() {

    bool found = false;
    TokenType result;
    if (auto match = ctre::multiline_starts_with<Regex>(input_.substr(pos_))) {

      mpl::for_sequence(std::make_index_sequence<TokenTypeSize>{}, [&](auto i) {
        if (match.template get<i + 1>()) {
          result = magic_enum::enum_value<TokenType>(i);
          found = true;
        }
      });
    }
    if (found) {
      return result;
    }
    return std::nullopt;
  }

  bool isToken(TokenType type) {
    bool found = false;
    if (auto match = ctre::multiline_starts_with<Regex>(input_.substr(pos_))) {
      auto type_integer = magic_enum::enum_integer(type);
      mpl::for_sequence(std::make_index_sequence<TokenTypeSize>{}, [&](auto i) {
        if (i == type_integer) {
          if (match.template get<i + 1>()) {
            found = true;
          }
        }
      });
    }
    return found;
  }

  [[deprecated("Don't use this function anymore")]] std::string_view consume() {
    if (auto match = ctre::multiline_starts_with<Regex>(input_.substr(pos_))) {
      pos_ += match.size();
      return match.to_view();
    }
    return {};
  }

  std::optional<std::string_view> consume(TokenType type) {

    bool found = false;
    std::string_view str = ""sv;

    if (auto match = ctre::multiline_starts_with<Regex>(input_.substr(pos_))) {

      auto type_integer = magic_enum::enum_integer(type);

      mpl::for_sequence(std::make_index_sequence<TokenTypeSize>{}, [&](auto i) {
        if (i == type_integer) {
          if (auto capture = match.template get<i + 1>()) {
            pos_ += capture.size();
            str = capture.to_view();
            found = true;
          }
        }
      });
    }
    if (found) {
      return str;
    }
    return std::nullopt;
  }

protected:
  std::string_view input_;

  std::size_t pos_ = 0;
};

// Regex for JSON tokens
constexpr auto jsonTokenRegex = ctll::fixed_string{
    "(\\s+)|(\\u007b)|(\\u007d)|(\\u005b)|(\\u005d)|(:)|(,)|(\")|"
    "(true)|(false)|(null)|(\\u005Cu[0-9a-fA-F]{4})|([^\"\\u005C\\u0000-"
    "\\u001f\\u007F])"};

enum class jsonTokenType {
  WS,
  OPEN_BRACE,
  CLOSE_BRACE,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  COLON,
  COMMA,
  DOUBLE_QUOTE,
  TRUE,
  FALSE,
  NULL_,
  HEX,
  STRING
};

using jsonTokenizer = tokenizer<jsonTokenType, jsonTokenRegex>;

// Regex for JSON tokens
constexpr auto stringTokenRegex =
    ctll::fixed_string{"(\")"
                       "|(\\u005Cu[0-9a-fA-F]{4})"
                       "|([^\"\\u005C\\u0000-\\u001f\\u007F]+)"
                       "|(\\u005C[bfnrt/\\\"])"};

enum class stringTokenType { DOUBLE_QUOTE, HEX, CHARS, ESCAPE };

using stringTokenizer = tokenizer<stringTokenType, stringTokenRegex>;

} // namespace escad::json