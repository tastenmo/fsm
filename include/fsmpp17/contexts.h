/**
 * @file contexts.h
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

namespace escad
{

/**
 * A set of references to different context types.
 *
 * If there are multiple contexts used by different states this set will be used
 * to provide a proper context to a state based on its constructor argument type.
 *
 **/
template<class... T> struct contexts {
    /**
     * Capture different Context types as references.
     **/
    explicit contexts(T&... ctxs) noexcept
        : contexts_ {ctxs...}
    {
    }

    /**
     * Get context by type.
     **/
    template<class U>
    auto& get() noexcept {
        return std::get<std::add_lvalue_reference_t<U>>(contexts_);
    }

    /**
     * Get context by type.
     **/
    template<class U>
    auto const& get() const noexcept {
        return std::get<std::add_lvalue_reference_t<U>>(contexts_);
    }

    template<class U>
    static auto constexpr has = mpl::type_list_contains_v<mpl::type_list<T...>, U>;

private:
    std::tuple<T&...> contexts_;
};

} // namespace fsm
