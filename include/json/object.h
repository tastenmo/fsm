#pragma once

#include "number.h"
#include "tokenizer.h"
#include <cmath>
#include <cstdint>
#include <map>
#include <sys/types.h>

#include "number.h"
#include "string.h"
#include <new_fsm/composite_state.h>
#include <new_fsm/initial_state.h>

#include <magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json::object {

class jsonObject;

using jsonValue = std::variant<std::monostate, bool, std::string,
                               number::JsonNumber, jsonObject>;

class jsonObject {

private:
  std::map<std::string, jsonValue> values_;
};

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

  std::map<std::string, jsonValue> values_;
};

struct Initial;
struct Key;
struct Colon;
struct String;
struct Number;
struct Boolean;
struct Null;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, Key, Colon, String, Number, Boolean, Null, Comma,
                      Finished, Error>;

using StateContainer = StateMachine<States, Context &>;

struct Initial : initial_state<Initial, StateContainer, Context> {

  Initial(Context &ctx) noexcept : initial_state(ctx), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Key, Error> const {
    if (ctx_.isToken(jsonTokenType::OPEN_BRACE)) {
      ctx_.start();
      ctx_.consume(jsonTokenType::OPEN_BRACE);
      return sibling<Key>();
    }

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Key : composite_state<Key, string::Initial, Context> {

  Key(Context &ctx) noexcept
      : composite_state(ctx, string::Context(ctx.view_)), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Colon, Error> const {
    if (nested_in<string::Finished>()) {
      std::cout << "key: " << nested_state<string::Finished>().ctx_.value()
                << std::endl;

      ctx_.consume(jsonTokenType::WS);

      if (ctx_.consume(jsonTokenType::COLON)) {
        return sibling<Colon>();
      };
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Colon : state<Colon, Context> {

  Colon(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  auto transitionInternalTo()
      -> transitions<String, Number, Boolean, Null, Error> const {

    ctx_.consume(jsonTokenType::WS);

    if (ctx_.isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return sibling<String>();
    }

    if (ctx_.isToken(jsonTokenType::TRUE) ||
        ctx_.isToken(jsonTokenType::FALSE)) {
      return sibling<Boolean>();
    }

    if (ctx_.isToken(jsonTokenType::NULL_)) {
      return sibling<Null>();
    }

    return sibling<Number>();
  }

  Context &ctx_;
};

struct String : composite_state<String, string::Initial, Context> {

  String(Context &ctx) noexcept
      : composite_state(ctx, string::Context(ctx.view_)), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested_context().value() << std::endl;

      ctx_.consume(jsonTokenType::WS);

      if (ctx_.consume(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (ctx_.consume(jsonTokenType::CLOSE_BRACE)) {
        return sibling<Finished>();
      };

      return sibling<Error>();
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Number : composite_state<Number, number::Initial, Context> {

  Number(Context &ctx) noexcept
      : composite_state(ctx, number::Context(ctx.view_)), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<number::Finished>()) {
      std::cout << "number value found: " << nested_context().value()
                << std::endl;

      ctx_.consume(jsonTokenType::WS);

      if (ctx_.consume(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (ctx_.consume(jsonTokenType::CLOSE_BRACE)) {
        return sibling<Finished>();
      };

      return sibling<Error>();
    }

    //   ctx_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Boolean : state<Boolean, Context> {

  Boolean(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {
    if (ctx_.consume(jsonTokenType::TRUE)) {
      std::cout << "boolean value True found: " << std::endl;
    } else if (ctx_.consume(jsonTokenType::FALSE)) {
      std::cout << "boolean value False found: " << std::endl;
    }
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {

    ctx_.consume(jsonTokenType::WS);

    if (ctx_.consume(jsonTokenType::COMMA)) {
      return sibling<Comma>();
    };

    if (ctx_.consume(jsonTokenType::CLOSE_BRACE)) {
      return sibling<Finished>();
    };

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Null : state<Null, Context> {

  Null(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {
    if (ctx_.consume(jsonTokenType::NULL_)) {
      std::cout << "null value found: " << std::endl;
    }
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {

    ctx_.consume(jsonTokenType::WS);

    if (ctx_.consume(jsonTokenType::COMMA)) {
      return sibling<Comma>();
    };

    if (ctx_.consume(jsonTokenType::CLOSE_BRACE)) {
      return sibling<Finished>();
    };

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Comma : state<Comma, Context> {

  Comma(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Key, Error> const {

    ctx_.consume(jsonTokenType::WS);

    if (ctx_.isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return sibling<Key>();
    }

    return sibling<Error>();
  }

  Context &ctx_;
};

struct Finished : state<Finished, Context> {

  Finished(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() { std::cout << "Finished" << std::endl; }

  Context &ctx_;
};

struct Error : state<Error, Context> {

  Error(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() { std::cout << "Error" << std::endl; }

  Context &ctx_;
};

} // namespace escad::json::object