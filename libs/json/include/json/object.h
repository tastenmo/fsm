#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <string_view>
#include <sys/types.h>

#include "json.h"
#include "tokenizer.h"

#include "kvp.h"

#include <fsm/composite_state.h>

#include <magic_enum/magic_enum.hpp>

using namespace spie::fsm;

namespace spie::json::object {

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

   void addValue(jsonKeyValuePair val) { values_.addValue(val); }

   jsonValue getValue(std::string_view key) {
      return values_.getValue(std::string(key));
   }

   jsonObject values() const { return values_; }

 private:
   std::size_t start_ = 0;
   std::size_t end_ = 0;

   jsonObject values_;
};

struct Initial;
struct KeyValuePair;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, KeyValuePair, Comma, Finished, Error>;

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionInternalTo() -> transitions<KeyValuePair, Error> const;
};

struct KeyValuePair : composite_state<KeyValuePair, kvp::Machine, Machine> {

   KeyValuePair(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Comma, Finished, Error> const;
};

struct Comma : state<Comma, Machine> {

   auto transitionInternalTo() -> transitions<KeyValuePair, Error> const;
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;

   void onEnter();
};

struct Error : state<Error, Machine> {

   using state<Error, Machine>::state;

   void onEnter() { std::cout << "Error" << std::endl; }
};

} // namespace spie::json::object
