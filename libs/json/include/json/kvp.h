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
#include <fsm/composite_state.h>

#include <magic_enum/magic_enum.hpp>

using namespace spie::fsm;

namespace spie::json::kvp {

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

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionInternalTo() -> transitions<Key, Error> const;
};

struct Key : composite_state<Key, string::Machine, Machine> {

   Key(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Colon, Error> const;
};

struct Colon : state<Colon, Machine> {

   using state<Colon, Machine>::state;

   auto transitionInternalTo() -> transitions<Value, Error> const;
};

struct Value : composite_state<Value, value::Machine, Machine> {

   Value(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;

   void onEnter() { std::cout << "Finished" << std::endl; }
};

struct Error : state<Error, Machine> {

   using state<Error, Machine>::state;

   void onEnter() { std::cout << "Error" << std::endl; }
};

} // namespace spie::json::kvp