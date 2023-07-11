#pragma once

#include "base/type_traits.h"
#include <tuple>

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
template <typename Class, typename T, typename Name> struct property {
  constexpr property(T Class::*aMember, Name aName)
      : member{aMember}, name{aName} {}

  using Type = T;

  T Class::*member;
  Name name;
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
template <typename Class, typename T, typename Name>
constexpr auto property(T Class::*member, Name name) {
  return details::property<Class, T, Name>{member, name};
}


struct properties {

  using prop_list = mpl::value_list<>;

  template <class... Ts>
  constexpr properties(const Ts ...ctxs) noexcept {
    prop_list{} + mpl::value_list<ctxs...>{};
    
    //properties_ = std::make_tuple(ctxs...);
  }

  template <class... Ts>
  static constexpr auto add(const Ts& ...ctxs) noexcept {
    prop_list{} + mpl::value_list<ctxs...>{};

    return std::make_tuple(prop_list{});
    
    //properties_ = std::make_tuple(ctxs...);
  }

  static constexpr auto properties_ = std::make_tuple(prop_list{});

  //template <std::size_t U> constexpr auto get() noexcept {
  //  return std::get<U>(properties_);
  //}
};

} // namespace reflection
} // namespace escad
