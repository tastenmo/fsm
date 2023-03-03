#pragma once

#include <sstream>
#include <string_view>

#include "../signal/dispatcher.h"

#include "logEvent.h"

namespace escad {

namespace logging {

// using LogEventQueue = wink::event_queue<LogEvent>;

using sv = std::string_view;

class Logger : public escad::dispatcher {

private:
  Logger() {}

  // static Logger *_logger;

  /**
   * @brief not cloneable
   *
   * @param other
   */
  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;

  /**
   * @brief not assignable
   *
   */
  Logger &operator=(const Logger &) = delete;
  Logger &operator=(Logger &&) = delete;

public:
  /**
   * @brief
   *
   * @return SignalHandler*
   */
  static Logger &Initialize() {
    static Logger _logger;
    return _logger;
  }
};

struct log {
  log(LogLevel tlevel, std::string_view tname,
      const std::experimental::source_location &loc =
          std::experimental::source_location::current())
      : logger(Logger::Initialize()), loglevel(tlevel), name(tname),
        location(loc) {

    // logger_ = Logger::Initialize();
  }

  ~log() {

    if (loglevel != LogLevel::NONE) {

      logger.enqueue(LogEvent(loglevel, name, stream.str(), location));
    }
  }

  Logger &logger;
  LogLevel loglevel;
  std::string_view name;
  const std::experimental::source_location location;
  std::stringstream stream;
};

template <typename T> log &operator<<(log &record, T &&t) {
  record.stream << std::forward<T>(t);
  return record;
}

template <typename T> log &operator<<(log &&record, T &&t) {
  return record << std::forward<T>(t);
}

struct debug : public log {
  debug(std::string_view name = sv(""),
        const std::experimental::source_location &loc =
            std::experimental::source_location::current())
      : log(LogLevel::DEBUG, name, loc) {}
};

struct info : public log {
  info(std::string_view name = sv(""),
       const std::experimental::source_location &loc =
           std::experimental::source_location::current())
      : log(LogLevel::INFO, name, loc) {}
};

struct notice : public log {
  notice(std::string_view name = sv(""),
         const std::experimental::source_location &loc =
             std::experimental::source_location::current())
      : log(LogLevel::NOTICE, name, loc) {}
};

struct warn : public log {
  warn(std::string_view name = sv(""),
       const std::experimental::source_location &loc =
           std::experimental::source_location::current())
      : log(LogLevel::WARNING, name, loc) {}
};

struct error : public log {
  error(std::string_view name = sv(""),
        const std::experimental::source_location &loc =
            std::experimental::source_location::current())
      : log(LogLevel::ERROR, name, loc) {}
};

struct fatal : public log {
  fatal(std::string_view name = sv(""),
        const std::experimental::source_location &loc =
            std::experimental::source_location::current())
      : log(LogLevel::FATAL, name, loc) {}
};

struct alert : public log {
  alert(std::string_view name = sv(""),
        const std::experimental::source_location &loc =
            std::experimental::source_location::current())
      : log(LogLevel::ALERT, name, loc) {}
};

struct emerg : public log {
  emerg(std::string_view name = sv(""),
        const std::experimental::source_location &loc =
            std::experimental::source_location::current())
      : log(LogLevel::EMERG, name, loc) {}
};

struct none : public log {
  none(std::string_view name = sv(""),
       const std::experimental::source_location &loc =
           std::experimental::source_location::current())
      : log(LogLevel::NONE, name, loc) {}
};

} // namespace logging
} // namespace escad
