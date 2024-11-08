#pragma once

#include <cmath>
#include <cstdint>
#include <sys/types.h>

#include <magic_enum.hpp>

#include "../new_fsm/state_machine.h"
#include "tokenizer.h"

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

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<Sign, Integer, Error> const {
    if (context_.isToken(numberTokenType::SIGN)) {
      context_.start();
      return sibling<Sign>();
    }

    if (context_.isToken(numberTokenType::DIGIT)) {
      context_.start();
      return sibling<Integer>();
    }
    return sibling<Error>();
  }
};

struct Sign : state<Sign, Context> {

  void onEnter() {
    if (auto sign = context_.consume(numberTokenType::SIGN)) {
      context_.add();
      if (sign == "-") {
        context_.info.sign = SIGN::MINUS;
      } else {
        context_.info.sign = SIGN::PLUS;
      }
    } else {
      // Should never get here!!
      context_.info.sign = SIGN::NONE;
    }
  }

  auto transitionInternalTo() -> transitions<Integer, Error> const {

    if (context_.isToken(numberTokenType::DIGIT)) {
      return sibling<Integer>();
    }
    return sibling<Error>();
  }
};

struct Integer : state<Integer, Context> {

  void onEnter() {
    if (auto digit = context_.consume(numberTokenType::DIGIT)) {
      context_.add();
      if (context_.info.integerCount == 0) {
        context_.info.integer = *digit->data() - '0';
      } else {
        context_.info.integer =
            context_.info.integer * 10 + (*digit->data() - '0');
      }
      context_.info.integerCount += 1;
    } else {
      // Should never get here!!
      context_.info.integer = 0;
      context_.info.integerCount = 0;
    }
  }

  auto transitionInternalTo()
      -> transitions<Integer, Decimal, Exponent, Finished> const {

    if (context_.isToken(numberTokenType::DIGIT)) {
      return sibling<Integer>();
    }

    if (context_.consume(numberTokenType::DOT)) {
      context_.add();
      return sibling<Decimal>();
    }

    return sibling<Finished>();
  }
};

struct Decimal : state<Decimal, Context> {

  void onEnter() {
    if (auto digit = context_.consume(numberTokenType::DIGIT)) {
      context_.add();
      if (context_.info.decimalCount == 0) {
        context_.info.decimal = *digit->data() - '0';
      } else {
        context_.info.decimal =
            context_.info.decimal * 10 + (*digit->data() - '0');
      }
      context_.info.decimalCount += 1;
    } else {
      // 0. will land here
      context_.info.decimal = 0;
      context_.info.decimalCount = 1;
    }
  }

  auto
  transitionInternalTo() -> transitions<Decimal, Exponent, Finished> const {

    if (context_.isToken(numberTokenType::DIGIT)) {
      return sibling<Decimal>();
    }

    if (context_.consume(numberTokenType::EXP)) {
      context_.add();
      return sibling<Exponent>();
    }

    return sibling<Finished>();
  }
};

struct Exponent : state<Exponent, Context> {

  void onEnter() {

    if (auto digit = context_.consume(numberTokenType::DIGIT)) {
      context_.add();
      if (context_.info.exponentCount == 0) {
        context_.info.exponent = *digit->data() - '0';
      } else {
        context_.info.exponent =
            context_.info.exponent * 10 + (*digit->data() - '0');
      }
      context_.info.exponentCount += 1;
    }

    else if (auto sign = context_.consume(numberTokenType::SIGN)) {
      context_.add();
      if (sign == "-") {
        context_.info.exponentSign = SIGN::MINUS;
      } else {
        context_.info.exponentSign = SIGN::PLUS;
      }
    }
  }

  auto transitionInternalTo() -> transitions<Exponent, Finished> const {

    if (context_.isToken(numberTokenType::DIGIT)) {
      return sibling<Exponent>();
    }

    return sibling<Finished>();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() { context_.number.construct(context_.info); }
};

struct Error : state<Error, Context> {

  void onEnter() { ; }
};

} // namespace escad::json::number