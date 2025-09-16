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

namespace spie {
namespace fsm {

namespace detail {

/**
 * @brief Tag type indicating no transition.
 */
struct none {};

/**
 * @brief Tag type indicating that the event was handled.
 */
struct handled {};

/**
 * @brief Helper struct for defining a transition type.
 * @tparam T The state type.
 */
template <class T> struct transition {
   using type = T;
};

enum class result { none, handled, transition };

} // namespace detail

/**
 * @brief Class representing a collection of transitions.
 * @tparam S The list of state types.
 */
template <class... S> class transitions {
   using result = detail::result;

 public:
   /**
    * @brief Constructs a new transitions object from another transitions
    * object.
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
   constexpr transitions(detail::transition<T>) noexcept
       : idx{mpl::type_list_index_v<T, list>}, outcome{result::transition} {}

   /**
    * @brief Constructs a new transitions object from another transitions
    * object.
    * @tparam T The list of state types.
    * @param t The transitions object.
    */
   template <class T>
   constexpr transitions(transitions<T> const &rhs) noexcept
       : idx{mpl::type_list_index_v<T, list>}, outcome{rhs.outcome} {}

   /**
    * @brief Constructs a new transitions object with no transition

    * @param h The handled tag object.
    */
   constexpr transitions(detail::none) noexcept
       : idx{mpl::type_list_index_v<detail::none, list>},
         outcome{result::none} {}

   /**
    * @brief Constructs a new transitions object indicating that the event was
    * handled.
    * @param t The transitions object.
    */
   constexpr transitions(transitions<detail::none>) noexcept
       : idx{mpl::type_list_index_v<detail::none, list>},
         outcome{result::none} {}

   constexpr transitions(detail::handled) noexcept
       : idx{mpl::type_list_index_v<detail::handled, list>},
         outcome{result::handled} {}

   constexpr transitions(transitions<detail::handled>) noexcept
       : idx{mpl::type_list_index_v<detail::handled, list>},
         outcome{result::handled} {}

   /**
    * @brief Checks if this object represents a transition.
    * @return true if the object represents a transition, false otherwise.
    */
   constexpr bool is_transition() const noexcept {
      return outcome == result::transition;
   }

   /**
    * @brief Checks if the event was handled.
    * @return true if the event was handled, false otherwise.
    */
   constexpr bool is_handled() const noexcept {
      return outcome == result::handled;
   }

   /**
    * @brief Checks if there is no transition.
    * @return true if there is no transition, false otherwise.
    */
   constexpr bool is_none() const noexcept { return outcome == result::none; }

   using list = mpl::type_list<S...>;

   std::size_t const idx;
   result const outcome;
};

/**
 * @brief Helper function for creating a transition object.
 * @tparam S The state type.
 * @return A transitions object representing the transition.
 */

template <class S> constexpr auto transition() noexcept {
   return transitions<S>{detail::transition<S>{}};
}

/**
 * @brief Helper function for creating a transitions object indicating that the
 * event was not handled.
 * @return A transitions object indicating that the event was not handled.
 */
constexpr auto none() noexcept {
   return transitions<detail::none>{detail::none{}};
}

/**
 * @brief Helper function for creating a transitions object indicating that the
 * event was handled.
 * @return A transitions object indicating that the event was handled.
 */
constexpr auto handled() noexcept {
   return transitions<detail::handled>{detail::handled{}};
}

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
                                        std::index_sequence<Is...>) noexcept {
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
                                   Func &&func) noexcept {
   for_each_transition_impl(trans, std::forward<Func>(func),
                            std::make_index_sequence<sizeof...(T)>{});
}

} // namespace fsm
} // namespace spie
