/**
 * @file state.h
 * @brief Defines the CRTP base class for FSM states and SFINAE-based helpers
 * for method detection.
 *
 * This file is part of the FSM (Finite State Machine) library. It provides:
 *   - The `state` CRTP base class for user-defined FSM states
 *   - SFINAE-based helper templates to detect presence of state methods (e.g.,
 * onEnter, onExit, transitionTo)
 *   - The `states` type-list container for compile-time state sets
 *
 * Usage:
 *   - Derive your state classes from `state<YourState, Machine>`
 *   - Optionally implement methods like `onEnter()`, `onEnter(const Event&)`,
 * `onExit()`, `transitionTo(const Event&)`, etc.
 *   - The base class will automatically call these methods if present
 *
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @date 2023-02-23
 * @version 0.2
 */

#pragma once

#include <optional>
#include <type_traits>

#include "transition.h"
#include <core/type_traits.h>

namespace spie {
namespace fsm {

namespace detail {

struct InternalEvent {};

/**
 * @brief Trait to detect if a type has a member function `onEnter()`.
 *
 * Usage: `has_onEnter_v<T>` is true if T has `void onEnter()`.
 * @tparam Target Type to check
 */
template <typename Target, typename = void>
struct has_onEnter : std::false_type {};

/**
 * @brief SFINAE specialization: true if Target has `void onEnter()`.
 */
template <typename Target>
struct has_onEnter<Target,
                   std::void_t<decltype(std::declval<Target>().onEnter())>>
    : std::true_type {};

/**
 * @brief Helper variable template for `has_onEnter` trait.
 */
template <class T> inline constexpr bool has_onEnter_v = has_onEnter<T>::value;

/**
 * @brief Trait to detect if a type has a member function `onEnter(const
 * Event&)`.
 *
 * Usage: `has_onEnterWithEvent_v<T, E>` is true if T has `void onEnter(const
 * E&)`.
 * @tparam Target Type to check
 * @tparam Event Event type
 */
template <typename Target, typename Event, typename = void>
struct has_onEnterWithEvent : std::false_type {};

/**
 * @brief SFINAE specialization: true if Target has `void onEnter(const
 * Event&)`.
 */
template <typename Target, typename Event>
struct has_onEnterWithEvent<Target, Event,
                            std::void_t<decltype(std::declval<Target>().onEnter(
                                std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Helper variable template for `has_onEnterWithEvent` trait.
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
 * @brief Trait to detect if a type has a member function `transitionTo(const
 * Event&)`.
 *
 * Usage: `has_transitionTo_v<T, E>` is true if T has `transitionTo(const E&)`.
 * @tparam Target Type to check
 * @tparam Event Event type
 */
template <typename Target, typename Event, typename = void>
struct has_transitionTo : std::false_type {};

/**
 * @brief SFINAE specialization: true if Target has `transitionTo(const
 * Event&)`.
 */
template <typename Target, typename Event>
struct has_transitionTo<
    Target, Event,
    std::void_t<decltype(std::declval<Target>().transitionTo(
        std::declval<Event>()))>> : std::true_type {};

/**
 * @brief Helper variable template for `has_transitionTo` trait.
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
 * @brief Compile-time container for a set of state types.
 *
 * Usage: `states<StateA, StateB, ...>`
 * Provides a type list and a count of states.
 */
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
template <class Derived, class Machine> struct state {
   // ...existing code...
   state(Machine &sm) : machine_(sm) {}

   /**
    * @brief Calls `onEnter(const Event&)` of Derived if present.
    *
    * If the derived state class implements `void onEnter(const Event&)`, it
    * will be called. Returns true if called, false otherwise.
    *
    * @tparam Target Derived state type
    * @tparam Event Event type
    * @param event Event object
    * @return true if `onEnter(const Event&)` was called, false otherwise
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
    * @brief Calls `onEnter()` of Derived if present.
    *
    * If the derived state class implements `void onEnter()`, it will be called.
    * Returns true if called, false otherwise.
    *
    * @tparam Target Derived state type
    * @return true if `onEnter()` was called, false otherwise
    */
   template <class Target = Derived> bool enter() {
      if constexpr (detail::has_onEnter_v<Target>) {
         static_cast<Target *>(this)->onEnter();
         return true;
      }
      return false;
   }

   /**
    * @brief Calls `onExit()` of Derived if present.
    *
    * If the derived state class implements `void onExit()`, it will be called.
    * Returns true if called, false otherwise.
    *
    * @tparam Target Derived state type
    * @return true if `onExit()` was called, false otherwise
    */
   template <class Target = Derived> bool exit() {
      if constexpr (detail::has_onExit_v<Target>) {
         static_cast<Target *>(this)->onExit();
         return true;
      }
      return false;
   }

   /**
    * @brief Calls `transitionTo(const Event&)` of Derived if present.
    *
    * If the derived state class implements `transitionTo(const Event&)`, it
    * will be called. Returns the result of the call if present.
    *
    * @tparam Target Derived state type
    * @tparam Event Event type
    * @param event Event object
    * @return Result of `transitionTo(const Event&)` if present
    */
   template <class Target = Derived, class Event>
   auto trans(const Event &event)
       -> decltype(std::declval<Target>().transitionTo(event)) {
      if constexpr (detail::has_transitionTo_v<Target, Event>) {
         return static_cast<Target *>(this)->transitionTo(event);
      }
   }

   /**
    * @brief Fallback for missing `transitionTo()` methods.
    *
    * Returns a default transition (none) if `transitionTo()` is not present in
    * Derived.
    *
    * @tparam Target Derived state type
    * @return transitions<detail::none>
    */
   template <class Target = Derived>
   auto trans(...) -> transitions<detail::none> {
      return detail::none{};
   }

   /**
    * @brief Calls `transitionInternalTo()` of Derived if present.
    *
    * If the derived state class implements `transitionInternalTo()`, it will be
    * called. Returns the result of the call if present.
    *
    * @tparam Target Derived state type
    * @return Result of `transitionInternalTo()` if present
    */
   template <class Target = Derived>
   auto
   transInternal() -> decltype(std::declval<Target>().transitionInternalTo()) {
      if constexpr (detail::has_transitionInternalTo_v<Target>) {
         return static_cast<Target *>(this)->transitionInternalTo();
      }
      // return detail::none{};
   }

   /**
    * @brief Default event dispatch (no-op).
    *
    * Override in your state class for custom event handling.
    * @tparam Event Event type
    * @return Always returns false
    */
   template <class Event> bool dispatch(const Event &) { return false; }

   /**
    * @brief Asynchronously dispatches an event to the machine.
    *
    * Calls `machine_.dispatch(event)`.
    * @param event Event object
    */
   template <class Event> void asyncDispatch(const Event &event) {
      machine_.dispatch(event);
   }

   /**
    * @brief Returns reference to the FSM context.
    */
   auto &context() { return machine_.context(); }

   /**
    * @brief Returns reference to the FSM machine.
    */
   auto &machine() { return machine_; }

 protected:
   /**
    * @brief Reference to the FSM machine instance.
    */
   Machine &machine_;
};

} // namespace fsm
} // namespace spie