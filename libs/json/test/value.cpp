#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include "core/utils.h"

#include <json/value.h>

using namespace spie::json;

using namespace std::literals;

TEST_CASE("Value - test_data", "[json]") {

   std::ifstream ifs;

   ifs.open("/workspaces/fsm/libs/json/test/data/1.json", std::ios::in);

   REQUIRE(ifs);

   std::string str(std::istreambuf_iterator<char>{ifs}, {});

   view v(str);

   value::Context ctx(v);

   auto fsm = value::Machine(mpl::type_identity<value::States>{}, ctx);

   fsm.emplace<value::Initial>();

   REQUIRE(fsm.is_in<value::Finished>());

   auto theObject = ctx.getValue();

   std::cout << "Parsed JSON: " << theObject.toString() << std::endl;

   REQUIRE(theObject.is<jsonObject>());

   auto myObject = theObject.get<jsonObject>();

   REQUIRE(myObject);
}