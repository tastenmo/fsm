#pragma once

namespace escad {

namespace reflection {

namespace details {

/**
 * @brief Reflective Property implementation of a public struct/class member
 *
 * @tparam Class Type uf the member
 * @tparam T Name or Alias of the member
 *
 * @todo use std::string_view instead of const char* for name
 * @todo add aliases for name
 */
template <typename Class, typename T> struct property {
  constexpr property(T Class::*aMember, const char *aName)
      : member{aMember}, name{aName} {}

  using Type = T;

  T Class::*member;
  const char *name;
};

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
template <typename Class, typename T>
constexpr auto property(T Class::*member, const char *name) {
  return details::property<Class, T>{member, name};
}

} // namespace reflection
} // namespace escad
