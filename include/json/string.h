#pragma once

#include "../new_fsm/initial_state.h"
#include "tokenizer.h"

using namespace escad::new_fsm;

namespace escad::json::string {

class Context : public stringTokenizer {

public:
  Context(view &input) : stringTokenizer(input) {
    std::cout << "string::Context(view &input)" << std::endl;
    std::cout << "string::Context.getView(): " << &getView() << std::endl;
  }
  Context(view &&input) : stringTokenizer(std::move(input)) {
    std::cout << "string::Context(view &&input)" << std::endl;
    std::cout << "string::Context.getView(): " << &getView() << std::endl;
  }

  ~Context() { std::cout << "string::~Context()" << std::endl; }

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

private:
  std::size_t start_ = 0;
  std::size_t end_ = 0;
};

struct Initial;
struct Content;
struct Finished;
struct Error;

using States = states<Initial, Content, Finished, Error>;

using StateContainer = state_variant<States, Context &>;

struct Initial : initial_state<Initial, StateContainer, Context> {

  Initial(Context &ctx) noexcept : initial_state(ctx), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Content, Error> const {
    if (ctx_.consume(stringTokenType::DOUBLE_QUOTE)) {
      ctx_.start();
      return sibling<Content>();
    }
    return sibling<Error>();
  }

  Context &ctx_;
};

struct Content : state<Content, Context> {

  Content(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  auto transitionInternalTo() -> transitions<Content, Finished> const {
    if (ctx_.isToken(stringTokenType::DOUBLE_QUOTE)) {
      return sibling<Finished>();
    }

    if (ctx_.consume(stringTokenType::HEX)) {
      ctx_.add();
      return sibling<Content>();
    }

    if (ctx_.consume(stringTokenType::CHARS)) {
      ctx_.add();
      return sibling<Content>();
    }

    if (ctx_.consume(stringTokenType::ESCAPE)) {
      ctx_.add();
      return sibling<Content>();
    }

    //    ctx_.consume();
    //    ctx_.add();
    return sibling<Error>();
  }

  Context &ctx_;
};

struct Finished : state<Finished, Context> {

  Finished(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() {
    ctx_.consume(stringTokenType::DOUBLE_QUOTE);
    // ctx_.consume();
  }

  Context &ctx_;
};

struct Error : state<Error, Context> {

  Error(Context &ctx) noexcept : state(ctx), ctx_(ctx) {}

  void onEnter() { ; }

  Context &ctx_;
};

} // namespace escad::json::string
