/**
 * 
*/

#pragma once

//#include "wink/slot.hpp"
#include <chrono>
#include <csignal>
#include <ctime>
#include <experimental/source_location>
#include <string_view>

namespace escad {

namespace logging {

enum class LogLevel { FATAL, ERROR, WARNING, INFO, DEBUG, TRACE, NONE };

struct LogEvent {

  LogLevel level;

  std::chrono::system_clock::time_point time_stamp;

  std::experimental::source_location location;

  std::string msg;

  LogEvent(LogLevel tlevel, const std::string tmsg,
           const std::experimental::source_location &tloc)
      : level(tlevel), location(tloc), msg(tmsg) {

    time_stamp = std::chrono::system_clock::now();
    ;
  }
};

//using LogEventSlot = wink::slot<void(const LogEvent &)>;

}
}
