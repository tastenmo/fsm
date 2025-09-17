#pragma once

#include <cmath>
#include <cstdint>
#include <sys/types.h>

#include <magic_enum/magic_enum.hpp>

#include "tokenizer.h"
#include <fsm/state_machine.h>

using namespace spie::fsm;

namespace spie::json::number {

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
   JsonNumber() = default;

   JsonNumber(unsigned value) { value_.emplace<1>(value); }
   JsonNumber(int value) { value_.emplace<2>(value); }
   JsonNumber(uint64_t value) { value_.emplace<3>(value); }
   JsonNumber(int64_t value) { value_.emplace<4>(value); }
   JsonNumber(double value) { value_.emplace<5>(value); }

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
         value *= std::pow(10.0,
                           (double)magic_enum::enum_integer(info.exponentSign) *
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
   std::variant<std::monostate, unsigned, int, uint64_t, int64_t, double>
       value_;
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

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionInternalTo() -> transitions<Sign, Integer, Error> const;
};

struct Sign : state<Sign, Machine> {

   using state<Sign, Machine>::state;

   void onEnter();

   auto transitionInternalTo() -> transitions<Integer, Error> const;
};

struct Integer : state<Integer, Machine> {

   using state<Integer, Machine>::state;

   void onEnter();

   auto transitionInternalTo()
       -> transitions<Integer, Decimal, Exponent, Finished> const;
};

struct Decimal : state<Decimal, Machine> {

   using state<Decimal, Machine>::state;

   void onEnter();

   auto
   transitionInternalTo() -> transitions<Decimal, Exponent, Finished> const;
};

struct Exponent : state<Exponent, Machine> {

   using state<Exponent, Machine>::state;

   void onEnter();

   auto transitionInternalTo() -> transitions<Exponent, Finished> const;
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;

   void onEnter();
};

struct Error : state<Error, Machine> {

   void onEnter() { ; }
};

} // namespace spie::json::number