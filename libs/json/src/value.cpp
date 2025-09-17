
#include <json/array.h>
#include <json/object.h>
#include <json/value.h>

namespace spie::json::value {

auto Initial::transitionInternalTo()
    -> transitions<String, Number, Boolean, Null, Object, Array, Error> const {

   context().consume(jsonTokenType::WS);

   context().start();

   if (context().isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return transition<String>();
   }

   if (context().isToken(jsonTokenType::TRUE) ||
       context().isToken(jsonTokenType::FALSE)) {
      return transition<Boolean>();
   }

   if (context().isToken(jsonTokenType::NULL_)) {
      return transition<Null>();
   }

   if (context().isToken(jsonTokenType::OPEN_BRACE)) {
      return transition<Object>();
   }

   if (context().isToken(jsonTokenType::OPEN_BRACKET)) {
      return transition<Array>();
   }

   return transition<Number>();
}

String::String(Machine &machine) noexcept
    : composite_state(string::Machine(mpl::type_identity<string::States>{},
                                      string::Context(machine.context().view_)),
                      machine) {
   nested_emplace<string::Initial>();
}

auto String::transitionInternalTo() -> transitions<Finished, Error> const {
   if (nested_in<string::Finished>()) {
      std::cout << "string value: " << nested().context().value() << std::endl;

      jsonValue val = std::string(nested().context().value());

      context().addValue(val);

      return transition<Finished>();
   }

   //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

   return transition<Error>();
}

Number::Number(Machine &machine) noexcept
    : composite_state(number::Machine(mpl::type_identity<number::States>{},
                                      number::Context(machine.context().view_)),
                      machine) {
   nested_emplace<number::Initial>();
}

auto Number::transitionInternalTo() -> transitions<Finished, Error> const {
   if (nested_in<number::Finished>()) {
      std::cout << "number value found: " << nested().context().value()
                << std::endl;

      jsonValue val = nested().context().number;

      context().addValue(val);

      return transition<Finished>();
   }

   return transition<Error>();
}

auto Boolean::transitionInternalTo() -> transitions<Finished, Error> const {
   if (context().consume(jsonTokenType::TRUE)) {
      std::cout << "boolean value True found: " << std::endl;
      jsonValue val = bool(true);

      context().addValue(val);

      return transition<Finished>();

   } else if (context().consume(jsonTokenType::FALSE)) {
      std::cout << "boolean value False found: " << std::endl;
      jsonValue val = bool(false);

      context().addValue(val);

      return transition<Finished>();
   }

   return transition<Error>();
}

auto Null::transitionInternalTo() -> transitions<Finished, Error> const {

   if (context().consume(jsonTokenType::NULL_)) {
      std::cout << "null value found: " << std::endl;

      jsonValue val = std::monostate();

      context().addValue(val);

      return transition<Finished>();
   }

   return transition<Error>();
}

Object::Object(Machine &machine) noexcept
    : recursive_state(object::Machine(mpl::type_identity<object::States>{},
                                      object::Context(machine.context().view_)),
                      machine) {
   nested_emplace<object::Initial>();
}

auto Object::transitionInternalTo() -> transitions<Finished, Error> const {
   if (nested_in<object::Finished>()) {
      std::cout << "object found: " << nested()->context().value() << std::endl;

      jsonValue val = nested()->context().values();
      context().addValue(val);

      return transition<Finished>();
   }

   return transition<Error>();
}

Array::Array(Machine &machine) noexcept
    : recursive_state(array::Machine(mpl::type_identity<array::States>{},
                                     array::Context(machine.context().view_)),
                      machine) {
   nested_emplace<array::Initial>();
}

auto Array::transitionInternalTo() -> transitions<Finished, Error> const {
   if (nested_in<array::Finished>()) {
      std::cout << "object found: " << nested()->context().value() << std::endl;

      jsonValue val = nested()->context().values();
      context().addValue(val);

      return transition<Finished>();
   }

   return transition<Error>();
}
} // namespace spie::json::value