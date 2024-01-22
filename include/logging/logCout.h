#pragma once

#include <chrono>
#include <iostream>
#include <string_view>

#include <date/date.h>
#include <magic_enum.hpp>

#include "logEvent.h"

// namespace escad {

// namespace logging {

namespace logging = escad::logging;

class LogCout {

public:
  LogCout(logging::LogLevel max = logging::LogLevel::DEBUG) : max_(max) { ; }

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

  void slot(const logging::LogEvent &event) {

    // auto level_name = magic_enum::enum_name(event.level);

    using namespace date;
    using namespace std::chrono;
    using namespace magic_enum::ostream_operators;

    if (event.level <= max_) {

      std::cout << "[" << event.time_stamp << "] "
                << "\033[32m " << event.level
                << " line: "
                //<< event.location.function_name() << " - "
                //<< event.location.file_name() << ": line "
                << event.location.line() << "\033[0m\t" << event.name << ": "
                << event.msg << std::endl;
    }

    //    std::cerr << "<" << magic_enum::enum_integer(event.level) << "> "
    //            << "[" << event.time_stamp << "] "
    //            << event.level << "/"
    //            << event.location.function_name()
    //            << "\033[0m\t" << event.msg << std::endl;
  }

private:
  logging::LogLevel max_;
};

//}
//}
