/**
 * @file states.hpp
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <tuple>
#include <type_traits>

#include "../base/assert.h"
#include "../base/type_traits.h"

#include "detail/handle_result.h"
#include "transitions.h"
//#include "fsmpp2/config.hpp"


namespace fsm
{

/**
 * Base class for an event.
 **/
struct event {};


/**
 * Defines a set of events. This is used as a parameter to a state_machine.
 **/

template<class... E>
using events = mpl::type_list<E...>;

/**
 * Defines a set of states. This is used as a parameter to a state_machine.
 **/
template<class... S>
struct states {
    using type_list = mpl::type_list<S...>;
    static constexpr auto count = type_list::size;
};

/**
 * Denotes a state.
 *
 * This is basic bulding block for a state machine. Each state must derive from this
 * template to provide necessery definitions and helper functions.
 *
 * If one or more SubStates is provided state machine will create a sub state machine.
 * All events will be passed down to a sub state machine first.
 **/

template<class... SubStates>
struct state {
    using substates_type = states<SubStates...>;
    using state_tag = void;

    /**
     * Execute transition to another state.
     *
     * It is intended to be returned from state's event handler.
     **/
    template<class S>
    auto transition() const {
        return transitions<S>{detail::transition<S>{}};
    }

    /**
     * Indicate that event was not handled.
     *
     * If there's parent state machine the vent will be processed there.
     * It is intended to be returned from state's event handler.
     **/
    auto not_handled() const {
        return transitions<>{detail::not_handled{}};
    }

    /**
     * Indicate that event was handled.
     *
     * Event processing will stop.
     * It is intended to be returned from state's event handler.
     **/
    auto handled() const {
        return transitions<>{detail::handled{}};
    }
};



} // namespace fsm
