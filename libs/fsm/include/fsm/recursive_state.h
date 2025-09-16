#pragma once

#include "state.h"
#include "state_machine.h"
#include <memory>

namespace spie::fsm {

template <class Derived, class NestedMachine, class Machine>
class recursive_state : public state<Derived, Machine> {

 public:
   recursive_state(NestedMachine &&nested, Machine &machine)
       : state<Derived, Machine>(machine) {

      nested_ = std::make_shared<NestedMachine>(std::move(nested));
   }

   template <class Event> bool dispatch(const Event &event) {
      return nested_->dispatch(event);
   }

   template <class State> auto nested_in() const {
      return nested_->template is_in<State>();
   }

   template <class State> auto &nested_state() {
      return nested_->template state<State>();
   }

   template <class State> void nested_emplace() {
      nested_->template emplace<State>();
   }

   std::shared_ptr<NestedMachine> nested() { return nested_; }

   auto &nested_context() { return nested_->context(); }
   const auto &nested_context() const { return nested_->context(); }

 private:
   std::shared_ptr<NestedMachine> nested_;
};

} // namespace spie::fsm
