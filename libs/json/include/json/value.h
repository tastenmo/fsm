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

using namespace spie::fsm;

namespace spie::json::object {

class Context;

struct Initial;
struct KeyValuePair;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, KeyValuePair, Comma, Finished, Error>;

using Machine = StateMachine<States, Context>;

} // namespace spie::json::object

namespace spie::json::array {

class Context;
struct Initial;
struct Value;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, Value, Comma, Finished, Error>;

using Machine = StateMachine<States, Context>;

} // namespace spie::json::array

namespace spie::json::value {

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

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionInternalTo() -> transitions<String, Number, Boolean, Null,
                                              Object, Array, Error> const;
};

struct String : composite_state<String, string::Machine, Machine> {

   String(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Number : composite_state<Number, number::Machine, Machine> {

   Number(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Boolean : state<Boolean, Machine> {

   using state<Boolean, Machine>::state;

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Null : state<Null, Machine> {

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Finished : state<Finished, Machine> {

   using state<Finished, Machine>::state;
   void onEnter() {}
};

struct Error : state<Error, Machine> {

   using state<Error, Machine>::state;
   void onEnter();
};

struct Object : recursive_state<Object, object::Machine, Machine> {

   Object(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

struct Array : recursive_state<Array, array::Machine, Machine> {

   Array(Machine &machine) noexcept;

   auto transitionInternalTo() -> transitions<Finished, Error> const;
};

} // namespace spie::json::value