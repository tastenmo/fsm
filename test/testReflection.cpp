#include <functional>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <ctre.hpp>
#include <reflection/struct_string.h>

// Declare the regex
constexpr auto rx = ctll::fixed_string{
    "TestStruct\\{([0-9]+);([\\-\\+]?[0-9]+[.,][0-9]+);(\\w+)\\}"};
// Test it matches a string at compile time
static_assert(ctre::match<rx>("TestStruct{1;2.321;test}"));
// static_assert(ctre::match<rx>("1234567890"));
// [-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?

namespace refl = escad::reflection;

struct TestStruct {

  int a;
  float b;
  std::string c;

  constexpr static auto pattern = ctll::fixed_string{
      "TestStruct\\{([0-9]+);([\\-\\+]?[0-9]+[.,][0-9]+);(\\w+)\\}"};

  constexpr static auto properties = std::make_tuple(
      refl::property(&TestStruct::a, "a"), refl::property(&TestStruct::b, "b"),
      refl::property(&TestStruct::c, "c"));
};

TEST_CASE("StructString basic", "[Reflection]") {

  TestStruct ts{1, 2.314f, "test"};

  auto str = refl::toString(ts);

  REQUIRE(str == "a = 1;\nb = 2.314;\nc = test;\n");

  auto ts2 = refl::fromString<TestStruct>("TestStruct{1;2.314;test}");

  REQUIRE(ts2.has_value());

  REQUIRE(ts2->a == 1);
  REQUIRE(ts2->b == 2.314f);
  REQUIRE(ts2->c == "test");
}
