/**
 * @file transition.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2023-02-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include "../base/type_traits.h"

namespace escad {
namespace new_fsm {

namespace detail {

struct handled {};
struct not_handled {};
template <class T> struct transition {
  using type = T;
};

} // namespace detail

template <class... S> class transitions {
private:
  enum class result { not_handled, handled, transition };

public:
  /**
   * @brief Construct new object from empty-listed transactions object.
   *
   * This is a case when handler declares a return type:
   *      transitions<A, B, C> handle(Foo);
   * but calls handled() or not_handled() which returns exactly transitions<>
   * type which needs to be converted here to transitions<A, B, C>.
   *
   */
  template <class... T>
  constexpr transitions(transitions<T...> const &rhs) noexcept
      : idx{0}, outcome{rhs.outcome} {}

  /**
   * @brief Construct a new transitions object directly from
   * detail::transition<>
   *
   * @tparam T state
   */
  template <class T>
  transitions(detail::transition<T>) noexcept
      : idx{mpl::type_list_index_v<T, list>}, outcome{result::transition} {}

  template <class T>
  transitions(transitions<T>) noexcept
      : idx{mpl::type_list_index_v<T, list>}, outcome{result::transition} {}

  /**
   * @brief Construct a new transitions object indicating handled event
   */
  transitions(detail::handled) noexcept
      : idx{mpl::type_list_index_v<detail::handled, list>},
        outcome{result::handled} {}

  transitions(transitions<detail::handled>) noexcept
      : idx{mpl::type_list_index_v<detail::handled, list>},
        outcome{result::handled} {}

  /**
   * @brief Construct a new transitions object indicating not handled event
   */
  transitions(detail::not_handled) noexcept
      : idx{mpl::type_list_index_v<detail::not_handled, list>},
        outcome{result::not_handled} {}

  transitions(transitions<detail::not_handled>) noexcept
      : idx{mpl::type_list_index_v<detail::not_handled, list>},
        outcome{result::not_handled} {}

  /**
   * @brief Check if this object is a transition type.
   *
   * @return true if the object should result in transition
   * @return false if the object is not resulting in transition
   */
  bool is_transition() const { return outcome == result::transition; }

  /**
   * @brief Check if the event was handled.
   *
   * @return true event was handled by the state.
   * @return false event was not handled. Possibly propagate event up in the SM.
   */
  bool is_handled() const { return outcome == result::handled; }

  using list = mpl::type_list<S...>;

  std::size_t const idx;
  result const outcome;
};

template <class S> inline auto trans() {
  return transitions<S>{detail::transition<S>{}};
}

inline auto not_handled() {
  return transitions<detail::not_handled>{detail::not_handled{}};
}

inline auto handled() {
  return transitions<detail::handled>{detail::handled{}};
}


template <std::size_t Index, class Transition>
using transition_t = mpl::type_list_element_t<Index, typename Transition::list>;


template <class Transition, class Func, std::size_t ...Is>
constexpr void for_each_transition_impl(Transition&& t, Func &&func, std::index_sequence<Is...>) {
  (func(std::integral_constant<std::size_t, Is>{}, t), ...);
}

template <class ... T, class Func>
constexpr void for_each_transition(transitions<T...> const &trans, Func &&func) {
  for_each_transition_impl(trans, std::forward<Func>(func), std::make_index_sequence<sizeof...(T)>{});

}

} // namespace new_fsm
} // namespace escad
