#pragma once

#include <core/type_traits.h>
#include <iostream>
#include <variant>

#include <core/utils.h>

#include "base_machine.h"

namespace spie::fsm {

template <class States, class Context>
class StateMachine
    : public StateMachineBase<StateMachine<States, Context>, States, Context> {
 public:
   using Base =
       StateMachineBase<StateMachine<States, Context>, States, Context>;
   using typename Base::states_variant;

   explicit StateMachine(mpl::type_identity<States>, Context &context)
       : Base(mpl::type_identity<States>{}, context) {}

   explicit StateMachine(mpl::type_identity<States>, Context &&context)
       : Base(mpl::type_identity<States>{}, std::forward<Context>(context)) {}
};

} // namespace spie::fsm
