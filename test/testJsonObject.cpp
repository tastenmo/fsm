#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/object.h>

using namespace escad::json;

using namespace std::literals;

TEST_CASE("Json_object", "[json]") {

  view v("{\"unsigned\":1234,\"string\":\"Das ist ein Test\"}"sv);

  object::Context ctx(v);

  auto fsm = StateMachine(mpl::type_identity<object::States>{}, ctx);

  fsm.emplace<object::Initial>();

  REQUIRE(fsm.is_in<object::Finished>());
}
