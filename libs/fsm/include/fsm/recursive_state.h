#pragma once

#include "state.h"
#include "state_machine.h"
#include <memory>

namespace escad::new_fsm {

template <class Derived, class NestedMachine, class Context = detail::NoContext>
class recursive_state : public state<Derived, Context> {

public:
  recursive_state(Context &context, NestedMachine &&nested)
      : state<Derived, Context>{context} {

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

private:
  std::shared_ptr<NestedMachine> nested_;
};

} // namespace escad::new_fsm
