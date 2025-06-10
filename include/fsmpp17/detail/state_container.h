/**
 * @file state_container.h
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

#include "../access_context.h"
#include "traits.h"

namespace escad::detail
{

    /**
     * Wrapper around std::variant, which is current implementation
     * of states container. This is done in separate class to abstract it
     * out in case we want to replace std::variant with something else.
     **/
    template <class States>
    class state_container
    {
    private:
        using type_list = typename States::type_list;

    public:
        /**
         * Enter 'State' state.
         *
         * It is realized by emplacing a new State object within a std::variant.
         **/
        template <class State>
        void enter()
        {
            states_.template emplace<State>();
        }

        /**
         * Enter 'State' state.
         *
         * Emplace new State and pass a Context to its constructor. This is caller
         * responsibility to determine if given State have proper constructor.
         **/
        template <class State, class Context>
        void enter(Context &ctx)
        {
            states_.template emplace<State>(ctx);
        }

        /**
         * Exit current state.
         **/
        void exit()
        {
            states_.template emplace<std::monostate>();
        }

        template <class F>
        auto visit(F &&fun)
        {
            std::visit(std::forward<F>(fun), states_);
        }

        template <class State>
        auto is_in() const
        {
            return std::holds_alternative<State>(states_);
        }

        template <class State>
        auto &state()
        {
            return std::get<State>(states_);
        }

    private:
        // Prepend a list of states with std::monostate. We want to avoid a situation
        // that State will be constructed by defaulted when created instance of this class.
        // 1. maybe we want to defer creation of State object
        // 2. first state does not necessarily have default constructor
        using states_variant_list = typename mpl::type_list_push_front<type_list, std::monostate>::result;

        // transform a type list to a corresponding variant
        using states_variant = typename mpl::type_list_rename<states_variant_list, std::variant>::result;

        states_variant states_;
    };

} // namespace fsm::detail
