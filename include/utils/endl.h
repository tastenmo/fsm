/**
 * @file endl.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2022-07-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <variant>

#include <ctre.hpp>

namespace escad {

namespace utils {

constexpr static auto endl_pattern = ctll::fixed_string{"([^\\n\\r\\t]*)"
                                                        "((\\r)|(\\n)|(\\t))"};

inline std::string replace_endl(const std::string str) {

  std::string result;

  for (auto match : ctre::multiline_range<endl_pattern>(str)) {

    if (match.get<1>()) {
      result += match.get<1>().to_view();
    }
    if (match.get<3>()) {
      result += "\\r";
    }
    if (match.get<4>()) {
      result += "\\n";
    }
    if (match.get<5>()) {
      result += "\\t";
    }
  }

  return result;
}

} // namespace utils
} // namespace escad
