#include <catch2/catch_all.hpp>
#include <reflect/property.h>
#include <iostream>

#include <ctre.hpp>

using namespace std::string_view_literals;

struct TestStruct {
   int a;
   float b;
   std::string c;
};

TEST_CASE("Single Property", "[reflect]") {

   auto prop_const_char = spie::reflect::property(&TestStruct::a, "abcd");

   STATIC_REQUIRE(
       std::is_same_v<decltype(prop_const_char),
                      spie::reflect::details::property<TestStruct, int,
                                                      const char *>>);
   REQUIRE(std::strcmp(prop_const_char.name, "abcd") == 0);

   auto prop_sv = spie::reflect::property(&TestStruct::a, "bcd"sv);

   STATIC_REQUIRE(
       std::is_same_v<decltype(prop_sv),
                      spie::reflect::details::property<TestStruct, int,
                                                      std::string_view>>);

   REQUIRE(prop_sv.name == "bcd"sv);
}

struct TestStruct2 {
   int a;
   float b;
   std::string c;

   constexpr static auto properties = std::make_tuple(
       spie::reflect::property(&TestStruct2::a, "a"sv),
       spie::reflect::property(&TestStruct2::b, "b"sv),
       spie::reflect::property(&TestStruct2::c, "c"sv));
};

TEST_CASE("Multiple Properties", "[reflect]") {

   TestStruct2 object{42, 3.14f, "Hello"};

   constexpr static auto numberOfProperties =
       std::tuple_size<decltype(TestStruct2::properties)>::value;

   STATIC_REQUIRE(numberOfProperties == 3u);

   mpl::for_sequence(
       std::make_index_sequence<numberOfProperties>{}, [&](auto i) {
          constexpr auto property = std::get<i>(TestStruct2::properties);

          std::cout << property.name << ": " << object.*(property.member)
                    << std::endl;
       });
}
