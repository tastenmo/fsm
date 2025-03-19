#pragma once

#include <cstddef>
#include <ctre.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <magic_enum/magic_enum.hpp>

#include <core/type_traits.h>

using namespace std::literals;

namespace escad::json {

struct view {

  view(std::string_view input) : input_(input) {}

  std::string_view substr() const { return input_.substr(pos_); }

  std::string_view substr(std::size_t len) const {
    return input_.substr(pos_, len);
  }

  std::string_view substr(std::size_t start, std::size_t len) const {
    return input_.substr(start, len);
  }

  std::string_view consume(std::size_t len) {
    auto result = input_.substr(pos_, len);
    pos_ += len;
    return result;
  }

  std::string_view input_;
  std::size_t pos_ = 0;
};

template <class TokenType, ctll::fixed_string Regex> class tokenizer {

public:
  using Token = TokenType;

  static constexpr auto TokenTypeSize = magic_enum::enum_count<TokenType>();

  tokenizer(view &input) : view_(input) {}

  tokenizer(view &&input) : view_(input) {}

  // Get the next token
  std::optional<TokenType> next() {

    bool found = false;
    TokenType result;
    if (auto match = ctre::multiline_starts_with<Regex>(view_.substr())) {

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
    if (auto match = ctre::multiline_starts_with<Regex>(view_.substr())) {
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
    if (auto match = ctre::multiline_starts_with<Regex>(view_.substr())) {
      view_.consume(match.size());
      return view_.consume(match.size());
    }
    return {};
  }

  std::optional<std::string_view> consume(TokenType type) {

    bool found = false;
    std::string_view str = ""sv;

    if (auto match = ctre::multiline_starts_with<Regex>(view_.substr())) {

      auto type_integer = magic_enum::enum_integer(type);

      mpl::for_sequence(std::make_index_sequence<TokenTypeSize>{}, [&](auto i) {
        if (i == type_integer) {
          if (auto capture = match.template get<i + 1>()) {
            str = view_.consume(capture.size());
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

  const view &getView() const { return view_; }

  // protected:
  view &view_;
};

// Regex for JSON tokens
constexpr auto jsonTokenRegex = ctll::fixed_string{
    "(\\s+)|(\\u007b)|(\\u007d)|(\\u005b)|(\\u005d)|(:)|(,)|(\")|"
    "(true)|(false)|(null)"};

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
  NULL_
};

using jsonTokenizer = tokenizer<jsonTokenType, jsonTokenRegex>;

// Regex for String tokens
constexpr auto stringTokenRegex =
    ctll::fixed_string{"(\")"
                       "|(\\u005Cu[0-9a-fA-F]{4})"
                       "|([^\"\\u005C\\u0000-\\u001f\\u007F]+)"
                       "|(\\u005C[bfnrt/\\\"])"};

enum class stringTokenType { DOUBLE_QUOTE, HEX, CHARS, ESCAPE };

using stringTokenizer = tokenizer<stringTokenType, stringTokenRegex>;

constexpr auto numberTokenRegex = ctll::fixed_string{"([+\\-])"
                                                     "|([0-9])"
                                                     "|(\\.)"
                                                     "|([eE])"};

enum class numberTokenType { SIGN, DIGIT, DOT, EXP };

using numberTokenizer = tokenizer<numberTokenType, numberTokenRegex>;

} // namespace escad::json