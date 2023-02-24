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
      -> decltype(std::declval<Target>().onEnter(event), void()) {
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
  auto enter() -> decltype(std::declval<Target>().onEnter(), void()) {
    static_cast<Target *>(this)->onEnter();
  }

  /**
   * @brief
   *
   * @param ...
   */
  void enter(...) const noexcept {}

  /**
   * @brief Calls the transitionTo(const Event &event) of Derived if it exists
   * 
   * @tparam Target 
   * @tparam Event 
   * @tparam NewState 
   * @param event 
   * @return decltype(std::declval<Target>().template transitionTo<NewState>(event),
   * std::optional<NewState>()) 
   * 
   * @remarks Does not work yet. The NewState return type should be replaced with std::variant<states ...> from the fsm ????
   */

  template <typename Target = Derived, typename Event, typename NewState>
  auto transition(const Event &event)
      -> decltype(std::declval<Target>().template transitionTo<NewState>(event),
                  std::optional<NewState>()) {
    return static_cast<Target *>(this)->template transitionTo<NewState>(event);
  }

  auto transition(...) const noexcept { return std::nullopt; }

};

} // namespace new_fsm
} // namespace escad