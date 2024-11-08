
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

struct Initial : initial_state<Initial, StateContainer, Context> {

  Initial(Context &ctx) noexcept : initial_state(ctx), ctx_(ctx) {}

  auto transitionTo(start) const { return sibling<String>(); }

  Context &ctx_;
};

struct String : composite_state<String, string::Initial, Context> {

  String(Context &ctx) noexcept
      : composite_state(ctx, string::Context{ctx.view_}), ctx_(ctx) {
    std::cout << "String &ctx_: " << &ctx_ << std::endl;

    std::cout << "String &ctx_.getView(): " << &ctx_.getView() << std::endl;

    std::cout << "String &nested_context(): " << &nested_context() << std::endl;

    std::cout << "String &nested_context().getView(): "
              << &nested_context().getView() << std::endl;
  }

  auto transitionTo(start) -> transitions<detail::none(), Number> const {
    if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested_context().value() << std::endl;

      ctx_.consume(jsonTokenType::WS);

      return sibling<Number>();
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return detail::none();
  }

  Context &ctx_;
};

struct Number : composite_state<Number, number::Initial, Context> {

  Number(Context &ctx) noexcept
      : composite_state(ctx, number::Context{ctx.view_}), ctx_(ctx) {}

  auto transitionTo(start) -> transitions<detail::none(), Number> const {
    if (nested_in<number::Finished>()) {
      std::cout << "number value: " << nested_context().value() << std::endl;

      ctx_.consume(jsonTokenType::WS);

      return sibling<Number>();
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return detail::none();
  }

  Context &ctx_;
};

struct Finished : state<Finished, Context> {

  Finished(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() { std::cout << "Finished" << std::endl; }

  Context &ctx_;
};

TEST_CASE("Json Context reference", "[new_fsm]") {

  Context ctx_{view{"\"simple string.\" 1234"sv}};

  auto fsm = Initial::create(ctx_);

  REQUIRE(&ctx_ == &fsm.context());

  REQUIRE(fsm.is_in<Initial>());
  REQUIRE(&ctx_ == &fsm.state<Initial>().ctx_);
  REQUIRE(&ctx_.getView() == &fsm.state<Initial>().ctx_.getView());

  auto result = fsm.dispatch(start{});
  REQUIRE(result);
  REQUIRE(fsm.is_in<String>());

  REQUIRE(&ctx_ == &fsm.state<String>().ctx_);

  auto ststring = fsm.state<String>();
  auto comp_state = ststring.nested_state<string::Finished>();

  REQUIRE(&ctx_.getView() == &fsm.state<String>().nested_context().getView());
}
