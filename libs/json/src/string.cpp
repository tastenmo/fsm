
#include <json/string.h>

#include <iostream>

namespace spie::json::string {

auto Initial::transitionInternalTo() -> transitions<Content, Error> const {
  if (context().consume(stringTokenType::DOUBLE_QUOTE)) {
    context().start();
    return transition<Content>();
  }
  return transition<Error>();
}

auto Content::transitionInternalTo() -> transitions<Content, Finished> const {
  if (context().isToken(stringTokenType::DOUBLE_QUOTE)) {
    return transition<Finished>();
  }

  if (context().consume(stringTokenType::HEX)) {
    context().add();
    return transition<Content>();
  }

  if (context().consume(stringTokenType::CHARS)) {
    context().add();
    return transition<Content>();
  }

  if (context().consume(stringTokenType::ESCAPE)) {
    context().add();
    return transition<Content>();
  }

  //    ctx_.consume();
  //    ctx_.add();
  return transition<Error>();
}

void Finished::onEnter() {
  context().consume(stringTokenType::DOUBLE_QUOTE);
  // ctx_.consume();
}

void Error::onEnter() {
  // Disabled for clarity
}

} // namespace spie::json::string
