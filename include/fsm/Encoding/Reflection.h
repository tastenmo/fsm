/**
 * @file Reflection.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief Helpers for intrusive reflections
 * @version 0.1
 * @date 2022-03-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <charconv>

namespace reflection {

/**
 * @brief for loop over a sequence of type T with a lambada
 *
 * @tparam T
 * @tparam S
 * @tparam F
 * @param f
 */
template <typename T, T... S, typename F>
constexpr void for_sequence(std::integer_sequence<T, S...>, F &&f) {
  using unpack_t = int[];
  (void)unpack_t{(static_cast<void>(f(std::integral_constant<T, S>{})), 0)...,
                 0};
}

/**
 * @brief Reflective Property implementation of a public struct/class member
 *
 * @tparam Class Type uf the member
 * @tparam T Name or Alias of the member
 *
 * @todo use std::string_view instead of const char* for name
 * @todo add aliases for name
 */
template <typename Class, typename T> struct PropertyImpl {
  constexpr PropertyImpl(T Class::*aMember, const char *aName)
      : member{aMember}, name{aName} {}

  using Type = T;

  T Class::*member;
  const char *name;
};

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
  return PropertyImpl<Class, T>{member, name};
}

/**
 * @brief Default detection pattern template for std::from_chars
 *
 * @tparam T
 * @tparam typename
 */
template <typename T, typename = void>
struct is_from_chars_convertible : std::false_type {};

/**
 * @brief Specialized detection pattern for std::from_chars
 *
 * @tparam T
 */
template <typename T>
struct is_from_chars_convertible<
    T, std::void_t<decltype(std::from_chars(std::declval<const char *>(),
                                            std::declval<const char *>(),
                                            std::declval<T &>()))>>
    : std::true_type {};


template <typename... Ts> struct Overload : Ts... { using Ts::operator()...; };
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

} // namespace reflection