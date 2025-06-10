/**
 * @file access_context.h
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

#include "../base/type_traits.h"

namespace escad
{

template<class C, class... D>
struct access_context final {
    using access_context_type = mpl::type_list<C, D...>;

    template<
        class T,
        class = std::enable_if_t<T::template has<C> && (T::template has<D> && ...)>
    >
    access_context(T& tuple_like) noexcept
        : ctx_ {tuple_like.template get<C>(), tuple_like.template get<D>()...}
    {
    }

    template<class T>
    T& get_context() noexcept {
        return std::get<T &>(ctx_);
    }

    template<class T>
    T const& get_context() const noexcept {
        return std::get<T &>(ctx_);
    }

private:
    std::tuple<C &, D &...> ctx_;
};

/**
 * @brief Brings the context into the State automatically.
 */
template<class C>
struct access_context<C> final {
    using access_context_type = C;

    access_context(C& c) noexcept
        : ctx_ {c}
    {
    }

    template<
        class U,
        class = std::enable_if_t<U::template has<C>>
    >
    access_context(U& tuple_like) noexcept
        : ctx_ {tuple_like.template get<C>()}
    {}

    /**
     * @brief Retrieve a context.
     *
     * @warning Calling this method in constructor is undefined behavior (crash in practice).
     *          If you need to access the Context in the constructor, get it via State's ctor parameter.
     */
    C& get_context() noexcept {
        return ctx_;
    }

    C const& get_context() const noexcept {
        return ctx_;
    }

private:
    C& ctx_;
};

} // namespace fsm
