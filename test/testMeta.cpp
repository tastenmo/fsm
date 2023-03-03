#include <catch2/catch_all.hpp>

#include <meta_container/meta_counter.hpp>
#include <meta_container/meta_list.hpp>
#include <meta_container/type_list.hpp>

#include <type_traits> // std::is_same

TEST_CASE("MetaCounter", "[meta_containers]") {
  using C1 = atch::meta_counter<class Counter1>;
  using C2 = atch::meta_counter<class Counter2>;

  C1::next(); // 1
  C1::next(); // 2
  C1::next(); // 3

  C2::next(); // 1
  C2::next(); // 2

  STATIC_REQUIRE(C1::value() == 3);
  STATIC_REQUIRE(C2::value() == 2);
}

TEST_CASE("MetaList", "[meta_containers]") {
  using LX = atch::meta_list<class b_atch_se>;

  LX::push<void, void, void, void>();
  LX::set<0, class Hello>();
  LX::set<2, class World>();
  LX::pop();

  LX::value<> x; // type_list<class Hello, void, class World>

  STATIC_REQUIRE(std::is_same<atch::type_list<class Hello, void, class World>,
                              LX::value<>>::value);
}