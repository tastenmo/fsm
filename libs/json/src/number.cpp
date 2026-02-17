#include <json/number.h>

#include <iostream>

namespace spie::json::number {

auto Initial::transitionInternalTo()
    -> transitions<Sign, Integer, Error> const {
  if (context().isToken(numberTokenType::SIGN)) {
    context().start();
    return transition<Sign>();
  }

  if (context().isToken(numberTokenType::DIGIT)) {
    context().start();
    return transition<Integer>();
  }
  return transition<Error>();
}

void Sign::onEnter() {
  if (auto sign = context().consume(numberTokenType::SIGN)) {
    context().add();
    if (sign == "-") {
      context().info.sign = SIGN::MINUS;
    } else {
      context().info.sign = SIGN::PLUS;
    }
  } else {
    // Should never get here!!
    context().info.sign = SIGN::NONE;
  }
}

auto Sign::transitionInternalTo() -> transitions<Integer, Error> const {

  if (context().isToken(numberTokenType::DIGIT)) {
    return transition<Integer>();
  }
  return transition<Error>();
}

void Integer::onEnter() {
  if (auto digit = context().consume(numberTokenType::DIGIT)) {
    context().add();
    if (context().info.integerCount == 0) {
      context().info.integer = *digit->data() - '0';
    } else {
      context().info.integer =
          context().info.integer * 10 + (*digit->data() - '0');
    }
    context().info.integerCount += 1;
  } else {
    // Should never get here!!
    context().info.integer = 0;
    context().info.integerCount = 0;
  }
}

auto Integer::transitionInternalTo()
    -> transitions<Integer, Decimal, Exponent, Finished> const {

  if (context().isToken(numberTokenType::DIGIT)) {
    return transition<Integer>();
  }

  if (context().consume(numberTokenType::DOT)) {
    context().add();
    return transition<Decimal>();
  }

  return transition<Finished>();
}

void Decimal::onEnter() {
  if (auto digit = context().consume(numberTokenType::DIGIT)) {
    context().add();
    if (context().info.decimalCount == 0) {
      context().info.decimal = *digit->data() - '0';
    } else {
      context().info.decimal =
          context().info.decimal * 10 + (*digit->data() - '0');
    }
    context().info.decimalCount += 1;
  } else {
    // 0. will land here
    context().info.decimal = 0;
    context().info.decimalCount = 1;
  }
}

auto Decimal::transitionInternalTo()
    -> transitions<Decimal, Exponent, Finished> const {

  if (context().isToken(numberTokenType::DIGIT)) {
    return transition<Decimal>();
  }

  if (context().consume(numberTokenType::EXP)) {
    context().add();
    return transition<Exponent>();
  }

  return transition<Finished>();
}

void Exponent::onEnter() {

  if (auto digit = context().consume(numberTokenType::DIGIT)) {
    context().add();
    if (context().info.exponentCount == 0) {
      context().info.exponent = *digit->data() - '0';
    } else {
      context().info.exponent =
          context().info.exponent * 10 + (*digit->data() - '0');
    }
    context().info.exponentCount += 1;
  }

  else if (auto sign = context().consume(numberTokenType::SIGN)) {
    context().add();
    if (sign == "-") {
      context().info.exponentSign = SIGN::MINUS;
    } else {
      context().info.exponentSign = SIGN::PLUS;
    }
  }
}

auto Exponent::transitionInternalTo() -> transitions<Exponent, Finished> const {

  if (context().isToken(numberTokenType::DIGIT)) {
    return transition<Exponent>();
  }

  return transition<Finished>();
}

void Finished::onEnter() { context().number.construct(context().info); }

void Error::onEnter() {
  // Disabled for clarity
}

} // namespace spie::json::number
