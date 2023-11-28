/**
 * @file state_variant.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include "../base/type_traits.h"
#include <variant>

namespace escad::new_fsm::detail {

/**
 * Wrapper around std::variant, which is current implementation
 * of states container. This is done in separate class to abstract it
 * out in case we want to replace std::variant with something else.
 **/
template <class States> class state_variant {
private:
  using type_list = typename States::type_list;

public:
  template <class State, class Context, class Event>
  void emplace(Context &ctx, Event const &e) {
    states_.template emplace<State>(ctx);
  }

  template <class State, class Event> void emplace(Event const &e) {
    states_.template emplace<State>();
  }

  template <class F> auto visit(F &&fun) {
    std::visit(std::forward<F>(fun), states_);
  }

  template <class State> auto is_in() const {
    return std::holds_alternative<State>(states_);
  }

  template <class State> auto &state() { return std::get<State>(states_); }

  constexpr bool valueless_by_exception() const noexcept {
    return states_.valueless_by_exception();
  };

private:
  // Prepend a list of states with std::monostate. We want to avoid a situation
  // that State will be constructed by defaulted when created instance of this
  // class.
  // 1. maybe we want to defer creation of State object
  // 2. first state does not necessarily have default constructor
  using states_variant_list =
      typename mpl::type_list_push_front<type_list, std::monostate>::result;

  // transform a type list to a corresponding variant
  using states_variant =
      typename mpl::type_list_rename<states_variant_list, std::variant>::result;

  states_variant states_;
};

} // namespace escad::new_fsm::detail
