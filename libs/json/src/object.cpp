
#include <json/object.h>

namespace spie::json::object {

auto Initial::transitionInternalTo() -> transitions<KeyValuePair, Error> const {
   if (context().isToken(jsonTokenType::OPEN_BRACE)) {
      context().start();
      context().consume(jsonTokenType::OPEN_BRACE);
      context().consume(jsonTokenType::WS);
      return transition<KeyValuePair>();
   }

   return transition<Error>();
}

KeyValuePair::KeyValuePair(Machine &machine) noexcept
    : composite_state(kvp::Machine(mpl::type_identity<kvp::States>{},
                                   kvp::Context(machine.context().view_)),
                      machine) {
   nested_emplace<kvp::Initial>();
}

auto KeyValuePair::transitionInternalTo()
    -> transitions<Comma, Finished, Error> const {
   if (nested_in<kvp::Finished>()) {
      std::cout << "key: " << nested().context().value() << std::endl;
      jsonKeyValuePair val = nested().context().getValue();

      context().addValue(val);

      context().consume(jsonTokenType::WS);

      if (context().isToken(jsonTokenType::COMMA)) {
         return transition<Comma>();
      };

      if (context().isToken(jsonTokenType::CLOSE_BRACE)) {
         return transition<Finished>();
      };
   }

   //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

   return transition<Error>();
}

auto Comma::transitionInternalTo() -> transitions<KeyValuePair, Error> const {

   if (context().consume(jsonTokenType::COMMA)) {
      context().consume(jsonTokenType::WS);
      return transition<KeyValuePair>();
   }

   return transition<Error>();
}

void Finished::onEnter() {
   context().consume(jsonTokenType::CLOSE_BRACE);
   std::cout << "Object -> Finished" << std::endl;
}

} // namespace spie::json::object
