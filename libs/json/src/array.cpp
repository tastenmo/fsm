
#include <json/array.h>

#include <iostream>

namespace spie::json::array {

auto Initial::transitionInternalTo()
    -> transitions<Value, Finished, Error> const {

  if (context().isToken(jsonTokenType::OPEN_BRACKET)) {
    context().start();
    context().consume(jsonTokenType::OPEN_BRACKET);
    context().consume(jsonTokenType::WS);

    if (context().isToken(jsonTokenType::CLOSE_BRACKET)) {
      return transition<Finished>();
    }

    return transition<Value>();
  }

  return transition<Error>();
}

Value::Value(Machine &machine) noexcept
    : composite_state(value::Machine(mpl::type_identity<value::States>{},
                                     value::Context(machine.context().view_)),
                      machine) {
  nested_emplace<value::Initial>();
}

auto Value::transitionInternalTo()
    -> transitions<Comma, Finished, Error> const {
  if (nested_in<value::Finished>()) {
    jsonValue val = nested().context().getValue();

    context().addValue(val);

    context().consume(jsonTokenType::WS);

    if (context().isToken(jsonTokenType::COMMA)) {
      return transition<Comma>();
    };

    if (context().isToken(jsonTokenType::CLOSE_BRACKET)) {
      return transition<Finished>();
    };

    return transition<Error>();
  }

  //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

  return transition<Error>();
}

auto Comma::transitionInternalTo()
    -> transitions<Value, Finished, Error> const {

  if (context().consume(jsonTokenType::COMMA)) {
    context().consume(jsonTokenType::WS);

    if (context().isToken(jsonTokenType::CLOSE_BRACKET)) {
      return transition<Finished>();
    };

    return transition<Value>();
  }

  return transition<Error>();
}

void Finished::onEnter() { context().consume(jsonTokenType::CLOSE_BRACKET); }

void Error::onEnter() {
  // Disabled for final testing
}

} // namespace spie::json::array
