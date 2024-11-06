#pragma once

#include "initial_state.h"

namespace escad::new_fsm {

template <class Derived, class NestedState, class Context = detail::NoContext>
class composite_state : public state<Derived, Context> {

  using NestedContext = typename NestedState::ctx;
  using NestedContainer = typename NestedState::states;

public:
  composite_state(Context &context)
      : state<Derived, Context>{context}, nested_(NestedState::create(context))

  {}

  composite_state(Context &context, NestedContext &nested_context)
      : state<Derived, Context>{context},
        nested_(NestedState::create(nested_context))

  {}

  composite_state(Context &context, NestedContext &&nested_context)
      : state<Derived, Context>{context},
        nested_(
            NestedState::create(std::forward<NestedContext>(nested_context)))

  {}

  template <class Event> bool dispatch(const Event &event) {
    return nested_.dispatch(event);
  }

  template <class State> auto nested_in() const {
    return nested_.template is_in<State>();
  }

  template <class State> auto &nested_state() {
    return nested_.template state<State>();
  }

  NestedContext &nested_context() { return nested_.context(); }

private:
  NestedContainer nested_;
};

} // namespace escad::new_fsm
