/**
 * @file symbol.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2024-02-05
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <bits/utility.h>
#include <type_traits>

#include <tuple>

#include "../base/type_traits.h"

namespace escad {
namespace new_fsm {

template <class... TSymbol> struct result_type {

  using symbol_list = mpl::type_list<TSymbol...>;

  // template <class... U> bool is(const U &...u) const {
  //   return for_each(u...);
  // }

  template <class... Args>
  constexpr bool operator()(const Args &...args) const {

    static_assert(sizeof...(Args) <= symbol_list::size,
                  "Number of arguments does not match number of symbols");

    if constexpr (sizeof...(Args) < symbol_list::size) {
      return false;
    } else {

      using idx = std::make_index_sequence<sizeof...(Args)>;

      return is(idx{}, args...);
    }
  }

  template <class T, T... ints, class... Args>
  constexpr bool is(const std::integer_sequence<T, ints...> &,
                    const Args &...args) const {
    return (... && mpl::type_list_element_t<ints, symbol_list>{}(args));
  }
};

} // namespace new_fsm
} // namespace escad