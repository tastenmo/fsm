#pragma once

#include <core/type_traits.h>
#include <iostream>
#include <variant>

#include <core/utils.h>

#include <boost/asio.hpp>

#include "base_machine.h"

namespace io = boost::asio;

namespace spie::fsm {

template <class States, class Context>
class AsyncStateMachine
    : public StateMachineBase<AsyncStateMachine<States, Context>, States,
                              Context> {
 public:
   using Base =
       StateMachineBase<AsyncStateMachine<States, Context>, States, Context>;
   using typename Base::states_variant;

   explicit AsyncStateMachine(mpl::type_identity<States>, Context &context,
                              io::io_context &io_context)
       : Base(mpl::type_identity<States>{}, context), io_context_(io_context) {}

   explicit AsyncStateMachine(mpl::type_identity<States>, Context &&context,
                              io::io_context &io_context)
       : Base(mpl::type_identity<States>{}, std::forward<Context>(context)),
         io_context_(io_context) {}

   template <class State> void asyncEmplace() {
      boost::asio::post(io_context_.get(),
                        [this]() { this->template emplace<State>(); });
   }

   template <class State> bool asyncHandle(State &state) {
      bool handled = false;
      boost::asio::post(io_context_.get(), [this, &state, &handled]() {
         handled = this->template handle<State>(state);
      });
      return handled;
   }

   /**
    * @brief Asynchronously dispatches an event to the state machine.
    *
    * This method posts the event dispatch to the Boost.Asio io_context
    * associated with the context of the state machine. The event will be
    * dispatched in the context of the io_context's event loop.
    *
    * @tparam Event The type of the event to dispatch.
    * @param event The event object to dispatch.
    */
   template <class Event> void asyncDispatch(const Event &event) {
      boost::asio::post(io_context_.get(),
                        [this, event]() { this->dispatch(event); });
   }

   io::io_context &get_io_context() { return io_context_.get(); }

 private:
   std::reference_wrapper<io::io_context> io_context_;
};

} // namespace spie::fsm