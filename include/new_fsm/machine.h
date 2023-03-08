#pragma once

#include <variant>

namespace escad {
namespace new_fsm {

template <class States>
class machine {
private:
  using type_list = typename States::type_list;

public:
  void initial() {
    if constexpr (States::count > 0) {
      using first_t = typename mpl::type_list_first<type_list>::type;
      enter<first_t>();
    }
  }

  /**
   * @brief dispatch an Event
   *
   * @tparam Event
   * @param event
   */
  template <typename Event> void dispatch(Event &&event) {

    auto result = std::visit(
        [&](auto &s) -> std::optional<StateVariant> {
          // return transition(s, std::forward<Event>(event));
          return s.transition(event);
        },
        state_);

    if (handle_result(result)) {
    }
  }

  template <class State> void enter() {
    states_.template emplace<State>();
    std::visit([&](auto &statePtr) { enter(statePtr); }, states_);
    std::visit([&](auto &statePtr) { enter(statePtr, event); }, states_);
  }

  template <class... T> bool handle_result(transitions<T...> t) {
    if (t.is_transition()) {
      handle_transition(t, std::make_index_sequence<sizeof...(T)>{});
      return true;
    }

    return t.is_handled();
  }

  template <class Transition, std::size_t... I>
  void handle_transition(Transition trans, std::index_sequence<I...>) {
    (handle_transition_impl<I>(trans), ...);
  }

  template <std::size_t I, class Transition>
  void handle_transition_impl(Transition trans) {
    if (trans.idx == I) {
      using transition_type_list = typename Transition::list;
      using type_at_index = mpl::type_list_element_t<I, transition_type_list>;

      if (mpl::type_list_contains_v<type_list, type_at_index>) {
        //                tracer_.template transition<type_at_index>();
        enter<type_at_index>();
      }
    }
  }

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

} // namespace new_fsm
} // namespace escad
