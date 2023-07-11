#include <functional>
#include <utility>
#include <cstring>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <ctre.hpp>
#include <reflection/struct_string.h>

using namespace std::string_view_literals;

TEST_CASE("regex", "[Reflection]"){

  constexpr auto rx = ctll::fixed_string{
    "TestStruct\\{(?<a>[0-9]+);(?<b>[\\-\\+]?[0-9]+[.,][0-9]+);(?<c>\\w+)\\}"};
// Test it matches a string at compile time
  STATIC_REQUIRE(ctre::match<rx>("TestStruct{1;2.321;test}"));

  constexpr auto match = ctre::match<rx>("TestStruct{1;2.321;test}");

  STATIC_REQUIRE(match);
  STATIC_REQUIRE(match.get<1>().to_view() == "1"sv);
  STATIC_REQUIRE(match.get<2>().to_view() == "2.321"sv);
  STATIC_REQUIRE(match.get<3>().to_view() == "test"sv);
  //STATIC_REQUIRE(match.get<3>() == "test"):

  constexpr auto key_a = ctll::fixed_string{"a"};
  constexpr auto key_b = ctll::fixed_string{"b"};

  STATIC_REQUIRE(match.get<key_a>().to_view() == "1"sv);
  STATIC_REQUIRE(match.get<key_b>().to_view() == "2.321"sv);
  STATIC_REQUIRE(match.get<ctll::fixed_string{"c"}>().to_view() == "test"sv);

}

// Declare the regex


// static_assert(ctre::match<rx>("1234567890"));
// [-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?

namespace refl = escad::reflection;

struct TestStruct {

  int a;
  float b;
  std::string c;

  constexpr static auto pattern = ctll::fixed_string{
      "TestStruct\\{(?<a>[0-9]+);(?<b>[\\-\\+]?[0-9]+[.,][0-9]+);(?<c>\\w+)\\}"};

  constexpr static auto properties = std::make_tuple(
      refl::property(&TestStruct::a, ctll::fixed_string{"a"}), refl::property(&TestStruct::b, ctll::fixed_string{"b"}),
      refl::property(&TestStruct::c, ctll::fixed_string{"c"}));
};


struct TestStruct2 : refl::properties {

  int a;
  float b;
  std::string c;

  //constexpr static auto props = refl::properties::add(refl::property(&TestStruct2::a, "a"));

  
//  constexpr TestStruct2() : refl::properties{refl::property(&TestStruct2::a, "a")} {}




};

TEST_CASE("Value list", "[Reflection]"){

  using type = mpl::type_list<int, char>;
  using other = mpl::type_list<double>;

  STATIC_REQUIRE(type::size == 2u);
  STATIC_REQUIRE(other::size == 1u);

  
  using val_list = mpl::value_list<17, 2, 'c'>;

  STATIC_REQUIRE(val_list::size == 3u);

  STATIC_REQUIRE(mpl::value_list_element_v<0u, val_list> == 17);

  constexpr static auto prop = refl::property(&TestStruct::a, "a");

  constexpr static auto prop1 = refl::property(&TestStruct::b, "b"sv);

  constexpr static auto prop2 = refl::property(&TestStruct::c, ctll::fixed_string{"c"});

  using prop_list = mpl::value_list<&prop, &prop1, &prop2>;

  STATIC_REQUIRE(prop_list::size == 3u);

  STATIC_REQUIRE(mpl::value_list_element_v<1u, prop_list>->name == "b"sv);

  STATIC_REQUIRE(mpl::value_list_element_v<2u, prop_list>->name.is_same_as(ctll::fixed_string{"c"}));

  using prop_tuple = mpl::value_list_expand_t<val_list>;

  STATIC_REQUIRE(std::is_same_v<std::tuple<int, int, char>, prop_tuple>);


}

TEST_CASE("Property", "[Reflection]") {

  auto prop_const_char = refl::property(&TestStruct::a, "abcd");

  STATIC_REQUIRE(
      std::is_same_v<decltype(prop_const_char),
                     refl::details::property<TestStruct, int, const char*>>);
  REQUIRE(std::strcmp(prop_const_char.name, "abcd")==0);

  auto prop_sv = refl::property(&TestStruct::a, "bcd"sv);

  STATIC_REQUIRE(
      std::is_same_v<decltype(prop_sv),
                     refl::details::property<TestStruct, int, std::string_view>>);

  REQUIRE(prop_sv.name == "bcd"sv);
}

TEST_CASE("Properties", "[Reflection]") {

  //auto props = refl::properties(refl::property(&TestStruct::a, "a"),
  //                       refl::property(&TestStruct::b, "b"),
  //                       refl::property(&TestStruct::c, "c"));

  //REQUIRE(props.get<1>().name == "b"sv);

}

TEST_CASE("StructString basic", "[Reflection]") {

  TestStruct ts{1, 2.314f, "test"};

  auto str = refl::toString(ts);

  REQUIRE(str == "1;\n2.314;\ntest;\n");

  auto ts2 = refl::fromString<TestStruct>("TestStruct{1;2.314;test}");

  REQUIRE(ts2.has_value());

  REQUIRE(ts2->a == 1);
  REQUIRE(ts2->b == 2.314f);
  REQUIRE(ts2->c == "test");
}
