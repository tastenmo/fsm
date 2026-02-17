
#include <json/array.h>
#include <json/object.h>
#include <json/value.h>

#include <iostream>

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

  //   if (context().isToken(numberTokenType::SIGN) ||
  //       context().isToken(numberTokenType::DIGIT)) {
  //      return transition<Number>();
  //   }

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
    context().addValue(jsonValue(std::string(nested().context().value())));

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
    context().addValue(jsonValue(nested().context().number));

    return transition<Finished>();
  }

  return transition<Error>();
}

auto Boolean::transitionInternalTo() -> transitions<Finished, Error> const {
  if (context().consume(jsonTokenType::TRUE)) {
    context().addValue(jsonValue(bool(true)));

    return transition<Finished>();

  } else if (context().consume(jsonTokenType::FALSE)) {
    context().addValue(jsonValue(bool(false)));

    return transition<Finished>();
  }

  return transition<Error>();
}

auto Null::transitionInternalTo() -> transitions<Finished, Error> const {

  if (context().consume(jsonTokenType::NULL_)) {
    context().addValue(jsonNull(jsonNull{}));

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
    context().addValue(jsonValue(nested()->context().values()));

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
    context().addValue(jsonValue(nested()->context().values()));

    return transition<Finished>();
  }

  return transition<Error>();
}

void Error::onEnter() {
  // auto &view = context().getView();
  // auto pos = view.pos_;
  // auto size = view.input_.size();
  // auto remaining = size > pos ? size - pos : 0U;
  // auto len = remaining < 32U ? remaining : 32U;
  // auto first = remaining ? static_cast<unsigned char>(view.input_[pos]) : 0U;
  // std::cerr << "[ERROR] value pos=" << pos << " first=" <<
  // static_cast<unsigned int>(first)
  //           << " next=" << view.input_.substr(pos, len) << std::endl;
}
} // namespace spie::json::value