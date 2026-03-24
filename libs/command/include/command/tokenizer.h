#pragma once

#include <json/tokenizer.h>

namespace spie::command {

using view = json::view;

constexpr auto commandTokenRegex = ctll::fixed_string{
    "(\\s+)|([A-Za-z_][A-Za-z0-9_]*)|(true|false)|([+\\-]?[0-9]+\\.[0-9]+)|"
    "([+\\-]?[0-9]+)|(\\\"[^\\\"]*\\\")|([^\\s]+)"};

enum class commandTokenType {
  WS,
  IDENTIFIER,
  BOOLEAN_LITERAL,
  FLOAT_LITERAL,
  INT_LITERAL,
  QUOTED_STRING,
  BARE_STRING
};

using commandTokenizer = json::tokenizer<commandTokenType, commandTokenRegex>;

} // namespace spie::command
