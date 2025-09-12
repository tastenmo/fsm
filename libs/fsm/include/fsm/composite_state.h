#pragma once

#include "state.h"
#include "state_machine.h"

namespace escad::new_fsm {

template <class Derived, class NestedMachine, class Machine>
class composite_state : public state<Derived, Machine> {

public:
  composite_state(NestedMachine &&nested, Machine &machine)
      : state<Derived, Machine>(machine), nested_(nested) {}

  template <class Event> bool dispatch(const Event &event) {
    return nested().dispatch(event);
  }

  template <class State> auto nested_in() const {
    return nested().template is_in<State>();
  }

  template <class State> auto &nested_state() {
    return nested().template state<State>();
  }

  template <class State> void nested_emplace() {
    nested().template emplace<State>();
  }

  auto& nested() { return nested_.get(); }
  const auto& nested() const { return nested_.get(); }

private:
  std::reference_wrapper<NestedMachine> nested_;
};

} // namespace escad::new_fsm
