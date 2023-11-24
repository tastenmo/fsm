/**
 * @file manager.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2023-11-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <type_traits>
#include <variant>

#include "../base/type_traits.h"

#include "state.h"
#include "transition.h"

namespace escad {
namespace new_fsm {

template <class States, class Context> class manager {

  // private:
  //     using type_list = typename States::type_list;

public:
  manager(Context &ctx) : context_{ctx} { enter_initial(); }

  template <class State> auto is_in() const {
    return std::holds_alternative<State>(states_);
  }

  template <class State> auto &state() { return std::get<State>(states_); }

  template <class Event> auto dispatch(Event const &e) {
    auto result = false;

    std::visit(
        [&](auto &state) {
          auto t = state.transition(e);

          result = handle_result(e, t);
        },
        states_);

    return result;
  }

private:
  template <class State> void emplace(Context &ctx) {
    states_.template emplace<State>(ctx);
  }

  template <class State> void enter() {

    // construct state with context
    emplace<State>(context_);

    std::visit([this](auto &state) { state.enter(); }, states_);
  }

  template <class State, class Event> void enter(Event const &e) {

    // construct state with context
    emplace<State>(context_);

    std::visit(
        [this, &e](auto &state) {
          state.enter();
          state.enter(e);
        },
        states_);
  }

  void enter_initial() {
    //        if constexpr (States::count > 0) {
    //            using first_t = typename
    //            mpl::type_list_first<type_list>::type; enter<first_t>();
    //        }
  }

  template <class Event, class... T>
  bool handle_result(Event const &e, transitions<T...> t) {
    if (t.is_transition()) {
      handle_transition(e, t, std::make_index_sequence<sizeof...(T)>{});
      return true;
    }

    return t.is_handled();
  }

  template <class Event, class Transition, std::size_t... I>
  void handle_transition(Event const &e, Transition trans,
                         std::index_sequence<I...>) {
    (handle_transition_impl<I>(e, trans), ...);
  }

  template <std::size_t I, class Event, class Transition>
  void handle_transition_impl(Event const &e, Transition trans) {
    if (trans.idx == I) {
      using transition_type_list = typename Transition::list;
      using type_at_index = mpl::type_list_element_t<I, transition_type_list>;
      
//      if constexpr (mpl::type_list_contains<mpl::type_list, typename Type><type_at_index>()) {

      enter<type_at_index>(e);
    }
  }

private:
  Context &context_;
  States states_;
};

} // namespace new_fsm
} // namespace escad
