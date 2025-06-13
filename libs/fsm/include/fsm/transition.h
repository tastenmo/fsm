/**
 * @file transition.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief Defines the transitions class and related helper functions.
 * @version 0.1
 * @date 2023-02-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <core/type_traits.h>

namespace escad {

namespace new_fsm {

namespace detail {

/**
 * @brief Tag type indicating no transition.
 */
struct none {};

/**
 * @brief Helper struct for defining a transition type.
 * @tparam T The state type.
 */
template <class T> struct sibling {
  using type = T;
};

template <class T> struct inner {
  using type = T;
};

template <class T> struct inner_entry {
  using type = T;
};

enum class result { none, sibling, inner, inner_entry };

} // namespace detail

/**
 * @brief Class representing a collection of transitions.
 * @tparam S The list of state types.
 */
template <class... S> class transitions {
  using result = detail::result;

public:
  /**
   * @brief Constructs a new transitions object from another transitions object.
   * @tparam T The list of state types.
   * @param rhs The transitions object to copy from.
   */
  template <class... T>
  constexpr transitions(transitions<T...> const &rhs) noexcept
      : idx{0}, outcome{rhs.outcome} {}

  /**
   * @brief Constructs a new transitions object from a detail::transition type.
   * @tparam T The state type.
   * @param t The detail::transition object.
   */
  template <class T>
  transitions(detail::sibling<T>) noexcept
      : idx{mpl::type_list_index_v<T, list>}, outcome{result::sibling} {}

  template <class T>
  transitions(detail::inner<T>) noexcept
      : idx{mpl::type_list_index_v<T, list>}, outcome{result::inner} {}

  template <class T>
  transitions(detail::inner_entry<T>) noexcept
      : idx{mpl::type_list_index_v<T, list>}, outcome{result::inner_entry} {}

  /**
   * @brief Constructs a new transitions object from another transitions object.
   * @tparam T The list of state types.
   * @param t The transitions object.
   */
  template <class T>
  transitions(transitions<T> const &rhs) noexcept
      : idx{mpl::type_list_index_v<T, list>}, outcome{rhs.outcome} {}

  /**
   * @brief Constructs a new transitions object with no transition

   * @param h The handled tag object.
   */
  transitions(detail::none) noexcept
      : idx{mpl::type_list_index_v<detail::none, list>}, outcome{result::none} {
  }

  /**
   * @brief Constructs a new transitions object indicating that the event was
   * handled.
   * @param t The transitions object.
   */
  transitions(transitions<detail::none>) noexcept
      : idx{mpl::type_list_index_v<detail::none, list>}, outcome{result::none} {
  }

  /**
   * @brief Checks if this object represents a transition.
   * @return true if the object represents a transition, false otherwise.
   */
  bool is_transition() const { return outcome == result::sibling; }

  bool is_sibling() const { return outcome == result::sibling; }

  bool is_inner() const { return outcome == result::inner; }

  bool is_inner_entry() const { return outcome == result::inner_entry; }

  /**
   * @brief Checks if the event was handled.
   * @return true if the event was handled, false otherwise.
   */
  bool is_none() const { return outcome == result::none; }

  using list = mpl::type_list<S...>;

  std::size_t const idx;
  result const outcome;
};

/**
 * @brief Helper function for creating a transition object.
 * @tparam S The state type.
 * @return A transitions object representing the transition.
 */
template <class S> inline auto sibling() {
  return transitions<S>{detail::sibling<S>{}};
}

template <class S> inline auto inner() {
  return transitions<S>{detail::inner<S>{}};
}

template <class S> inline auto inner_entry() {
  return transitions<S>{detail::inner_entry<S>{}};
}

/**
 * @brief Helper function for creating a transitions object indicating that the
 * event was not handled.
 * @return A transitions object indicating that the event was not handled.
 */
inline auto none() { return transitions<detail::none>{detail::none{}}; }

/**
 * @brief Type alias for the transition type at a specific index.
 * @tparam Index The index of the transition.
 * @tparam Transition The transitions object.
 */
template <std::size_t Index, class Transition>
using transition_t = mpl::type_list_element_t<Index, typename Transition::list>;

/**
 * @brief Helper function for getting the transition type at a specific index.
 * @tparam Index The index of the transition.
 * @tparam Transition The transitions object.
 * @param t The transitions object.
 * @return The transition type at the specified index.
 */
template <class Transition, class Func, std::size_t... Is>
constexpr void for_each_transition_impl(Transition &&t, Func &&func,
                                        std::index_sequence<Is...>) {
  (func(std::integral_constant<std::size_t, Is>{}, t), ...);
}

/**
 * @brief Helper function for iterating over each transition in a transitions
 * object.
 * @tparam T The list of state types.
 * @tparam Func The type of the function to apply to each transition.
 * @param trans The transitions object.
 * @param func The function to apply to each transition.
 */
template <class... T, class Func>
constexpr void for_each_transition(transitions<T...> const &trans,
                                   Func &&func) {
  for_each_transition_impl(trans, std::forward<Func>(func),
                           std::make_index_sequence<sizeof...(T)>{});
}

} // namespace new_fsm
} // namespace escad
