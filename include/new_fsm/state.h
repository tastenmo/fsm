/**
 * @file state.h
 * @brief This file contains the definition of the state class and related
 * helper templates. The state class is a CRTP (Curiously Recurring Template
 * Pattern) base class for states in a finite state machine. It provides methods
 * for handling state entry, event dispatching, and state transitions. The
 * helper templates in this file are used for detecting if a state has specific
 * methods, such as onEnter() and transitionTo(). This file is part of the FSM
 * (Finite State Machine) library.
 *
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @version 0.1
 * @date 2023-02-23
 *
 * @namespace escad
 * @namespace new_fsm
 */

#pragma once

#include <optional>
#include <type_traits>

#include "../base/type_traits.h"
#include "transition.h"

namespace escad {
/**
 * @file state.h
 * @brief Defines the state class and helper templates for state management in a
 * finite state machine.
 *
 * This file contains the implementation of the state class, which is a CRTP
 * (Curiously Recurring Template Pattern) base class for states in a finite
 * state machine. It also includes helper templates for detecting the presence
 * of certain methods in derived state classes.
 *
 * The state class provides methods for handling state entry, event dispatching,
 * and state transitions. It uses SFINAE (Substitution Failure Is Not An Error)
 * and type traits to conditionally call methods based on their existence in the
 * derived state class.
 *
 * The helper templates in the details namespace are used to detect the presence
 * of the following methods in the derived state class:
 * - onEnter(): Called when entering the state without an event.
 * - onEnter(const Event&): Called when entering the state with an event.
 * - transitionTo(const Event&): Called to transition to another state based on
 * an event.
 *
 * The state class also includes a handle_result() method for handling the
 * result of state transitions and a dispatch() method for dispatching events to
 * the state.
 *
 * This file is part of the new_fsm namespace.
 */
namespace new_fsm {

namespace detail {

struct NoContext {};

struct InternalEvent {};

/**
 * @brief Default case of helper for detecting if type Target has a onEnter()
 * method.
 *
 * @tparam Target The type to check for the presence of onEnter() method.
 * @tparam void Empty type used for SFINAE.
 */
template <typename Target, typename = void>
struct has_onEnter : std::false_type {};

/**
 * @brief Specialization for detecting if type Target has a onEnter() method.
 *
 * @tparam Target The type to check for the presence of onEnter() method.
 */
template <typename Target>
struct has_onEnter<Target,
                   std::void_t<decltype(std::declval<Target>().onEnter())>>
    : std::true_type {};

/**
 * @brief Helper helper template to access the value of has_onEnter.
 *
 * @tparam T The type to check for the presence of onEnter() method.
 */
template <class T> inline constexpr bool has_onEnter_v = has_onEnter<T>::value;

/**
 * @brief Default case of helper for detecting if type Target has a
 * onEnter(const Event &) method.
 *
 * @tparam Target The type to check for the presence of onEnter(const Event &)
 * method.
 * @tparam Event The event type.
 * @tparam void Empty type used for SFINAE.
 */
template <typename Target, typename Event, typename = void>
struct has_onEnterWithEvent : std::false_type {};

/**
 * @brief Specialization for detecting if type Target has a onEnter(const Event
 * &) method.
 *
 * @tparam Target The type to check for the presence of onEnter(const Event &)
 * method.
 * @tparam Event The event type.
 */
template <typename Target, typename Event>
struct has_onEnterWithEvent<Target, Event,
                            std::void_t<decltype(std::declval<Target>().onEnter(
                                std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Helper helper template to access the value of has_onEnterWithEvent.
 *
 * @tparam T The type to check for the presence of onEnter(const Event &)
 * method.
 * @tparam E The event type.
 */
template <class T, class E>
inline constexpr bool has_onEnterWithEvent_v =
    has_onEnterWithEvent<T, E>::value;

template <typename Target, typename = void>
struct has_onExit : std::false_type {};

template <typename Target>
struct has_onExit<Target,
                  std::void_t<decltype(std::declval<Target>().onExit())>>
    : std::true_type {};

template <class T> inline constexpr bool has_onExit_v = has_onExit<T>::value;

/**
 * @brief Default case of helper for detecting if type Target has a
 * transitionTo(const Event &) method.
 *
 * @tparam Target The type to check for the presence of transitionTo(const Event
 * &) method.
 * @tparam Event The event type.
 * @tparam void Empty type used for SFINAE.
 */
template <typename Target, typename Event, typename = void>
struct has_transitionTo : std::false_type {};

/**
 * @brief Specialization for detecting if type Target has a transitionTo(const
 * Event &) method.
 *
 * @tparam Target The type to check for the presence of transitionTo(const Event
 * &) method.
 * @tparam Event The event type.
 */
template <typename Target, typename Event>
struct has_transitionTo<
    Target, Event,
    std::void_t<decltype(std::declval<Target>().transitionTo(
        std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Helper helper template to access the value of has_transitionTo.
 *
 * @tparam T The type to check for the presence of transitionTo(const Event &)
 * method.
 * @tparam E The event type.
 */
template <class T, class E>
inline constexpr bool has_transitionTo_v = has_transitionTo<T, E>::value;

template <typename Target, typename = void>
struct has_transitionInternalTo : std::false_type {};

template <typename Target>
struct has_transitionInternalTo<
    Target,
    std::void_t<decltype(std::declval<Target>().transitionInternalTo())>>
    : std::true_type {};

template <class T>
inline constexpr bool has_transitionInternalTo_v =
    has_transitionInternalTo<T>::value;

} // namespace detail

/**
 * Defines a set of states. This is used as a parameter to a StateContainer.
 **/
template <class... S> struct states {
  using type_list = mpl::type_list<S...>;
  static constexpr auto count = type_list::size;
};

/**
 * @brief state is a CRTP base class for states.
 *
 * @tparam Derived
 * @tparam StateContainer
 */
template <class Derived, class Context = detail::NoContext> struct state {
  using ctx = Context;

  //state() : context_{} {}
  state(Context &context) : context_(context) {}

  /**
   * @brief Calls onEnter(const Event &event) of Derived if it exists.
   *
   * @tparam Target The Derived type.
   * @tparam Event The Event type.
   * @param event The event object.
   * @return decltype(std::declval<Target>().onEnter(event), void())
   */
  template <class Target = Derived, class Event>
  bool enter(const Event &event) {
    if constexpr (detail::has_onEnterWithEvent_v<Target, Event>) {
      static_cast<Target *>(this)->onEnter(event);
      return true;
    }
    return false;
  }

  /**
   * @brief Calls onEnter() of Derived if it exists.
   *
   * @tparam Target The Derived type.
   * @return decltype(std::declval<Target>().onEnter(), void())
   *
   * @see https://arne-mertz.de/2017/01/decltype-declval/
   */
  template <class Target = Derived> bool enter() {
    if constexpr (detail::has_onEnter_v<Target>) {
      static_cast<Target *>(this)->onEnter();
      return true;
    }
    return false;
  }

  template <class Target = Derived> bool exit() {
    if constexpr (detail::has_onExit_v<Target>) {
      static_cast<Target *>(this)->doRun();
      return true;
    }
    return false;
  }

  /**
   * @brief Calls transitionTo(const Event &event) of Derived if it exists.
   *
   * @tparam Target The Derived type.
   * @tparam Event The Event type.
   * @param event The event object.
   * @return decltype(std::declval<Target>().transitionTo(event))
   */
  template <class Target = Derived, class Event>
  auto transition(const Event &event)
      -> decltype(std::declval<Target>().transitionTo(event)) {
    if constexpr (detail::has_transitionTo_v<Target, Event>) {
      return static_cast<Target *>(this)->transitionTo(event);
    }
  }

  /**
   * @brief For non-existing transitionTo() methods.
   *
   * @tparam Target The Derived type.
   * @param ...
   * @return transitions<detail::not_handled>
   */
  template <class Target = Derived>
  auto transition(...) -> transitions<detail::none> {
    return detail::none{};
  }

  /**
   * @brief Calls transitionInternalTo() of Derived if it exists.
   *
   * @tparam Target The Derived type.
   * @return decltype(std::declval<Target>().transitionTo())
   */
  template <class Target = Derived>
  auto transitionInternal()
      -> decltype(std::declval<Target>().transitionInternalTo()) {
    if constexpr (detail::has_transitionInternalTo_v<Target>) {
      return static_cast<Target *>(this)->transitionInternalTo();
    }
    //return detail::none{};
  }

  // template <class Target = Derived>
  //  auto transitionInternal(...) -> transitions<detail::none> {
  //    return detail::none{};
  //  }

  template <class Event>
  bool dispatch(const Event &) {
    return false;
  }

protected:
  /**
   * @brief Reference to the StateContainer.
   */
  Context& context_;
};

} // namespace new_fsm
} // namespace escad