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

#include "wink/signal.hpp"
#include "wink/slot.hpp"

namespace fsm {

template <typename... Ts> struct Overload : Ts... { using Ts::operator()...; };
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

/**
 * @brief CRTP based FSM
 *
 * @tparam Derived
 * @tparam possible States bound in std::variant
 */
template <typename StateVariant> class fsm {

public:
  using NewStateSlot = wink::slot<void(const StateVariant &)>;
  using NewStateSignal = wink::signal<NewStateSlot>;

  NewStateSignal NewState;

  /**
   * @brief Get the state object
   *
   * @return const StateVariant&
   */
  void init(const StateVariant &state) { 
    state_ = state; 
    }
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
  template <typename State>
  bool is_state() { return std::holds_alternative<State>(state_); }

  /**
   * @brief dispatch an Event
   *
   * @tparam Event
   * @param event
   */
  template <typename Event> void dispatch(Event &&event) {
    //Derived &child = static_cast<Derived &>(*this);
    // visitor to call on_event for actual state
    // gets new_state to transition or std::nullopt to stay in state_;
    auto new_state = std::visit(
        [&](auto &s) -> std::optional<StateVariant> {
          return transition(s, std::forward<Event>(event));
        },
        state_);
    // transition to new state
    if (new_state) {

      state_ = *std::move(new_state);

      // call onEnter of the state if it exists, uses decltype SFINAE, see below
      std::visit([&](auto& statePtr) { enter(statePtr, event); }, state_);

      // emit State Changed
      NewState.emit(state_);
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
        NewState.emit(state_);
      }

      // std::visit([&](auto statePtr) { handle(statePtr, event); }, state_);
    }
  }

private:
  /**
   * @brief actual state
   *
   */
  StateVariant state_;

  void enter(...) {}

  template <typename State, typename Event>
  auto enter(State &state, const Event &event)
      -> decltype(state.onEnter(event)) {
    return state.onEnter(event);
  }

  template <typename State, typename Event>
  auto enter(State &state, const Event &event) -> decltype(state.onEnter()) {
    return state.onEnter();
  }

  void exit(...) {}

  template <typename State, typename Event>
  auto exit(State &state, const Event &event)
      -> decltype(state.onExit(event)) {
    return state.onExit(event);
  }

  template <typename State, typename Event>
  auto exit(State &state, const Event &event) -> decltype(state.onExit()) {
    return state.onExit();
  }

  std::optional<StateVariant> handle(...) {
    return std::nullopt;
  }

  template <typename State, typename Event>
  auto handle(State &state, const Event &event)
      -> decltype(state.handle(event)) {
    return state.handle(event);
  }

  template <typename State, typename Event>
  auto handle(State &state, const Event &event) -> decltype(state.handle()) {
    return state.handle();
  }

  std::optional<StateVariant> transition(...) {
    return std::nullopt;
  }

  template <typename State, typename Event>
  auto transition(State &state, const Event &event)
      -> decltype(state.transitionTo(event)) {
    return state.transitionTo(event);
  }

  template <typename State, typename Event>
  auto transition(State &state, const Event &event) -> decltype(state.transitionTo()) {
    return state.transitionTo();
  }
};

} // namespace fsm
