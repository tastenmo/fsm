#pragma once

#include "state.h"
#include "state_machine.h"

namespace escad::new_fsm {

template <class Derived, class NestedMachine, class Context = detail::NoContext>
class composite_state : public state<Derived, Context> {

public:
  composite_state(Context &context, NestedMachine &&nested)
      : state<Derived, Context>{context}, nested_(nested) {}

  template <class Event> bool dispatch(const Event &event) {
    return nested_.dispatch(event);
  }

  template <class State> auto nested_in() const {
    return nested_.template is_in<State>();
  }

  template <class State> auto &nested_state() {
    return nested_.template state<State>();
  }

  template <class State> void nested_emplace() {
    nested_.template emplace<State>();
  }

  mpl::const_reference_t<NestedMachine> nested() { return nested_; }

private:
  NestedMachine nested_;
};

} // namespace escad::new_fsm
