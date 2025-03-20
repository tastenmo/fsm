#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <string_view>
#include <sys/types.h>

#include <magic_enum/magic_enum.hpp>

#include "json.h"
#include "tokenizer.h"

#include "value.h"
#include <fsm/composite_state.h>

using namespace escad::new_fsm;

namespace escad::json::array {

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

  void addValue(jsonValue val) { values_.addValue(val); }

  jsonArray getValue() { return values_; }

  jsonArray values() const { return values_; }

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;

  jsonArray values_;
};

struct Initial;
struct Value;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, Value, Comma, Finished, Error>;

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<Value, Finished, Error> const {

    if (context_.isToken(jsonTokenType::OPEN_BRACKET)) {
      context_.start();
      context_.consume(jsonTokenType::OPEN_BRACKET);
      context_.consume(jsonTokenType::WS);

      if (context_.isToken(jsonTokenType::CLOSE_BRACKET)) {
        return sibling<Finished>();
      }

      return sibling<Value>();
    }

    return sibling<Error>();
  }
};

struct Value : composite_state<Value, value::StateContainer, Context> {

  Value(Context &ctx) noexcept
      : composite_state(
            ctx, value::StateContainer(mpl::type_identity<value::States>{},
                                       value::Context(ctx.view_))) {
    nested_emplace<value::Initial>();
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<value::Finished>()) {
      std::cout << "value: " << nested().context().value() << std::endl;

      jsonValue val = nested().context().getValue();

      context_.addValue(val);

      context_.consume(jsonTokenType::WS);

      if (context_.consume(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (context_.consume(jsonTokenType::CLOSE_BRACKET)) {
        return sibling<Finished>();
      };

      return sibling<Error>();
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Comma : state<Comma, Context> {

  auto transitionInternalTo() -> transitions<Value, Finished, Error> const {

    context_.consume(jsonTokenType::WS);

    if (context_.consume(jsonTokenType::CLOSE_BRACKET)) {
      return sibling<Finished>();
    };

    return sibling<Value>();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() { std::cout << "Finished" << std::endl; }
};

struct Error : state<Error, Context> {

  void onEnter() { std::cout << "Error" << std::endl; }
};
} // namespace escad::json::array
