#pragma once

#include <array>
#include <string_view>

#include <reflect_json/reflect_json.h>

#include "registry.h"

namespace spie::command::schema {

template <class T>
inline constexpr std::size_t property_count =
    std::tuple_size_v<decltype(T::properties)>;

template <class T, std::size_t... Index>
consteval auto property_names_impl(std::index_sequence<Index...>) {
  return std::array<std::string_view, sizeof...(Index)>{
      std::string_view{std::get<Index>(T::properties).name}...};
}

template <class T>
inline constexpr auto property_names =
    property_names_impl<T>(std::make_index_sequence<property_count<T>>{});

template <class Command, class Response> struct method_descriptor {
  using command_type = Command;
  using response_type = Response;

  static constexpr std::string_view name = Command::cmd;
  static constexpr auto input_names = property_names<Command>;
  static constexpr auto output_names = property_names<Response>;
};

template <class Registry, class Func> void for_each_method(Func &&func) {
  Registry::for_each_binding([&](auto binding_identity) {
    using binding_type = typename decltype(binding_identity)::type;
    using descriptor = method_descriptor<typename binding_type::command_type,
                                         typename binding_type::response_type>;
    func(mpl::type_identity<descriptor>{});
  });
}

} // namespace spie::command::schema