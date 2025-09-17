
#include <json/kvp.h>

namespace spie::json::kvp {

auto Initial::transitionInternalTo() -> transitions<Key, Error> const {

   context().consume(jsonTokenType::WS);

   context().start();

   if (context().isToken(jsonTokenType::DOUBLE_QUOTE)) {
      return transition<Key>();
   }

   return transition<Error>();
}

Key::Key(Machine &machine) noexcept
    : composite_state(string::Machine(mpl::type_identity<string::States>{},
                                      string::Context(machine.context().view_)),
                      machine) {
   nested_emplace<string::Initial>();
}

auto Key::transitionInternalTo() -> transitions<Colon, Error> const {
   if (nested_in<string::Finished>()) {
      std::cout << "key: " << nested().context().value() << std::endl;
      context().key(nested().context().value());

      context().consume(jsonTokenType::WS);

      if (context().isToken(jsonTokenType::COLON)) {
         return transition<Colon>();
      };
   }

   //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

   return transition<Error>();
}

auto Colon::transitionInternalTo() -> transitions<Value, Error> const {

   if (context().consume(jsonTokenType::COLON)) {
      return transition<Value>();
   };

   return transition<Error>();
}

Value::Value(Machine &machine) noexcept
    : composite_state(value::Machine(mpl::type_identity<value::States>{},
                                     value::Context(machine.context().view_)),
                      machine) {
   nested_emplace<value::Initial>();
}

auto Value::transitionInternalTo() -> transitions<Finished, Error> const {
   if (nested_in<value::Finished>()) {
      std::cout << "value: " << nested().context().value() << std::endl;

      jsonValue val = nested().context().getValue();

      context().addValue(val);

      context().consume(jsonTokenType::WS);

      return transition<Finished>();
   }

   //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

   return transition<Error>();
}

} // namespace spie::json::kvp