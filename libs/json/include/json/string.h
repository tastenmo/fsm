#pragma once

#include "tokenizer.h"
#include <fsm/state_machine.h>


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

using StateContainer = StateMachine<States, Context>;

struct Initial : state<Initial, Context> {

  auto transitionInternalTo() -> transitions<Content, Error> const {
    if (context_.consume(stringTokenType::DOUBLE_QUOTE)) {
      context_.start();
      return sibling<Content>();
    }
    return sibling<Error>();
  }
};

struct Content : state<Content, Context> {

  auto transitionInternalTo() -> transitions<Content, Finished> const {
    if (context_.isToken(stringTokenType::DOUBLE_QUOTE)) {
      return sibling<Finished>();
    }

    if (context_.consume(stringTokenType::HEX)) {
      context_.add();
      return sibling<Content>();
    }

    if (context_.consume(stringTokenType::CHARS)) {
      context_.add();
      return sibling<Content>();
    }

    if (context_.consume(stringTokenType::ESCAPE)) {
      context_.add();
      return sibling<Content>();
    }

    //    ctx_.consume();
    //    ctx_.add();
    return sibling<Error>();
  }
};

struct Finished : state<Finished, Context> {

  void onEnter() {
    context_.consume(stringTokenType::DOUBLE_QUOTE);
    // ctx_.consume();
  }
};

struct Error : state<Error, Context> {

  void onEnter() { ; }
};

} // namespace escad::json::string
