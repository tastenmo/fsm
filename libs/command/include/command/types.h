#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <string>
#include <string_view>

namespace spie::command {

struct parse_storage {
  std::string input{};
  std::deque<std::string> strings{};

  [[nodiscard]] std::string_view input_view() const { return input; }

  std::string_view own(std::string_view value) {
    strings.emplace_back(value);
    return strings.back();
  }
};

enum class parse_error_code {
  none,
  empty_input,
  invalid_syntax,
  unknown_command,
  argument_count_mismatch,
  conversion_error,
  json_error,
  json_shape_error,
  ambiguous_command
};

struct parse_error {
  parse_error_code code = parse_error_code::none;
  std::string message{};
  std::size_t position = 0;
  std::size_t argument_index = 0;
};

template <class Registry> struct parse_result {
  typename Registry::command_variant command{};
  parse_error error{};
  std::shared_ptr<parse_storage> storage = std::make_shared<parse_storage>();

  [[nodiscard]] bool ok() const { return error.code == parse_error_code::none; }
};

} // namespace spie::command
