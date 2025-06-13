#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <string_view>
#include <sys/types.h>

#include "json.h"
#include "tokenizer.h"

// #include "array.h"
#include "number.h"
// #include "object.h"
#include "string.h"

#include <fsm/composite_state.h>
#include <fsm/recursive_state.h>

#include <magic_enum/magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json::object {

class Context;

struct Initial;
struct KeyValuePair;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, KeyValuePair, Comma, Finished, Error>;

using StateContainer = StateMachine<States, Context>;

} // namespace escad::json::object

namespace escad::json::array {

class Context;
struct Initial;
struct Value;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, Value, Comma, Finished, Error>;

using StateContainer = StateMachine<States, Context>;

} // namespace escad::json::array

namespace escad::json::value {

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

  void addValue(jsonValue val) { value_ = val; }

  jsonValue getValue() const { return value_; }

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;

  jsonValue value_;
};

struct Initial;
struct String;
struct Number;
struct Boolean;
struct Object;
struct Array;
struct Null;
struct Finished;
struct Error;

using States = states<Initial, String, Number, Boolean, Object, Array, Null,
                      Finished, Error>;

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<String, Number, Boolean, Null,
                                             Object, Array, Error> const {

    context_.consume(jsonTokenType::WS);

    context_.start();

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

    if (context_.isToken(jsonTokenType::OPEN_BRACKET)) {
      return sibling<Array>();
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

  auto transitionInternalTo() -> transitions<Finished, Error> const {
    if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested().context().value() << std::endl;

      jsonValue val = std::string(nested().context().value());

      context_.addValue(val);

      return sibling<Finished>();
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

  auto transitionInternalTo() -> transitions<Finished, Error> const {
    if (nested_in<number::Finished>()) {
      std::cout << "number value found: " << nested().context().value()
                << std::endl;

      jsonValue val = nested().context().number;

      context_.addValue(val);

      return sibling<Finished>();
    }

    return sibling<Error>();
  }
};

struct Boolean : state<Boolean, Context> {

  auto transitionInternalTo() -> transitions<Finished, Error> const {
    if (context_.consume(jsonTokenType::TRUE)) {
      std::cout << "boolean value True found: " << std::endl;
      jsonValue val = bool(true);

      context_.addValue(val);

      return sibling<Finished>();

    } else if (context_.consume(jsonTokenType::FALSE)) {
      std::cout << "boolean value False found: " << std::endl;
      jsonValue val = bool(false);

      context_.addValue(val);

      return sibling<Finished>();
    }

    return sibling<Error>();
  }
};

struct Null : state<Null, Context> {

  auto transitionInternalTo() -> transitions<Finished, Error> const {

    if (context_.consume(jsonTokenType::NULL_)) {
      std::cout << "null value found: " << std::endl;

      jsonValue val = std::monostate();

      context_.addValue(val);

      return sibling<Finished>();
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

struct Object : recursive_state<Object, object::StateContainer, Context> {

  Object(Context &ctx) noexcept;

  auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Array : recursive_state<Array, array::StateContainer, Context> {

  Array(Context &ctx) noexcept;

  auto transitionInternalTo() -> transitions<Finished, Error> const;
};

} // namespace escad::json::value