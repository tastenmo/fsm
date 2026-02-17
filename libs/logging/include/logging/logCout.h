#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <string_view>

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_iostream.hpp>

#include "logEvent.h"

namespace spie {

namespace logging {

class LogCout {

public:
  LogCout(LogLevel max = LogLevel::DEBUG) : max_(max) { ; }

  /**
   * @brief not cloneable
   *
   * @param other
   */
  LogCout(LogCout &other) = delete;

  /**
   * @brief not assignable
   *
   */
  void operator=(const LogCout &) = delete;

  void slot(const LogEvent &event) {

    using namespace std::chrono;

    if (event.level <= max_) {

      // Get current time as a time_t for simple formatting
      auto sctp =
          std::chrono::time_point_cast<std::chrono::seconds>(event.time_stamp);
      auto tt = std::chrono::system_clock::to_time_t(sctp);

      auto level_name = magic_enum::enum_name(event.level);

      std::cout << "[" << std::ctime(&tt) << "\033[32m " << level_name
                << " line: " << event.location.line() << "\033[0m\t"
                << event.name << ": " << event.msg << std::endl;
    }
  }

private:
  LogLevel max_;
};

} // namespace logging
} // namespace spie
