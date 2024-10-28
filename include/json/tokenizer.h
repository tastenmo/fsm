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

namespace escad::json {

// Regex for JSON tokens
constexpr auto tokenRegEx = ctll::fixed_string{
    "(\\s+)|(\\u007b)|(\\u007d)|(\\u005b)|(\\u005d)|(:)|(,)|(\")|"
    "(true)|(false)|(null)|(\\u005Cu[0-9a-fA-F]{4})|([^\"\\u005C\\u0000-"
    "\\u001f\\u007F])"};

enum class tokenType {
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

constexpr auto tokenTypeSize = magic_enum::enum_count<tokenType>();

class tokenizer {

public:
  tokenizer(std::string_view input) : input_(input), pos_(0) {}

  // Get the next token
  std::optional<tokenType> next() {

    if (auto match =
            ctre::multiline_starts_with<tokenRegEx>(input_.substr(pos_))) {

      tokenType result;

      mpl::for_sequence(std::make_index_sequence<tokenTypeSize>{}, [&](auto i) {
        if (match.template get<i + 1>()) {
          result = magic_enum::enum_value<tokenType>(i);
        }
      });

      return result;
    }
    return std::nullopt;
  }

  bool isToken(tokenType type) { return next() == type; }

  std::string_view consume() {
    if (auto match =
            ctre::multiline_starts_with<tokenRegEx>(input_.substr(pos_))) {
      pos_ += match.size();
      return match.to_view();
    }
    return {};
  }

protected:
  std::string_view input_;

  std::size_t pos_ = 0;
};

} // namespace escad::json