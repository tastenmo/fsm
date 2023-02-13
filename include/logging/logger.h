#pragma once

#include <sstream>

#include "../signal/dispatcher.h"

#include "logEvent.h"

namespace escad {

namespace logging{

//using LogEventQueue = wink::event_queue<LogEvent>;

class Logger : public escad::dispatcher {

private:
  Logger() { }

  //static Logger *_logger;

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
  Logger& operator=(const Logger &) = delete;
  Logger& operator=(Logger &&) = delete;

  public:  

/**
 * @brief 
 * 
 * @return SignalHandler* 
 */
  static Logger& Initialize()
  {
    static Logger _logger;
    return _logger;
  }

};

struct log
{
  log(LogLevel tlevel, const std::experimental::source_location& loc = std::experimental::source_location::current()): logger(Logger::Initialize()), loglevel(tlevel), location(loc) {

    //logger_ = Logger::Initialize();
    
  }

  ~log() {

    if(loglevel != LogLevel::NONE){

      logger.enqueue(LogEvent(loglevel, stream.str(), location));
    }
  }

  Logger &logger;
  LogLevel loglevel;
  const std::experimental::source_location location;
  std::stringstream stream;
};

template <typename T>
log& operator<<(log& record, T&& t) {
    record.stream << std::forward<T>(t);
    return record;
}

template <typename T>
log& operator<<(log&& record, T&& t) {
    return record << std::forward<T>(t);
}

struct debug : public log
{
  debug(const std::experimental::source_location& loc= std::experimental::source_location::current()) : log(LogLevel::DEBUG, loc) {}

};

struct info : public log
{
  info(const std::experimental::source_location& loc= std::experimental::source_location::current()) : log(LogLevel::INFO, loc) {}

};

struct warn : public log
{
  warn(const std::experimental::source_location& loc= std::experimental::source_location::current()) : log(LogLevel::WARNING, loc) {}

};

struct error : public log
{
  error(const std::experimental::source_location& loc= std::experimental::source_location::current()) : log(LogLevel::ERROR, loc) {}

};

struct fatal : public log
{
  fatal(const std::experimental::source_location& loc= std::experimental::source_location::current()) : log(LogLevel::FATAL, loc) {}

};

struct none : public log {
  none(const std::experimental::source_location& loc= std::experimental::source_location::current()) : log(LogLevel::NONE, loc) {}
  
};

}
}
