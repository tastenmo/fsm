#include <functional>
#include <utility>
#include <iostream>
#include <chrono>

#include <catch2/catch_test_macros.hpp>

#include <date/date.h>
#include <date/tz.h>

using namespace date;
using namespace std::literals;


TEST_CASE("date", "[date]"){

    std::stringstream ss;

    std::chrono::system_clock::time_point time_stamp =
        std::chrono::system_clock::now();

    // std::cout << "time_stamp: "
    //           << format("%D %T",
    //                     floor<std::chrono::seconds>(time_stamp))
    //           << std::endl;

    ss << floor<std::chrono::seconds>(time_stamp);

    std::cout << ss.str() << std::endl;

    std::cout << format("%Y-%m-%d %H:%M:%S", floor<std::chrono::seconds>(time_stamp)) << std::endl;

    std::stringstream ss_local;
    auto t = make_zoned(current_zone(), floor<std::chrono::seconds>(time_stamp));
    ss_local << t;

    std::cout << ss_local.str() << std::endl;
    std::cout << format("%Y-%m-%d %H:%M:%S", t) << std::endl;



}