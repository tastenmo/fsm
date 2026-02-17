#pragma once

#include "tokenizer.h"
#include <fsm/state_machine.h>

using namespace spie::fsm;

namespace spie::json::string {

class Context : public stringTokenizer {

 public:
   Context(view &input) : stringTokenizer(input) {}
   Context(view &&input) : stringTokenizer(std::move(input)) {}

   ~Context() = default;

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

using Machine = StateMachine<States, Context>;

struct Initial : state<Initial, Machine> {

   using state<Initial, Machine>::state;

   auto transitionInternalTo() -> transitions<Content, Error> const;
};

struct Content : state<Content, Machine> {

   using state<Content, Machine>::state;

   auto transitionInternalTo() -> transitions<Content, Finished> const;
};

struct Finished : state<Finished, Machine> {

   void onEnter();
};

struct Error : state<Error, Machine> {
   void onEnter();
};

} // namespace spie::json::string
