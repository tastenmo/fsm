#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <string_view>
#include <sys/types.h>

#include "json.h"
#include "tokenizer.h"

#include "number.h"
#include "string.h"
#include <new_fsm/composite_state.h>
#include <new_fsm/recursive_state.h>

#include <magic_enum/magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json::object {

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

  std::string_view key() const { return key_; }

  void key(std::string_view key) { key_ = key; }

  void addValue(jsonValue val) { values_.addValue(std::string(key_), val); }

  jsonValue getValue(std::string_view key) {
    return values_.getValue(std::string(key));
  }

  jsonObject values() const { return values_; }

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;

  std::string_view key_;

  jsonObject values_;
};

struct Initial;
struct Key;
struct Colon;
struct String;
struct Number;
struct Boolean;
struct Object;
struct Null;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, Key, Colon, String, Number, Boolean, Object,
                      Null, Comma, Finished, Error>;

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<Key, Error> const {
    if (context_.isToken(jsonTokenType::OPEN_BRACE)) {
      context_.start();
      context_.consume(jsonTokenType::OPEN_BRACE);
      context_.consume(jsonTokenType::WS);
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

      if (context_.consume(jsonTokenType::COLON)) {
        return sibling<Colon>();
      };
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Colon : state<Colon, Context> {

  auto transitionInternalTo()
      -> transitions<String, Number, Boolean, Null, Object, Error> const {

    context_.consume(jsonTokenType::WS);

    if (context_.isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return sibling<String>();
    }

    if (context_.isToken(jsonTokenType::TRUE) ||
        context_.isToken(jsonTokenType::FALSE)) {
      return sibling<Boolean>();
    }

    if (context_.isToken(jsonTokenType::NULL_)) {
      return sibling<Null>();
    }

    if (context_.isToken(jsonTokenType::OPEN_BRACE)) {
      return sibling<Object>();
    }

    return sibling<Number>();
  }
};

struct String : composite_state<String, string::StateContainer, Context> {

  String(Context &ctx) noexcept
      : composite_state(
            ctx, string::StateContainer(mpl::type_identity<string::States>{},
                                        string::Context(ctx.view_))) {
    nested_emplace<string::Initial>();
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested().context().value() << std::endl;

      jsonValue val = std::string(nested().context().value());

      context_.addValue(val);

      context_.consume(jsonTokenType::WS);

      if (context_.consume(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (context_.consume(jsonTokenType::CLOSE_BRACE)) {
        return sibling<Finished>();
      };

      return sibling<Error>();
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Number : composite_state<Number, number::StateContainer, Context> {

  Number(Context &ctx) noexcept
      : composite_state(
            ctx, number::StateContainer(mpl::type_identity<number::States>{},
                                        number::Context(ctx.view_))) {
    nested_emplace<number::Initial>();
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<number::Finished>()) {
      std::cout << "number value found: " << nested().context().value()
                << std::endl;

      jsonValue val = nested().context().number;

      context_.addValue(val);

      context_.consume(jsonTokenType::WS);

      if (context_.consume(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (context_.consume(jsonTokenType::CLOSE_BRACE)) {
        return sibling<Finished>();
      };

      return sibling<Error>();
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Boolean : state<Boolean, Context> {

  void onEnter() {
    if (context_.consume(jsonTokenType::TRUE)) {
      std::cout << "boolean value True found: " << std::endl;
      jsonValue val = bool(true);

      context_.addValue(val);

    } else if (context_.consume(jsonTokenType::FALSE)) {
      std::cout << "boolean value False found: " << std::endl;
      jsonValue val = bool(false);

      context_.addValue(val);
    }
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {

    context_.consume(jsonTokenType::WS);

    if (context_.consume(jsonTokenType::COMMA)) {
      return sibling<Comma>();
    };

    if (context_.consume(jsonTokenType::CLOSE_BRACE)) {
      return sibling<Finished>();
    };

    return sibling<Error>();
  }
};

struct Null : state<Null, Context> {

  void onEnter() {
    if (context_.consume(jsonTokenType::NULL_)) {
      std::cout << "null value found: " << std::endl;

      jsonValue val = std::monostate();

      context_.addValue(val);
    }
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {

    context_.consume(jsonTokenType::WS);

    if (context_.consume(jsonTokenType::COMMA)) {
      return sibling<Comma>();
    };

    if (context_.consume(jsonTokenType::CLOSE_BRACE)) {
      return sibling<Finished>();
    };

    return sibling<Error>();
  }
};

struct Comma : state<Comma, Context> {

  auto transitionInternalTo() -> transitions<Key, Error> const {

    context_.consume(jsonTokenType::WS);

    if (context_.isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return sibling<Key>();
    }

    return sibling<Error>();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() { std::cout << "Finished" << std::endl; }
};

struct Error : state<Error, Context> {

  void onEnter() { std::cout << "Error" << std::endl; }
};

struct Object : recursive_state<Object, StateContainer, Context> {

  Object(Context &ctx) noexcept
      : recursive_state(ctx, StateContainer(mpl::type_identity<States>{},
                                            Context(ctx.view_))) {
    nested_emplace<Initial>();
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<Finished>()) {
      std::cout << "object found: " << nested()->context().value() << std::endl;

      jsonValue val = nested()->context().values();
      context_.addValue(val);

      context_.consume(jsonTokenType::WS);

      if (context_.consume(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (context_.consume(jsonTokenType::CLOSE_BRACE)) {
        return sibling<Finished>();
      };

      return sibling<Error>();
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

} // namespace escad::json::object