
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <fsm/composite_state.h>
#include <json/number.h>
#include <json/string.h>
#include <json/tokenizer.h>

using namespace spie::json;

using namespace std::literals;

class Context : public jsonTokenizer {

 public:
   Context(view &input) : jsonTokenizer(input) {}
   Context(view &&input) : jsonTokenizer(input) {}

   std::string_view value() const {
      return getView().substr(start_, end_ - start_);
   }

   /**
    * @brief Get the size of the string in bytes
    *
    */
   std::size_t size() const { return end_ - start_; }

   std::size_t start() {
      start_ = end_ = getView().pos_;
      return start_;
   }

   std::size_t add() {
      end_ = getView().pos_;
      return end_ - start_;
   }

 private:
   std::size_t start_ = 0;
   std::size_t end_ = 0;
};

struct start {};

struct Initial;
struct String;
struct Number;
struct Finished;

using States = states<Initial, String, Number, Finished>;

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionTo(start) const { return transition<String>(); }
};

struct String : composite_state<String, string::Machine, Machine> {

   String(Machine &machine) noexcept;

   auto transitionTo(start) -> transitions<detail::none(), Number> const;
};

struct Number : composite_state<Number, number::Machine, Machine> {

   Number(Machine &machine) noexcept;

   auto transitionTo(start) -> transitions<detail::none(), Number> const;
};

struct Finished : state<Finished, Machine> {

   void onEnter() { std::cout << "Finished" << std::endl; }
};

String::String(Machine &machine) noexcept
    : composite_state(string::Machine(mpl::type_identity<string::States>{},
                                      string::Context(machine.context().view_)),
                      machine) {
   nested_emplace<string::Initial>();
}

auto String::transitionTo(start) -> transitions<detail::none(), Number> const {
   if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested().context().value() << std::endl;

      context().consume(jsonTokenType::WS);

      return transition<Number>();
   }

   //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

   return detail::none();
}

Number::Number(Machine &machine) noexcept
    : composite_state(number::Machine(mpl::type_identity<number::States>{},
                                      number::Context(machine.context().view_)),
                      machine) {
   nested_emplace<number::Initial>();
}

auto Number::transitionTo(start) -> transitions<detail::none(), Number> const {
   if (nested_in<number::Finished>()) {
      std::cout << "number value: " << nested().context().value() << std::endl;

      context().consume(jsonTokenType::WS);

      return transition<Number>();
   }

   //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

   return detail::none();
}

TEST_CASE("Context reference", "[new_fsm]") {

   Context ctx_{view{"\"simple string.\" 1234"sv}};

   auto fsm = StateMachine(mpl::type_identity<States>{}, ctx_);

   fsm.emplace<Initial>();

   REQUIRE(&ctx_ == &fsm.context());

   REQUIRE(fsm.is_in<Initial>());
   //  REQUIRE(&ctx_ == &fsm.state<Initial>().ctx_);
   REQUIRE(&ctx_.getView() == &fsm.state<Initial>().context().getView());

   auto result = fsm.dispatch(start{});
   REQUIRE(result);
   REQUIRE(fsm.is_in<String>());

   auto &ststring = fsm.state<String>();
   auto comp_state = ststring.nested_state<string::Finished>();

   REQUIRE(ststring.nested().context().value() == "simple string."sv);
   REQUIRE(comp_state.context().value() == "simple string."sv);

   REQUIRE(&ctx_.getView() ==
           &fsm.state<String>().nested().context().getView());

   REQUIRE(ctx_.getView().pos_ == 16);

   result = fsm.dispatch(start{});
   REQUIRE(result);

   REQUIRE(fsm.is_in<Number>());
}
