
#pragma once

#include "context.h"
#include "state.h"
#include "state_machine.h"

namespace spie::fsm {

/**
 * @brief Composite state for hierarchical FSMs.
 *
 * This class represents a state that contains a nested state machine, enabling
 * hierarchical state machines. It inherits from the base state class and
 * delegates event dispatch and state management to the nested machine.
 *
 * @tparam Derived The derived state type (CRTP)
 * @tparam NestedMachine The nested state machine type
 * @tparam Machine The parent FSM machine type
 */
template <class Derived, class NestedMachine, class Machine>
class composite_state : public state<Derived, Machine> {
 public:
   /**
    * @brief Constructs a composite state with a nested state machine and parent
    * machine.
    * @param nested The nested state machine (moved in)
    * @param machine Reference to the parent FSM machine
    */
   composite_state(NestedMachine &&nested, Machine &machine)
       : state<Derived, Machine>(machine), nested_(std::move(nested)) {}

   /**
    * @brief Dispatches an event to the nested state machine.
    * @tparam Event The event type
    * @param event The event to dispatch
    * @return True if the event was handled by the nested machine
    */
   template <class Event> bool dispatch(const Event &event) {
      return nested().dispatch(event);
   }

   /**
    * @brief Checks if the nested machine is in the given state.
    * @tparam State The nested state type
    * @return True if the nested machine is in the given state
    */
   template <class State> auto nested_in() const {
      return nested().template is_in<State>();
   }

   /**
    * @brief Returns a reference to the current nested state.
    * @tparam State The nested state type
    * @return Reference to the nested state
    */
   template <class State> auto &nested_state() {
      return nested().template state<State>();
   }

   /**
    * @brief Emplaces a new state in the nested machine.
    * @tparam State The nested state type to emplace
    */
   template <class State> void nested_emplace() {
      nested().template emplace<State>();
   }

   /**
    * @brief Returns a reference to the nested state machine.
    */
   auto &nested() { return nested_; }
   /**
    * @brief Returns a const reference to the nested state machine.
    */
   const auto &nested() const { return nested_; }

   /**
    * @brief Returns a reference to the nested machine's context.
    */
   auto &nested_context() { return nested_.context(); }
   /**
    * @brief Returns a const reference to the nested machine's context.
    */
   const auto &nested_context() const { return nested_.context(); }

 private:
   NestedMachine nested_;
};

} // namespace spie::fsm
