
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <fsm/transition.h>

using namespace spie::fsm;

struct event1 {};
struct event2 {};
struct event3 {};
struct event4 {

   event4(int val) : value_(val) {}

   int value_;
};

struct event5 {};

struct StateFirst {};
struct StateSecond {};
struct StateThird {};

auto transitionTo(const event1 &) { return transition<StateSecond>(); }

auto transitionTo(const event2 &) { return none(); }

auto transitionTo(const event3 &) { return handled(); }

auto transitionTo(const event4 &event)
    -> transitions<StateFirst, StateSecond, detail::handled, detail::none> {
   if (event.value_ == 1) {
      return transition<StateFirst>();
   } else if (event.value_ == 2) {
      return transition<StateSecond>();
   } else if (event.value_ == 3) {
      return handled();
   }
   return none(); // does not work yet
                  // return trans<StateSecond>();
}

TEST_CASE("transition_typelist", "[new_fsm]") {

   using tran =
       transitions<StateFirst, StateSecond, detail::handled, detail::none>;

   STATIC_REQUIRE(std::is_same_v<transition_t<0u, tran>, StateFirst>);
   STATIC_REQUIRE(std::is_same_v<transition_t<1u, tran>, StateSecond>);
   STATIC_REQUIRE(std::is_same_v<transition_t<2u, tran>, detail::handled>);
   STATIC_REQUIRE(std::is_same_v<transition_t<3u, tran>, detail::none>);
}

TEST_CASE("transition", "[new_fsm]") {

   auto result = transitionTo(event1{});

   STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<StateSecond>>);
   REQUIRE(result.is_transition());
   REQUIRE(result.idx == 0);

   auto result2 = transitionTo(event3{});
   STATIC_REQUIRE(
       std::is_same_v<decltype(result2), transitions<detail::handled>>);
   // Removed: REQUIRE(result2.is_inner());
   REQUIRE(result2.is_handled());
   REQUIRE(result.idx == 0);

   auto result3 = transitionTo(event4{1});
   STATIC_REQUIRE(std::is_same_v<decltype(result3),
                                 transitions<StateFirst, StateSecond,
                                             detail::handled, detail::none>>);
   // Removed: REQUIRE(result3.is_inner_entry());
   REQUIRE(result3.is_transition());
   REQUIRE(result3.idx == 0);
}

TEST_CASE("none", "[new_fsm]") {

   auto result = transitionTo(event2{});

   STATIC_REQUIRE(std::is_same_v<decltype(result), transitions<detail::none>>);
   REQUIRE(result.is_none());
   REQUIRE_FALSE(result.is_transition());
   CHECK(result.idx == 0); // fails is 1, I dont know why
}

TEST_CASE("handled transition", "[new_fsm]") {
   auto result = handled();
   STATIC_REQUIRE(
       std::is_same_v<decltype(result), transitions<detail::handled>>);
   REQUIRE(result.is_handled());
   REQUIRE_FALSE(result.is_transition());
   REQUIRE_FALSE(result.is_none());
   CHECK(result.idx == 0);

   auto result2 = transitionTo(event3{});
   STATIC_REQUIRE(
       std::is_same_v<decltype(result2), transitions<detail::handled>>);
   REQUIRE(result2.is_handled());
   REQUIRE_FALSE(result2.is_transition());
   REQUIRE_FALSE(result2.is_none());
   CHECK(result2.idx == 0);

   auto result3 = transitionTo(event4{3});
   using tran =
       transitions<StateFirst, StateSecond, detail::handled, detail::none>;
   STATIC_REQUIRE(std::is_same_v<decltype(result3), tran>);
   REQUIRE(result3.is_handled());
   REQUIRE(result3.idx == 2);
}

TEST_CASE("multiple transition path", "[new_fsm]") {

   using tran =
       transitions<StateFirst, StateSecond, detail::handled, detail::none>;

   auto result1 = transitionTo(event4{1});

   STATIC_REQUIRE(std::is_same_v<decltype(result1), tran>);
   REQUIRE(result1.is_transition());
   REQUIRE(result1.idx == 0);

   auto result2 = transitionTo(event4{2});
   STATIC_REQUIRE(std::is_same_v<decltype(result2), tran>);
   // Removed: REQUIRE(result2.is_inner());
   REQUIRE(result2.idx == 1);

   auto result3 = transitionTo(event4{3});
   STATIC_REQUIRE(std::is_same_v<decltype(result3), tran>);
   // Removed: REQUIRE(result3.is_inner_entry());
   REQUIRE(result3.idx == 2);

   auto result4 = transitionTo(event4{4});
   STATIC_REQUIRE(std::is_same_v<decltype(result4), tran>);
   REQUIRE_FALSE(result4.is_transition());

   REQUIRE(result4.is_none());
   REQUIRE(result4.idx == 3);

   // REQUIRE(first.count1 == 1);
}

TEST_CASE("transition_for", "[new_fsm]") {

   auto trans1 = transitionTo(event1{});

   bool result1 = false;

   for_each_transition(trans1, [&](auto i, auto t) {
      INFO("i: " << i << " t: " << t.idx);
      if (i == t.idx) {
         result1 = true;
      }
   });

   REQUIRE(result1);

   result1 = false;
   auto trans2 = transitionTo(event4{1});

   for_each_transition(trans2, [&](auto i, auto t) {
      INFO("i: " << i << " t: " << t.idx);
      if (i == t.idx) {
         INFO("matched");
         result1 = true;
      }
   });

   REQUIRE(result1);

   result1 = false;
   auto trans3 = transitionTo(event4{2});

   for_each_transition(trans3, [&](auto i, auto t) {
      INFO("i: " << i << " t: " << t.idx);
      if (i == t.idx) {
         INFO("matched");
         result1 = true;
      }
   });

   REQUIRE(result1);
}
