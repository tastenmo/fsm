/**
 * @file state_variant.h
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

#include <core/type_traits.h>
#include <iostream>
#include <variant>

#include <core/utils.h>

#include "state.h"
#include "transition.h"

namespace escad::new_fsm {

/**
 * @brief A template class representing a variant of states.
 *
 * This class is used to manage a variant of states in a finite state machine
 * (FSM). It provides methods for emplacing states, dispatching events, and
 * accessing the current state.
 *
 * @tparam States The type representing the list of states in the FSM.
 */
template <class States, class Context = detail::NoContext> class StateMachine {
public:
  using type_list = typename States::type_list;

  using ctx = Context;

  using states_variant_list =
      typename mpl::type_list_push_front<type_list, std::monostate>::result;

  // transform a type list to a corresponding variant
  using states_variant =
      typename mpl::type_list_rename<states_variant_list, std::variant>::result;

  /**
   * @brief Default constructor for the state_variant class.
   *
   * This constructor creates a state_variant object with an States variant.
   * The mpl::type_identity is used to pass the States type to the constructor
   * and to enable class template argument deduction (CTAD).
   * Partial CTAD is not supported.
   * If context is a l-value, a reference to it is stored in content_.
   * If context is a r-value, it is moved into the content_.
   *
   * @param identity The type_identity object used to pass the States type to
   * the constructor.
   * @param context The context object.
   */
  explicit StateMachine(mpl::type_identity<States>, Context &&context)
      : context_(std::forward<Context>(context)) {}

  /**
   * @brief Emplaces a state of type State into the variant.
   *
   * This method constructs a state of type State and adds it to the variant.
   * It also calls the enter() method of the newly added state.
   *
   * @tparam State The type of the state to be emplaced.
   */
  template <class State> void emplace() {
    if constexpr (std::is_constructible_v<State, Context &>) {
      states_.template emplace<State>(context_);
    } else {
      states_.template emplace<State>();
    }

    std::visit(overloaded{[](auto &state) { state.enter(); },
                          [](std::monostate) { ; }},
               states_);

    // run internal transition handling
    visit(overloaded{[&](auto &state) { handle(state); },

                     [](std::monostate) { ; }});
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
    if constexpr (std::is_constructible_v<State, Context &>) {
      states_.template emplace<State>(context_);
    } else {
      states_.template emplace<State>();
    }
    std::visit(overloaded{[&e](auto &state) {
                            // state.enter();
                            if (!state.enter(e)) {
                              state.enter();
                            }
                          },
                          [](std::monostate) { ; }},
               states_);
  }

  template <class E> auto dispatch(E const &e) {

    auto result = false;

    visit(overloaded{[&](auto &state) { result = state.dispatch(e); },

                     [](std::monostate) { ; }});

    if (result) {
      return true;
    }

    visit(overloaded{[&](auto &state) { result = handle(state, e); },

                     [](std::monostate) { ; }});

    if (result) {
      return true;
    }

    // Run internal transition handling

    visit(overloaded{[&](auto &state) { result = handle(state); },

                     [](std::monostate) { ; }});

    return result;
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

  template <class State, class Event>
  bool handle(State &state, Event const &e) {

    if (handle_result(state.transition(e), e)) {
      return true;
    } else {
      return false;
    }
  }

  template <class State> bool handle(State &state) {

    if constexpr (detail::has_transitionInternalTo_v<State>) {
      return handle_result(state.transitionInternal());
    }

    return false;
  }

  /**
   * @brief Handles the result of all transitions.
   *
   * @tparam Event The Event type.
   * @tparam Transition The Transition type.
   * @param e The event object.
   * @param t The transition object.
   * @return true if the event is handled, false otherwise.
   */
  template <class Transition, class Event>
  bool handle_result(Transition t, Event const &e) {
    if (t.is_transition()) {
      bool handled = false;
      for_each_transition(t, [&](auto i, auto t) {
        if (i == t.idx) {
          using type_at_index = transition_t<i, Transition>;
          if constexpr (mpl::type_list_contains_v<states_variant_list,
                                                  type_at_index>) {
            emplace<type_at_index>(e);
            handled = true;
          }
        }
      });
      return handled;
    }
    // return t.is_none();
    return false;
  }

  template <class Transition> bool handle_result(Transition t) {
    if (t.is_transition()) {
      bool handled = false;
      for_each_transition(t, [&](auto i, auto t) {
        if (i == t.idx) {
          using type_at_index = transition_t<i, Transition>;
          if constexpr (mpl::type_list_contains_v<states_variant_list,
                                                  type_at_index>) {
            emplace<type_at_index>();
            handled = true;
          }
        }
      });
      return handled;
    }
    // return t.is_none();
    return false;
  }

  /**
   * @brief Visits the current state with the given visitor function.
   *
   * This method applies the given visitor function to the current state in the
   * variant.
   *
   * @tparam F The type of the visitor function.
   * @param fun The visitor function to be applied.
   */
  template <class F> auto visit(F &&fun) {
    std::visit(std::forward<F>(fun), states_);
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

  /**
   * @brief Checks if the variant is valueless by exception.
   *
   * This method checks if the variant is in a valueless state due to an
   * exception. It returns true if the variant is valueless by exception, and
   * false otherwise.
   *
   * @return true if the variant is valueless by exception, false otherwise.
   */
  constexpr bool valueless_by_exception() const noexcept {
    return states_.valueless_by_exception();
  };

  /**
   * @brief Returns a const reference to the context object.
   *
   * mpl::const_reference_t alway gives a const reference to a type (be it a
   * reference or not)
   *
   * @return A const reference to the context object.
   */
  mpl::const_reference_t<Context> context() const { return context_; }

private:
  states_variant states_;
  Context context_;
};

/**
 * @brief Deduction guide for the state_variant class.
 *
 * This deduction guide enables class template argument deduction (CTAD) for the
 * state_variant class.
 *
 * @tparam States The type representing the list of states in the FSM.
 * @tparam Context The type of the context object.
 * @param identity The type_identity object used to pass the States type to the
 * constructor.
 * @param context The context object.
 */
template <class States, class Context>
explicit StateMachine(mpl::type_identity<States>,
                      Context &&) -> StateMachine<States, Context>;

} // namespace escad::new_fsm
