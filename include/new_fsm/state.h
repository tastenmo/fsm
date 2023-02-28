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

#include "transition.h"

namespace escad {
namespace new_fsm {


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
  auto enter(const Event &event)
      -> std::void_t<decltype(std::declval<Target>().onEnter(event))> {
    static_cast<Target *>(this)->onEnter(event);
  }

  /**
   * @brief Calls onEnter() ot Derived if it exists
   *
   * @tparam Target The Derived type
   * @return decltype(std::declval<Target>().onEnter(), void())
   *
   * @see https://arne-mertz.de/2017/01/decltype-declval/
   */
  template <typename Target = Derived>
  auto enter() -> std::void_t<decltype(std::declval<Target>().onEnter())> {
    static_cast<Target *>(this)->onEnter();
  }

  /**
   * @brief
   *
   * @param ...
   */
  void enter(...) const noexcept {}

  template <typename Target = Derived, typename Event>
  auto transition(const Event &event)
      -> decltype(std::declval<Target>().transitionTo(event)) {
    return static_cast<Target *>(this)->transitionTo(event);
  }

  auto transition(...) const noexcept {
    return transitions<>{detail::not_handled{}};
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
};

} // namespace new_fsm
} // namespace escad