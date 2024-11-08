#pragma once

#include "../new_fsm/initial_state.h"
#include "tokenizer.h"
#include <cmath>
#include <cstdint>
#include <sys/types.h>

#include <magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json::number {

enum class SIGN { NONE = 0, PLUS = 1, MINUS = -1 };

struct NumberInfo {
  SIGN sign = SIGN::NONE;
  uint64_t integer = 0;
  unsigned integerCount = 0;
  uint64_t decimal = 0;
  unsigned decimalCount = 0;
  SIGN exponentSign = SIGN::NONE;
  unsigned exponent = 0;
  unsigned exponentCount = 0;
};

class JsonNumber {

public:
  void construct(NumberInfo &info) {
    if (info.decimalCount == 0) {
      constructInteger(info);
      return;
    }

    double value = (double)info.integer;
    if (info.decimalCount > 0) {
      value += info.decimal / std::pow(10.0, info.decimalCount);
    }
    if (info.sign == SIGN::MINUS) {
      value = -value;
    }

    if (info.exponentSign == SIGN::NONE) {
      info.exponentSign = SIGN::PLUS;
    }

    if (info.exponentCount > 0) {
      value *=
          std::pow(10.0, (double)magic_enum::enum_integer(info.exponentSign) *
                             (double)info.exponent);
    }
    value_ = value;
  }

  void constructInteger(NumberInfo &info) {
    if (info.sign == SIGN::NONE) {
      if (info.integer > std::numeric_limits<unsigned>::max()) {
        value_.emplace<3>(info.integer);
      } else {
        value_.emplace<1>(static_cast<unsigned>(info.integer));
      }
    } else {
      int64_t value = magic_enum::enum_integer(info.sign) * info.integer;
      if (value > std::numeric_limits<int>::max() ||
          value < std::numeric_limits<int>::min()) {
        value_.emplace<4>(value);
      } else {
        value_.emplace<2>(static_cast<int>(value));
      }
    }
  }
  template <typename T> std::optional<T> get() const {
    if (std::holds_alternative<T>(value_)) {
      return std::get<T>(value_);
    }
    return std::nullopt;
  }

private:
  std::variant<std::monostate, unsigned, int, uint64_t, int64_t, double> value_;
};

class Context : public numberTokenizer {

public:
  Context(view &input) : numberTokenizer(input) {}
  Context(view &&input) : numberTokenizer(input) {}

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
    ;
    return end_ - start_;
  }

  NumberInfo info;

  JsonNumber number;

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;
};

struct Initial;
struct Sign;
struct Integer;
struct Decimal;
struct Exponent;
struct Finished;
struct Error;

using States =
    states<Initial, Sign, Integer, Decimal, Exponent, Finished, Error>;

using StateContainer = StateMachine<States, Context &>;

struct Initial : initial_state<Initial, StateContainer, Context> {

  Initial(Context &ctx) noexcept : initial_state(ctx), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Sign, Integer, Error> const {
    if (ctx_.isToken(numberTokenType::SIGN)) {
      ctx_.start();
      return sibling<Sign>();
    }

    if (ctx_.isToken(numberTokenType::DIGIT)) {
      ctx_.start();
      return sibling<Integer>();
    }
    return sibling<Error>();
  }

  Context &ctx_;
};

struct Sign : state<Sign, Context> {

  Sign(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {
    if (auto sign = ctx_.consume(numberTokenType::SIGN)) {
      ctx_.add();
      if (sign == "-") {
        ctx_.info.sign = SIGN::MINUS;
      } else {
        ctx_.info.sign = SIGN::PLUS;
      }
    } else {
      // Should never get here!!
      ctx_.info.sign = SIGN::NONE;
    }
  }

  auto transitionInternalTo() -> transitions<Integer, Error> const {

    if (ctx_.isToken(numberTokenType::DIGIT)) {
      return sibling<Integer>();
    }
    return sibling<Error>();
  }

  Context &ctx_;
};

struct Integer : state<Integer, Context> {

  Integer(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {
    if (auto digit = ctx_.consume(numberTokenType::DIGIT)) {
      ctx_.add();
      if (ctx_.info.integerCount == 0) {
        ctx_.info.integer = *digit->data() - '0';
      } else {
        ctx_.info.integer = ctx_.info.integer * 10 + (*digit->data() - '0');
      }
      ctx_.info.integerCount += 1;
    } else {
      // Should never get here!!
      ctx_.info.integer = 0;
      ctx_.info.integerCount = 0;
    }
  }

  auto transitionInternalTo()
      -> transitions<Integer, Decimal, Exponent, Finished> const {

    if (ctx_.isToken(numberTokenType::DIGIT)) {
      return sibling<Integer>();
    }

    if (ctx_.consume(numberTokenType::DOT)) {
      ctx_.add();
      return sibling<Decimal>();
    }

    return sibling<Finished>();
  }

  Context &ctx_;
};

struct Decimal : state<Decimal, Context> {

  Decimal(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {
    if (auto digit = ctx_.consume(numberTokenType::DIGIT)) {
      ctx_.add();
      if (ctx_.info.decimalCount == 0) {
        ctx_.info.decimal = *digit->data() - '0';
      } else {
        ctx_.info.decimal = ctx_.info.decimal * 10 + (*digit->data() - '0');
      }
      ctx_.info.decimalCount += 1;
    } else {
      // 0. will land here
      ctx_.info.decimal = 0;
      ctx_.info.decimalCount = 1;
    }
  }

  auto
  transitionInternalTo() -> transitions<Decimal, Exponent, Finished> const {

    if (ctx_.isToken(numberTokenType::DIGIT)) {
      return sibling<Decimal>();
    }

    if (ctx_.consume(numberTokenType::EXP)) {
      ctx_.add();
      return sibling<Exponent>();
    }

    return sibling<Finished>();
  }

  Context &ctx_;
};

struct Exponent : state<Exponent, Context> {

  Exponent(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {

    if (auto digit = ctx_.consume(numberTokenType::DIGIT)) {
      ctx_.add();
      if (ctx_.info.exponentCount == 0) {
        ctx_.info.exponent = *digit->data() - '0';
      } else {
        ctx_.info.exponent = ctx_.info.exponent * 10 + (*digit->data() - '0');
      }
      ctx_.info.exponentCount += 1;
    }

    else if (auto sign = ctx_.consume(numberTokenType::SIGN)) {
      ctx_.add();
      if (sign == "-") {
        ctx_.info.exponentSign = SIGN::MINUS;
      } else {
        ctx_.info.exponentSign = SIGN::PLUS;
      }
    }
  }

  auto transitionInternalTo() -> transitions<Exponent, Finished> const {

    if (ctx_.isToken(numberTokenType::DIGIT)) {
      return sibling<Exponent>();
    }

    return sibling<Finished>();
  }

  Context &ctx_;
};

struct Finished : state<Finished, Context> {

  Finished(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() { ctx_.number.construct(ctx_.info); }

  Context &ctx_;
};

struct Error : state<Error, Context> {

  Error(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() { ; }

  Context &ctx_;
};

} // namespace escad::json::number