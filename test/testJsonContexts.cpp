
#include <chrono>
#include <functional>
#include <iostream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <json/number.h>
#include <json/string.h>
#include <json/tokenizer.h>
#include <new_fsm/composite_state.h>

using namespace escad::json;

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

using StateContainer = StateMachine<States, Context &>;

struct Initial : state<Initial, Context> {

  auto transitionTo(start) const { return sibling<String>(); }
};

struct String : composite_state<String, string::StateContainer, Context> {

  String(Context &ctx) noexcept
      : composite_state(
            ctx, string::StateContainer(mpl::type_identity<string::States>{},
                                        string::Context(ctx.view_))) {
    nested_emplace<string::Initial>();
  }

  auto transitionTo(start) -> transitions<detail::none(), Number> const {
    if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested().context().value() << std::endl;

      context_.consume(jsonTokenType::WS);

      return sibling<Number>();
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return detail::none();
  }
};

struct Number : composite_state<Number, number::StateContainer, Context> {

  Number(Context &ctx) noexcept
      : composite_state(
            ctx, number::StateContainer(mpl::type_identity<number::States>{},
                                        number::Context(ctx.view_))) {
    nested_emplace<number::Initial>();
  }

  auto transitionTo(start) -> transitions<detail::none(), Number> const {
    if (nested_in<number::Finished>()) {
      std::cout << "number value: " << nested().context().value() << std::endl;

      context_.consume(jsonTokenType::WS);

      return sibling<Number>();
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return detail::none();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() { std::cout << "Finished" << std::endl; }
};

TEST_CASE("Json Context reference", "[new_fsm]") {

  Context ctx_{view{"\"simple string.\" 1234"sv}};

  auto fsm = StateMachine(mpl::type_identity<States>{}, ctx_);

  REQUIRE(&ctx_ == &fsm.context());

  REQUIRE(fsm.is_in<Initial>());
  //  REQUIRE(&ctx_ == &fsm.state<Initial>().ctx_);
  REQUIRE(&ctx_.getView() == &fsm.state<Initial>().context().getView());

  auto result = fsm.dispatch(start{});
  REQUIRE(result);
  REQUIRE(fsm.is_in<String>());

  auto ststring = fsm.state<String>();
  auto comp_state = ststring.nested_state<string::Finished>();

  REQUIRE(ststring.nested().context().value() == "simple string."sv);
  REQUIRE(comp_state.context().value() == "simple string."sv);

  REQUIRE(&ctx_.getView() == &fsm.state<String>().nested().context().getView());

  REQUIRE(ctx_.getView().pos_ == 15);

  result = fsm.dispatch(start{});
  REQUIRE(result);

  REQUIRE(fsm.is_in<Number>());

  
}
