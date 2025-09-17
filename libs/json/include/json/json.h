#pragma once

#include <cmath>

#include <map>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

#include "number.h"
#include "string.h"

#include <magic_enum/magic_enum.hpp>

namespace spie::json {

/**
 * @brief Compile-time container for a set of state types.
 *
 * Usage: `states<StateA, StateB, ...>`
 * Provides a type list and a count of states.
 */
template <class... S> struct valueTypes {
   using type_list = mpl::type_list<S...>;
   static constexpr auto count = type_list::size;
};

class jsonObject;
class jsonArray;

struct jsonError {

   std::string msg;

   unsigned errorCode;

   unsigned position;
};

struct jsonNull {};

using ValueTypes = valueTypes<jsonNull, bool, std::string, number::JsonNumber,
                              jsonObject, jsonArray, jsonError>;

template <class ValueTypes> class jsonValueType {

 public:
   using type_list = typename ValueTypes::type_list;

   using variant_list =
       typename mpl::type_list_push_front<type_list, std::monostate>::result;

   // transform a type list to a corresponding variant
   using type_variant =
       typename mpl::type_list_rename<variant_list, std::variant>::result;

   jsonValueType() : value_() {}
   jsonValueType(jsonNull null) : value_(null) {}
   jsonValueType(bool b) : value_(b) {}
   jsonValueType(std::string str) : value_(str) {}
   jsonValueType(number::JsonNumber number) : value_(number) {}

   jsonValueType(jsonObject obj);
   jsonValueType(jsonArray arr);
   jsonValueType(jsonError err) : value_(err) {}

   // template <typename T>
   // jsonValueType(T &&typ) : value_(std::forward<T>(typ)){};

   template <class ValueType> auto is() const {
      return std::holds_alternative<ValueType>(value_);
   }

   template <class ValueType> std::optional<ValueType> get() {

      if (is<ValueType>()) {

         return std::get<ValueType>(value_);
      }
      return std::nullopt;
   }

 private:
   type_variant value_;
};

using jsonValue = jsonValueType<ValueTypes>;

using jsonKeyValuePair = std::pair<std::string, jsonValue>;

class jsonObject {

 public:
   void addValue(jsonKeyValuePair);

   jsonValue getValue(std::string_view key);

 private:
   std::map<std::string, jsonValue> values_;
};

class jsonArray {
 public:
   void addValue(jsonValue val);
   jsonValue getValue(unsigned index);

 private:
   std::vector<jsonValue> values_;
};

template <class ValueTypes>
jsonValueType<ValueTypes>::jsonValueType(jsonObject obj) : value_(obj) {}

template <class ValueTypes>
jsonValueType<ValueTypes>::jsonValueType(jsonArray arr) : value_(arr) {}

} // namespace spie::json
