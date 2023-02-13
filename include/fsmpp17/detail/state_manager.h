/**
 * @file state_manager.hpp
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <variant>

#include "../../base/type_traits.h"

#include "../transitions.h"
#include "../states.h"
#include "../contexts.h"
//#include "fsmpp2/config.hpp"
#include "state_container.h"
#include "substate_manager_container.h"
#include "traits.h"
#include <variant>

namespace escad::detail
{

struct NullTracer {
    template<class State, class E>
    void begin_event_handling () {}
    void end_event_handling(bool) {}
    template<class State>
    void transition() {}
};

/**
 * Manages a set of state, creates, destroys and pass events to a proper state
 **/
template<class States, class Context, class Tracer = NullTracer>
struct state_manager
{
private:
   using type_list = typename States::type_list;

public:
    state_manager(Context &ctx, Tracer& tracer)
        : context_ {ctx}
        , tracer_ {tracer}
    {
        enter_first();
    }

    ~state_manager() {
        exit();
    }

    template<class T>
    void enter() {
        exit();

        // create substate manager
        substates_.template create<T>(context_, tracer_);

        // construct state
        emplace_state<T>(context_);
    }

    void exit() {
        states_.exit();
    }

    template<class E>
    auto dispatch(E const& e) {
        auto result = false;

        substates_.visit(
            [&](auto &substate) {
                //Unused << this;
                result = substate_dispatch(substate, e);
            }
        );

        if (result == false) {
            states_.visit([this, &e, &result](auto &state) {
                tracer_.template begin_event_handling<
                    std::remove_reference_t<decltype(state)>,
                    std::remove_reference_t<decltype(e)>>();

                result = handle(state, e);
                tracer_.end_event_handling(result);
            });
        }

        return result;
    }

    template<class S>
    bool is_in() const {
        return states_.template is_in<S>();
    }

    template<class S>
    S& state() {
        return states_.template state<S>();
    }

private:
    template<class T, class C>
    void emplace_state(C &c) {
        if constexpr (std::is_constructible_v<T, C&>) {
            states_.template enter<T>(c);
        } else {
            states_.template enter<T>();
        }
    }

    template<class T, class... C>
    static constexpr bool is_constructible_by_one_of() {
        return (std::is_constructible_v<T, C&> || ...);
    }

    template<class T, class U, class... C>
    void try_emplace_state(escad::contexts<C...> &ctx) {
        if constexpr (std::is_constructible_v<T, U&>) {
            states_.template enter<T>(ctx.template get<U>());
        }
    }

    template<class T, class... C>
    void emplace_state(escad::contexts<C...> &ctx) {
        if constexpr(std::is_constructible_v<T, escad::contexts<C...> &>) {
            states_.template enter<T>(ctx);
        } else {
            if constexpr(is_constructible_by_one_of<T, C...>()) {
                (try_emplace_state<T, C, C...>(ctx), ...);
            } else {
                states_.template enter<T>();
            }
        }
    }

    void enter_first() {
        if constexpr (States::count > 0) {
            using first_t = typename mpl::type_list_first<type_list>::type;
            enter<first_t>();
        }
    }

    template<class SS, class E>
    bool substate_dispatch(SS& substate, E const &e) {
        return substate.dispatch(e);
    }

    template<class E>
    bool substate_dispatch(std::monostate, E const&) {
        return false;
    }

    template<class S, class E>
    auto handle(S &state, E const& e) -> std::enable_if_t<detail::can_handle_event<S, E>::value, bool> {
        if (handle_result(state.handle(e))) {
            return true;
        } else {
            return false;
        }
    }

    template<class S, class E>
    auto handle(S &state, E const& e) -> std::enable_if_t<detail::can_handle_event_with_context<S, E, Context>::value, bool> {
        if (handle_result(state.handle(e, context_))) {
            return true;
        } else {
            return false;
        }
    }

    template<class S, class E>
    auto handle(S&, E const& ) -> std::enable_if_t<
            !detail::can_handle_event<S, E>::value && !detail::can_handle_event_with_context<S, E, Context>::value,
    bool> {
        return false;
    }

    // state handler declared a return transitions<> return type
    template<class... T>
    bool handle_result(transitions<T...> t) {
        if (t.is_transition()) {
            handle_transition(t, std::make_index_sequence<sizeof...(T)>{});
            return true;
        }

        return t.is_handled();
    }

    template<class Transition, std::size_t... I>
    void handle_transition(Transition trans, std::index_sequence<I...>) {
        (handle_transition_impl<I>(trans), ...);
    }

    template<std::size_t I, class Transition>
    void handle_transition_impl(Transition trans) {
        if (trans.idx == I) {
            using transition_type_list = typename Transition::list;
            //using type_at_index = typename meta::type_list_type<I, transition_type_list>::type;
            using type_at_index = mpl::type_list_element_t<I, transition_type_list>;

            //if constexpr (meta::type_list_has<type_at_index>(type_list{})) {
            if (mpl::type_list_contains_v<type_list, type_at_index>) {
                tracer_.template transition<type_at_index>();
                enter<type_at_index>();
            }
        }
    }

private:
    template<class X>
    using SelfWrapper = state_manager<X, Context, Tracer>;

    using StateContainer = state_container<States>;
    using SubStateContainer = substate_manager_container<States, SelfWrapper>;

    Context&                context_;
    StateContainer          states_;
    SubStateContainer       substates_;
    Tracer&                 tracer_;
};

} // namespace fsm::detail
