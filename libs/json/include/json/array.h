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

using namespace spie::fsm;

namespace spie::json::array {

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

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionInternalTo() -> transitions<Value, Finished, Error> const;
};

struct Value : composite_state<Value, value::Machine, Machine> {

   Value(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Comma, Finished, Error> const;
};

struct Comma : state<Comma, Machine> {

   using state<Comma, Machine>::state;

   auto transitionInternalTo() -> transitions<Value, Finished, Error> const;
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;

   void onEnter() { std::cout << "Finished" << std::endl; }
};

struct Error : state<Error, Machine> {

   using state<Error, Machine>::state;

   void onEnter() { std::cout << "Error" << std::endl; }
};
} // namespace spie::json::array
