#include <functional>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <logging/logger.h>

#include <logging/logCout.h>

namespace logging = escad::logging;




TEST_CASE("Logger basic", "[Logging]"){

    logging::Logger &logger = logging::Logger::Initialize();

    LogCout logCout_(logging::LogLevel::DEBUG);

    REQUIRE(logger.size() == 0u);

    logging::info() << "Logger not started.";
    logger.update();

    REQUIRE(logger.size() == 0u);

    logger.slot<logging::LogEvent>().connect<&LogCout::slot>(logCout_);

    logging::info() << "Logger started.";

    REQUIRE(logger.size<logging::LogEvent>() == 1u);

    logger.update();

    REQUIRE(logger.size<logging::LogEvent>() == 0u);





}