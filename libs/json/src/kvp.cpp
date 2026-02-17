
#include <json/kvp.h>

#include <iostream>

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
    jsonValue val = nested().context().getValue();

    context().addValue(val);

    context().consume(jsonTokenType::WS);

    return transition<Finished>();
  }

  //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

  return transition<Error>();
}

void Error::onEnter() {
  // auto &view = context().getView();
  // auto pos = view.pos_;
  // auto size = view.input_.size();
  // auto remaining = size > pos ? size - pos : 0U;
  // auto len = remaining < 32U ? remaining : 32U;
  // auto first = remaining ? static_cast<unsigned char>(view.input_[pos]) : 0U;
  // std::cerr << "[ERROR] kvp pos=" << pos << " first=" << static_cast<unsigned
  // int>(first)
  //           << " next=" << view.input_.substr(pos, len) << std::endl;
}

} // namespace spie::json::kvp