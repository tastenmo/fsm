#pragma once

#include <reflect/property.h>

namespace spie {

namespace details {
template <typename Class, typename T, typename Name>
using property = ::spie::reflect::details::property<Class, T, Name>;

} // namespace details

/**
 * @brief Reflective property
 *
 * One could overload this function to accept both a getter and a setter instead
 * of a member.
 *
 * @tparam Class
 * @tparam T
 * @param member
 * @param name
 * @return constexpr auto
 *
 * @todo use std::string_view instead of const char* for name
 * @todo add aliases for name
 */
template <typename Class, typename T, typename Name>
constexpr auto property(T Class::*member, Name name) {
   return ::spie::reflect::property(member, name);
}

using properties = ::spie::reflect::properties;

} // namespace spie
