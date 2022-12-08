/**
 * @file traits.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "../access_context.h"

namespace escad::detail
{

template<class T, class E>
class can_handle_event
{
    template<class U>
    static auto test(int) -> decltype(std::declval<U>().handle(std::declval<E>()), std::true_type{});

    template<class>
    static std::false_type test(...);

public:
    static constexpr auto value = std::is_same_v<std::true_type, decltype(test<T>(0))>;
};

template<class T, class E, class C>
class can_handle_event_with_context
{
    static C& get_context() noexcept;

    template<class U>
    static auto test(int) -> decltype(std::declval<U>().handle(std::declval<E>(), get_context()), std::true_type{});

    template<class>
    static std::false_type test(...);

public:
    static constexpr auto value = std::is_same_v<std::true_type, decltype(test<T>(0))>;
};

} // namespace fsm::detail
