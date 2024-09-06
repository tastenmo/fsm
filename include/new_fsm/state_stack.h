/**
 * @file state_stack.h
 * @brief This file contains the definition of the state_variant class, which is
 * a wrapper around std::variant. It provides a container for states and allows
 * for easy manipulation and dispatching of events.
 * @version 0.1
 * @date 2023-11-28
 * @author Martin Heubuch (martin.heubuch@escad.de)
 *
 * @details The state_variant class is used as the current implementation of the
 * states container in the FSM library. It is designed to be easily replaceable
 * with another implementation if needed. The class provides methods for adding
 * states, dispatching events, and accessing the current state. It also supports
 * visiting the states using a visitor function. The states are stored in a
 * std::variant, which allows for efficient storage and retrieval. The first
 * state in the variant is always std::monostate, which allows for deferred
 * creation of the actual state objects. The state_variant class is part of the
 * escad::new_fsm::detail namespace.
 */

#pragma once

#include "../base/type_traits.h"
#include <iostream>
#include <vector>

#include "../base/utils.h"
//#include "../signal/signal.h"
#include "state.h"

namespace escad::new_fsm {

template <class Context = detail::NoContext> class state_stack {
public:

  
  using ctx = Context;


  template <class T = Context,
            std::enable_if_t<std::is_constructible_v<T>, bool> = true>
  state_stack()
      : context_{}

  {}
  /**
   * @brief Default constructor for the state_stack class.
   */
  template <class T = Context,
            // enable this constructor only if Context is an lvalue ref
            std::enable_if_t<std::is_lvalue_reference_v<T>, bool> = true>
  state_stack(Context &context)
      : context_(context) {}

  template <class T = Context,
            // enable this constructor only if Context is an rvalue ref
            std::enable_if_t<!std::is_lvalue_reference_v<T>, bool> = true>
  state_stack(Context &&context)
      : context_{std::move(context)}
  {}

  /**
   * @brief Emplaces a state of type State into the variant.
   *
   * This method constructs a state of type State and adds it to the variant.
   * It also calls the enter() method of the newly added state.
   *
   * @tparam State The type of the state to be emplaced.
   */
  template <class State> void emplace() {
    if(state_stack_.size() > 0){
      state_stack_.back().exit();
      state_stack_.pop_back();
    }

    if constexpr (std::is_constructible_v<State, decltype(*this), Context &>) {
      state_stack_.emplace_back<State>(*this, context_);
    } else {
      state_stack_.emplace_back<State>(*this);
    }

    state_stack_.back().enter();

  }

  /**
   * @brief Emplaces a state of type State into the variant and passes an event
   * to it.
   *
   * This method constructs a state of type State and adds it to the variant.
   * It also calls the enter() method of the newly added state, passing the
   * given event.
   *
   * @tparam State The type of the state to be emplaced.
   * @tparam Event The type of the event to be passed to the state.
   * @param e The event to be passed to the state.
   */
  template <class State, class Event> void emplace(Event const &e) {
  
    if(state_stack_.size() > 0){
      state_stack_.back().exit();
      state_stack_.pop_back();
    }

    if constexpr (std::is_constructible_v<State, decltype(*this), Context &>) {
      state_stack_.emplace_back<State>(*this, context_);
    } else {
      state_stack_.emplace_back<State>(*this);
    }
    state_stack_.back().enter();

  }

  /**
   * @brief Handle an event.
   *
   * This method dispatches the given event to the current state in the variant.
   * It returns true if the event was handled by the state, and false otherwise.
   *
   * @tparam Event The type of the event to be dispatched.
   * @param e The event to be dispatched.
   * @return true if the event was handled by the state, false otherwise.
   */
  template <class Event> bool handle(Event const &e) {

    auto t = state_stack_.back().transition(e);

    return handle_result(e, t);
  }

  template <class Event, class Transition>
  bool handle_result(Event const &e, Transition t) {
    if (t.is_transition()) {
      bool handled = false;
      for_each_transition(t, [&](auto i, auto t) {
        if (i == t.idx) {
          using type_at_index = transition_t<i, Transition>;
//          if constexpr (mpl::type_list_contains_v<
//                            typename StateContainer::type_list,
//                            type_at_index>) {
            state_stack_.template emplace<type_at_index>(e);
            handled = true;
//          }
        }
      });
      return handled;
    }
    return t.is_handled();
  }

  
  /**
   * @brief Checks if the current state is of type State.
   *
   * This method checks if the current state in the variant is of type State.
   * It returns true if the current state is of type State, and false otherwise.
   *
   * @tparam State The type of the state to check.
   * @return true if the current state is of type State, false otherwise.
   */
  template <class State> auto is_in() const {
    return std::holds_alternative<State>(states_);
  }

  /**
   * @brief Returns a reference to the current state.
   *
   * This method returns a reference to the current state in the variant.
   *
   * @tparam State The type of the state to be accessed.
   * @return A reference to the current state.
   */
  template <class State> auto &state() { return std::get<State>(states_); }

  
  Context &context() { return context_; }

private:
  std::vector<state> state_stack_;
  Context context_;
  
};

} // namespace escad::new_fsm
