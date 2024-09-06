
#pragma once

#include "state.h"
#include "state_variant.h"

namespace escad::new_fsm {

template <class Derived, class StateContainer, class Context = detail::NoContext>
class initial_state : public state<Derived, Context> {
public:
  // using Container = state_variant<States, Context>;

  using state<Derived, Context>::state;

  

public:

  static StateContainer create() {

    auto container = StateContainer();

    container.template emplace<Derived>();

    return container;
  }


  static StateContainer create(Context& context) {

    auto container = StateContainer(context);

    container.template emplace<Derived>();

    return container;
  }

  static StateContainer create(Context&& context) {

    auto container = StateContainer(std::move(context));

    container.template emplace<Derived>();

    return container;
  }
};

} // namespace escad::new_fsm