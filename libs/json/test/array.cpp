
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/array.h>

using namespace spie::json;

using namespace std::literals;

TEST_CASE("Array - numbers", "[json]") {

   view v("[ 0, 1, 2, 3, 4, 5]"sv);

   array::Context ctx(v);

   auto fsm = array::Machine(mpl::type_identity<array::States>{}, ctx);

   fsm.emplace<array::Initial>();

   REQUIRE(fsm.is_in<array::Finished>());

   auto theArray = ctx.values();

   auto value_obj = theArray.getValue(0);

   REQUIRE(value_obj);

   auto value = value_obj->get<number::JsonNumber>();

   REQUIRE(value);

   REQUIRE(value->get<unsigned>() == 0);
}
