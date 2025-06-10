#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <string_view>
#include <sys/types.h>

#include "json.h"
#include "tokenizer.h"

#include "kvp.h"

#include <new_fsm/composite_state.h>

#include <magic_enum/magic_enum.hpp>

using namespace escad::new_fsm;

namespace escad::json::object {

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

  void addValue(jsonKeyValuePair val) { values_.addValue(val); }

  jsonValue getValue(std::string_view key) {
    return values_.getValue(std::string(key));
  }

  jsonObject values() const { return values_; }

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;

  jsonObject values_;
};

struct Initial;
struct KeyValuePair;
struct Comma;
struct Finished;
struct Error;

using States = states<Initial, KeyValuePair, Comma, Finished, Error>;

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<KeyValuePair, Error> const {
    if (context_.isToken(jsonTokenType::OPEN_BRACE)) {
      context_.start();
      context_.consume(jsonTokenType::OPEN_BRACE);
      context_.consume(jsonTokenType::WS);
      return sibling<KeyValuePair>();
    }

    return sibling<Error>();
  }
};

struct KeyValuePair
    : composite_state<KeyValuePair, kvp::StateContainer, Context> {

  KeyValuePair(Context &ctx) noexcept
      : composite_state(ctx,
                        kvp::StateContainer(mpl::type_identity<kvp::States>{},
                                            kvp::Context(ctx.view_))) {
    nested_emplace<kvp::Initial>();
  }

  auto transitionInternalTo() -> transitions<Comma, Finished, Error> const {
    if (nested_in<kvp::Finished>()) {
      std::cout << "key: " << nested().context().value() << std::endl;
      jsonKeyValuePair val = nested().context().getValue();

      context_.addValue(val);

      context_.consume(jsonTokenType::WS);

      if (context_.isToken(jsonTokenType::COMMA)) {
        return sibling<Comma>();
      };

      if (context_.isToken(jsonTokenType::CLOSE_BRACE)) {
        return sibling<Finished>();
      };
    }

    //   context_.isToken(jsonTokenType::COLON) { return sibling<String>(); }

    return sibling<Error>();
  }
};

struct Comma : state<Comma, Context> {

  auto transitionInternalTo() -> transitions<KeyValuePair, Error> const {

    if (context_.consume(jsonTokenType::COMMA)) {
      context_.consume(jsonTokenType::WS);
      return sibling<KeyValuePair>();
    }

    return sibling<Error>();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() {
    context_.consume(jsonTokenType::CLOSE_BRACE);
    std::cout << "Object -> Finished" << std::endl;
  }
};

struct Error : state<Error, Context> {

  void onEnter() { std::cout << "Error" << std::endl; }
};

} // namespace escad::json::object
