#pragma once

#include <tuple>
#include <variant>

#include <core/type_traits.h>

namespace spie::command {

namespace details {

template <class T> inline constexpr bool always_false_v = false;

template <class Command, class... Bindings> struct command_index_of;

template <bool Match, class Command, class First, class... Rest>
struct command_index_selector;

template <class Command, class First, class... Rest>
struct command_index_selector<true, Command, First, Rest...>
    : std::integral_constant<std::size_t, 0> {};

template <class Command, class First, class... Rest>
struct command_index_selector<false, Command, First, Rest...>
    : std::integral_constant<std::size_t,
                             1 + command_index_of<Command, Rest...>::value> {};

template <class Command, class First, class... Rest>
struct command_index_of<Command, First, Rest...>
    : command_index_selector<std::is_same_v<Command, typename First::command_type>,
                             Command, First, Rest...> {};

template <class Command> struct command_index_of<Command> {
  static_assert(always_false_v<Command>,
                "command type is not part of this registry");
};

template <class Command, class... Bindings> struct response_for;

template <bool Match, class Command, class First, class... Rest>
struct response_for_selector;

template <class Command, class First, class... Rest>
struct response_for_selector<true, Command, First, Rest...> {
  using type = typename First::response_type;
};

template <class Command, class First, class... Rest>
struct response_for_selector<false, Command, First, Rest...> {
  using type = typename response_for<Command, Rest...>::type;
};

template <class Command, class First, class... Rest>
struct response_for<Command, First, Rest...>
    : response_for_selector<std::is_same_v<Command, typename First::command_type>,
                            Command, First, Rest...> {};

template <class Command> struct response_for<Command> {
  static_assert(always_false_v<Command>,
                "response type for command is missing in this registry");
};

} // namespace details

template <class Command, class Response> struct binding {
  using command_type = Command;
  using response_type = Response;
};

template <class... Bindings> struct registry {
  using binding_tuple = std::tuple<Bindings...>;

  static constexpr std::size_t size = sizeof...(Bindings);

  template <std::size_t Index>
  using binding_at = std::tuple_element_t<Index, binding_tuple>;

  using command_variant =
      std::variant<std::monostate, typename Bindings::command_type...>;
  using response_variant =
      std::variant<std::monostate, typename Bindings::response_type...>;

  template <class Command>
  static constexpr std::size_t command_index =
      details::command_index_of<Command, Bindings...>::value;

  template <class Command>
  using response_for = typename details::response_for<Command, Bindings...>::type;

  template <class Func> static void for_each_binding(Func &&func) {
    mpl::for_sequence(std::make_index_sequence<size>{}, [&](auto i) {
      using current_binding = binding_at<i>;
      func(mpl::type_identity<current_binding>{});
    });
  }
};

} // namespace spie::command
