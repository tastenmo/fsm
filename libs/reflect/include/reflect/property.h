#pragma once

#include <core/type_traits.h>
#include <tuple>

namespace spie::reflect {

namespace details {

/**
 * @brief Intrusive property descriptor for a public struct/class member.
 */
template <typename Class, typename T, typename Name> struct property {
  constexpr property(T Class::*aMember, Name aName)
      : member{aMember}, name{aName} {}

  using Type = T;

  T Class::*member;
  Name name;
};

} // namespace details

template <typename Class, typename T, typename Name>
constexpr auto property(T Class::*member, Name name) {
  return details::property<Class, T, Name>{member, name};
}

struct properties {

  using prop_list = mpl::value_list<>;

  template <class... Ts> constexpr properties(const Ts... ctxs) noexcept {
    prop_list{} + mpl::value_list<ctxs...>{};
  }

  template <class... Ts>
  static constexpr auto add(const Ts &...ctxs) noexcept {
    prop_list{} + mpl::value_list<ctxs...>{};
    return std::make_tuple(prop_list{});
  }

  static constexpr auto properties_ = std::make_tuple(prop_list{});
};

} // namespace spie::reflect
