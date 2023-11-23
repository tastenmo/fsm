/**
 * @file state.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2023-02-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <type_traits>

#include "../base/type_traits.h"
#include "transition.h"

namespace escad {
namespace new_fsm {

namespace details {
  /**
   * @brief Default case of helper for detecting if type Target has a onEnter() method.
   * 
   */
template <typename Target, typename = void>
struct has_onEnter : std::false_type {};

/**
 * @brief Specialization for detecting if type Target has a onEnter() method.
 * 
 */
template <typename Target>
struct has_onEnter<Target,
                   std::void_t<decltype(std::declval<Target>().onEnter())>>
    : std::true_type {};

/**
 * @brief Helper helper template to acces the value of has_onEnter.
 * 
 */
template <class T> inline constexpr bool has_onEnter_v = has_onEnter<T>::value;

  /**
   * @brief Default case of helper for detecting if type Target has a onEnter(const Event &) method.
   * 
   */
template <typename Target, typename Event, typename = void>
struct has_onEnterWithEvent : std::false_type {};

/**
 * @brief Specialization for detecting if type Target has a onEnter(const Event &) method.
 * 
 */
template <typename Target, typename Event>
struct has_onEnterWithEvent<Target, Event,
                            std::void_t<decltype(std::declval<Target>().onEnter(
                                std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Helper helper template to acces the value of has_onEnterWithEvent.
 */
template <class T, class E>
inline constexpr bool has_onEnterWithEvent_v =
    has_onEnterWithEvent<T, E>::value;

/**
 * @brief Default case of helper for detecting if type Target has a transitionTo(const Event &) method.
 * 
 */
template <typename Target, typename Event, typename = void>
struct has_transitionTo : std::false_type {};

/**
 * @brief Specialization for detecting if type Target has a transitionTo(const Event &) method.
 * 
 */
template <typename Target, typename Event>
struct has_transitionTo<
    Target, Event,
    std::void_t<decltype(std::declval<Target>().transitionTo(
        std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Helper helper template to acces the value of has_transitionTo.
 * 
 */
template <class T, class E>
inline constexpr bool has_transitionTo_v = has_transitionTo<T, E>::value;

} // namespace details

/**
 * Defines a set of states. This is used as a parameter to a state_machine.
 **/
template <class... S> struct states {
  using type_list = mpl::type_list<S...>;
  static constexpr auto count = type_list::size;
};

/**
 * @brief state base
 *
 * @tparam Derived
 */

template <typename Derived> struct state {

  /**
   * @brief Calls onEnter(const Event &event) ot Derived if it exists
   *
   * @tparam Target The Derived type
   * @tparam Event The Event
   * @param event
   * @return decltype(std::declval<Target>().onEnter(event), void())
   */

  template <typename Target = Derived, typename Event>
  auto enter(const Event &event) {
    if constexpr (details::has_onEnterWithEvent_v<Target, Event>) {

      static_cast<Target *>(this)->onEnter(event);
    }
  }

  /**
   * @brief Calls onEnter() ot Derived if it exists
   *
   * @tparam Target The Derived type
   * @return decltype(std::declval<Target>().onEnter(), void())
   *
   * @see https://arne-mertz.de/2017/01/decltype-declval/
   */
  template <typename Target = Derived> auto enter() {
    if constexpr (details::has_onEnter_v<Target>) {
      static_cast<Target *>(this)->onEnter();
    }
  }

  template <typename Target = Derived, typename Event>
  auto transition(const Event &event)
      -> decltype(std::declval<Target>().transitionTo(event)) {
    if constexpr (details::has_transitionTo_v<Target, Event>) {
      return static_cast<Target *>(this)->transitionTo(event);
    }
  }

  /*
    template <typename Target = Derived, typename Event>
    auto transition(const Event &event)
        -> std::enable_if_t<detail::hasTransitionTo<Target, Event>::value, bool>
    {

      return static_cast<Target *>(this)->transitionTo(event);
    }
  */
  template <class S> auto trans() const {
    return transitions<S>{detail::transition<S>{}};
  }

  auto not_handled() const { return transitions<>{detail::not_handled{}}; }

  auto handled() const { return transitions<>{detail::handled{}}; }
};

} // namespace new_fsm
} // namespace escad