#pragma once

#include "state.h"
#include "state_machine.h"
#include "context.h"

namespace spie::fsm {

template <class Derived, class NestedMachine, class Machine>
class composite_state : public state<Derived, Machine> {

public:
  composite_state(NestedMachine &&nested, Machine &machine)
      : state<Derived, Machine>(machine), nested_(std::move(nested)) {}

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

  auto& nested() { return nested_; }
  const auto& nested() const { return nested_; }

  auto& nested_context() { return nested_.context(); }
  const auto& nested_context() const { return nested_.context(); }

private:
  NestedMachine nested_;
};

} // namespace spie::fsm
