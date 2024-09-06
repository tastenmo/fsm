

#pragma once

#include "../base/type_traits.h"
#include "transition.h"
#include "state.h"

namespace escad {
    namespace new_fsm{

template <class... S> struct transition_list {
  using type_list = mpl::type_list<S...>;
  static constexpr auto count = type_list::size;
};
   
//template <class State> struct internalTransitions{
//  using type = State;
//};

template <class Target>
  using internalTransitions = decltype(std::declval<Target>().transitionInternalTo());


template <class Target, class Event>
 using externalTransitions = decltype(std::declval<Target>().transitionTo(std::declval<Event>()));

//template <class Target, class Event>
//  constexpr auto externalTransitions() {
//    return decltype(std::declval<Target>().transitionTo(std::declval<Event>()));
//  }


} // namespace new_fsm

} // namespace escad
