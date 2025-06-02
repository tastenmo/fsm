/**
 * 
*/

#pragma once

//#include "wink/slot.hpp"
#include <chrono>
#include <csignal>
#include <ctime>
#include <source_location>
#include <string_view>

namespace escad {

namespace logging {

enum class LogLevel {EMERG = 0, ALERT = 1, FATAL = 2, ERROR = 3, WARNING = 4, NOTICE = 5, INFO = 6, DEBUG = 7, TRACE, NONE };

struct LogEvent {

  LogLevel level;

  std::chrono::system_clock::time_point time_stamp;

  std::source_location location;

  std::string name;

  std::string msg;

  LogEvent(LogLevel tlevel, std::string_view tname, std::string_view tmsg,
           const std::source_location &tloc)
      : level(tlevel), location(tloc), name(tname), msg(tmsg) {

    time_stamp = std::chrono::system_clock::now();
    ;
  }
};

//using LogEventSlot = wink::slot<void(const LogEvent &)>;

}
}
