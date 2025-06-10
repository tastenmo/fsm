#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <string_view>
#include <sys/types.h>

#include "json.h"
#include "tokenizer.h"

#include "string.h"
#include "value.h"
#include <new_fsm/composite_state.h>

#include <magic_enum/magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json::kvp {

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

  void key(std::string_view key) { key_ = key; }

  void addValue(jsonValue val) {
    value_ = jsonKeyValuePair(std::string(key_), val);
  }

  jsonKeyValuePair getValue() const { return value_; }

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;

  std::string_view key_;

  jsonKeyValuePair value_;
};

struct Initial;
struct Key;
struct Colon;
struct Value;
struct Finished;
struct Error;

using States = states<Initial, Key, Colon, Value, Finished, Error>;

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<Key, Error> const {

    context_.consume(jsonTokenType::WS);

    context_.start();

    if (context_.isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return sibling<Key>();
    }

    return sibling<Error>();
  }
};

struct Key : composite_state<Key, string::StateContainer, Context> {

  Key(Context &ctx) noexcept
      : composite_state(
            ctx, string::StateContainer(mpl::type_identity<string::States>{},
                                        string::Context(ctx.view_))) {
    nested_emplace<string::Initial>();
  }

  auto transitionInternalTo() -> transitions<Colon, Error> const {
    if (nested_in<string::Finished>()) {
      std::cout << "key: " << nested().context().value() << std::endl;
      context_.key(nested().context().value());

      context_.consume(jsonTokenType::WS);

      if (context_.isToken(jsonTokenType::COLON)) {
        return sibling<Colon>();
      };
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Colon : state<Colon, Context> {

  auto transitionInternalTo() -> transitions<Value, Error> const {

    if (context_.consume(jsonTokenType::COLON)) {
      return sibling<Value>();
    };

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

  auto transitionInternalTo() -> transitions<Finished, Error> const {
    if (nested_in<value::Finished>()) {
      std::cout << "value: " << nested().context().value() << std::endl;

      jsonValue val = nested().context().getValue();

      context_.addValue(val);

      context_.consume(jsonTokenType::WS);

      return sibling<Finished>();
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() { std::cout << "Finished" << std::endl; }
};

struct Error : state<Error, Context> {

  void onEnter() { std::cout << "Error" << std::endl; }
};

} // namespace escad::json::kvp