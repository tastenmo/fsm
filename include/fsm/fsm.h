/**
 * @file fsm.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief Finite state machine implementation
 * @version 0.1
 * @date 2022-03-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <optional>
#include <utility>
#include <variant>

#include "../signal/signal.h"

namespace escad {

inline namespace fsm_ {

template <typename... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

/**
 * @brief This file contains the implementation of the FSM (Finite State Machine) library.
 * 
 * The FSM library provides a set of templates and utilities for creating and managing finite state machines.
 * It includes type traits to check if a class has certain member functions, such as onEnter, onEnterWithEvent,
 * transitionTo, and handle. These traits are used to enable or disable certain functionality based on the
 * presence of these member functions.
 * 
 * The details namespace contains internal implementation details of the FSM library.
 */
namespace details {

/**
 * @brief Type trait to check if a class has the onEnter member function.
 * 
 * This trait is used to determine if a state class has an onEnter member function, which is called when the
 * state is entered.
 * 
 * @tparam Target The class to check.
 * @tparam void Dummy template parameter for SFINAE.
 */
template <typename Target, typename = void>
struct has_onEnter : std::false_type {};

/**
 * @brief Type trait specialization for checking if a class has the onEnter member function.
 * 
 * This specialization is enabled if the expression decltype(std::declval<Target>().onEnter()) is well-formed,
 * which means the class has the onEnter member function.
 * 
 * @tparam Target The class to check.
 */
template <typename Target>
struct has_onEnter<Target,
           std::void_t<decltype(std::declval<Target>().onEnter())>>
  : std::true_type {};

/**
 * @brief Convenience variable template to check if a class has the onEnter member function.
 * 
 * This variable template is defined as the value of the has_onEnter trait for the given class.
 * 
 * @tparam T The class to check.
 */
template <class T>
inline constexpr bool has_onEnter_v = has_onEnter<T>::value;

/**
 * @brief Type trait to check if a class has the onEnter member function with a specific event type.
 * 
 * This trait is used to determine if a state class has an onEnter member function that takes an event
 * parameter of a specific type. This is useful for states that need to perform different actions based
 * on the event that triggered the state transition.
 * 
 * @tparam Target The class to check.
 * @tparam Event The event type to check.
 * @tparam void Dummy template parameter for SFINAE.
 */
template <typename Target, typename Event, typename = void>
struct has_onEnterWithEvent : std::false_type {};

/**
 * @brief Type trait specialization for checking if a class has the onEnter member function with a specific event type.
 * 
 * This specialization is enabled if the expression decltype(std::declval<Target>().onEnter(std::declval<Event>())) is well-formed,
 * which means the class has the onEnter member function with the specified event type.
 * 
 * @tparam Target The class to check.
 * @tparam Event The event type to check.
 */
template <typename Target, typename Event>
struct has_onEnterWithEvent<Target, Event,
              std::void_t<decltype(std::declval<Target>().onEnter(
                std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Convenience variable template to check if a class has the onEnter member function with a specific event type.
 * 
 * This variable template is defined as the value of the has_onEnterWithEvent trait for the given class and event type.
 * 
 * @tparam T The class to check.
 * @tparam E The event type to check.
 */
template <class T, class E>
inline constexpr bool has_onEnterWithEvent_v =
  has_onEnterWithEvent<T, E>::value;

/**
 * @brief Type trait to check if a class has the transitionTo member function.
 * 
 * This trait is used to determine if a state class has a transitionTo member function, which is called to
 * transition to a new state.
 * 
 * @tparam Target The class to check.
 * @tparam Event The event type to check.
 * @tparam void Dummy template parameter for SFINAE.
 */
template <typename Target, typename Event, typename = void>
struct has_transitionTo : std::false_type {};

/**
 * @brief Type trait specialization for checking if a class has the transitionTo member function.
 * 
 * This specialization is enabled if the expression decltype(std::declval<Target>().transitionTo(std::declval<Event>())) is well-formed,
 * which means the class has the transitionTo member function.
 * 
 * @tparam Target The class to check.
 * @tparam Event The event type to check.
 */
template <typename Target, typename Event>
struct has_transitionTo<
  Target, Event,
  std::void_t<decltype(std::declval<Target>().transitionTo(
    std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Convenience variable template to check if a class has the transitionTo member function.
 * 
 * This variable template is defined as the value of the has_transitionTo trait for the given class and event type.
 * 
 * @tparam T The class to check.
 * @tparam E The event type to check.
 */
template <class T, class E>
inline constexpr bool has_transitionTo_v = has_transitionTo<T, E>::value;

/**
 * @brief Type trait to check if a class has the handle member function.
 * 
 * This trait is used to determine if a state class has a handle member function, which is called to handle
 * an event.
 * 
 * @tparam Target The class to check.
 * @tparam Event The event type to check.
 * @tparam void Dummy template parameter for SFINAE.
 */
template <typename Target, typename Event, typename = void>
struct has_handle : std::false_type {};

/**
 * @brief Type trait specialization for checking if a class has the handle member function.
 * 
 * This specialization is enabled if the expression decltype(std::declval<Target>().handle(std::declval<Event>())) is well-formed,
 * which means the class has the handle member function.
 * 
 * @tparam Target The class to check.
 * @tparam Event The event type to check.
 */
template <typename Target, typename Event>
struct has_handle<
  Target, Event,
  std::void_t<decltype(std::declval<Target>().handle(std::declval<Event>()))>>
  : std::true_type {};

/**
 * @brief Convenience variable template to check if a class has the handle member function.
 * 
 * This variable template is defined as the value of the has_handle trait for the given class and event type.
 * 
 * @tparam T The class to check.
 * @tparam E The event type to check.
 */
template <class T, class E>
inline constexpr bool has_handle_v = has_handle<T, E>::value;

} // namespace details

/**
 * @brief CRTP based FSM
 *
 * @tparam Derived
 * @tparam possible States bound in std::variant
 *
 * @remark Implementing a state in a cpp file is not a good idea, because SFINAE
 * does not work then!!!
 */
template <typename StateVariant> class fsm {
public:
  using NewStateType = escad::signal<void(const StateVariant &)>;

  /**
   * @brief Construct a new fsm object
   *
   */
  fsm() : state_{}, NewStateSignal_{}, NewState{NewStateSignal_} {}

  fsm(const StateVariant &initial)
      : state_{initial}, NewStateSignal_{}, NewState{NewStateSignal_} {}

  /**
   * @brief Get the state object
   *
   * @return const StateVariant&
   */
  void init(const StateVariant &state) { state_ = state; }
  /**
   * @brief Get the state object
   *
   * @return const StateVariant&
   */
  const StateVariant &get_state() const { return state_; }

  /**
   * @brief Get the state object
   *
   * @return StateVariant&
   */
  StateVariant &get_state() { return state_; }

  /**
   * @brief Check the state object
   *
   * @return StateVariant&
   */
  template <typename State> bool is_state() {
    return std::holds_alternative<State>(state_);
  }

  /**
   * @brief dispatch an Event
   *
   * @tparam Event
   * @param event
   */
  template <typename Event> void dispatch(Event &&event) {
    // Derived &child = static_cast<Derived &>(*this);
    //  visitor to call on_event for actual state
    //  gets new_state to transition or std::nullopt to stay in state_;
    auto new_state = std::visit(
        [&](auto &s) -> std::optional<StateVariant> {
          // return transition(s, std::forward<Event>(event));
          return transition(s, event);
        },
        state_);
    // transition to new state
    if (new_state) {
      state_ = *std::move(new_state);

      // call onEnter of the state if it exists, uses decltype SFINAE, see below
      std::visit([&](auto &statePtr) { enter(statePtr); }, state_);
      std::visit([&](auto &statePtr) { enter(statePtr, event); }, state_);

      // emit State Changed
      NewStateSignal_.publish(state_);
    } else {
      auto new_state_internal = std::visit(
          [&](auto &statePtr) -> std::optional<StateVariant> {
            return handle(statePtr, event);
          },
          state_);
      if (new_state_internal) {
        state_ = *std::move(new_state_internal);

        // call onEnter of the state if it exists, uses decltype SFINAE, see
        // below
        std::visit([&](auto &statePtr) { enter(statePtr, event); }, state_);

        // emit State Changed
        NewStateSignal_.publish(state_);
      }

      // std::visit([&](auto statePtr) { handle(statePtr, event); }, state_);
    }
  }

private:
  StateVariant state_;
  NewStateType NewStateSignal_;

public:
  escad::slot<NewStateType> NewState;

private:
  template <typename State, typename Event>
  void enter(State &state, const Event &event) {
    if constexpr (details::has_onEnterWithEvent_v<State, Event>) {
      state.onEnter(event);
    }
  }

  template <typename State> void enter(State &state) {
    if constexpr (details::has_onEnter_v<State>) {
      state.onEnter();
    }
  }

  template <typename State, typename Event>
  std::optional<StateVariant> handle(State &state, const Event &event) {
    if constexpr (details::has_handle_v<State, Event>) {

      return state.handle(event);
    } else {
      return std::nullopt;
    }
  }

  template <typename State, typename Event>
  std::optional<StateVariant> transition(State &state, const Event &event) {
    if constexpr (details::has_transitionTo_v<State, Event>) {
      return state.transitionTo(event);
    } else {
      return std::nullopt;
    }
  }
};

} // namespace fsm_

} // namespace escad
