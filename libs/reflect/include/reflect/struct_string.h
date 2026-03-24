#pragma once

#include <charconv>
#include <optional>
#include <sstream>
#include <string_view>

#include <core/type_traits.h>
#include <reflect/property.h>

#include <ctre.hpp>

namespace spie::reflect {

namespace details {

template <typename StructType> struct struct_string {
  constexpr struct_string() = default;

  constexpr static auto nbProperties =
      std::tuple_size<decltype(StructType::properties)>::value;

  constexpr auto match(std::string_view sv) noexcept {
    return ctre::match<StructType::pattern>(sv);
  }

  std::string toString(const StructType &object) {
    std::stringstream ss;

    mpl::for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
      constexpr auto prop = std::get<i>(StructType::properties);
      ss << object.*(prop.member) << ";" << std::endl;
    });

    return ss.str();
  }

  std::optional<StructType> fromString(std::string_view vw) {
    StructType object{};

    if (auto m = match(vw)) {
      bool conversionOk = true;

      mpl::for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
        if (!conversionOk) {
          return;
        }

        constexpr auto prop = std::get<i>(StructType::properties);
        const std::string_view capture = m.template get<prop.name>().to_view();
        using objectType = typename decltype(prop)::Type;

        if constexpr (std::is_convertible_v<std::string_view, objectType>) {
          object.*(prop.member) = capture;
        } else if constexpr (std::is_same_v<objectType, std::string>) {
          object.*(prop.member) = std::string{capture};
        } else if constexpr (mpl::is_from_chars_convertible<objectType>::value) {
          auto [ptr, ec] = std::from_chars(capture.data(),
                                           capture.data() + capture.size(),
                                           object.*(prop.member));
          if (ec != std::errc{} || ptr != capture.data() + capture.size()) {
            conversionOk = false;
          }
        } else if constexpr (std::is_same_v<objectType, double>) {
          std::string str{capture};
          size_t read = 0;
          try {
            object.*(prop.member) = std::stod(str, &read);
            if (read != str.size()) {
              conversionOk = false;
            }
          } catch (...) {
            conversionOk = false;
          }
        } else if constexpr (std::is_same_v<objectType, float>) {
          std::string str{capture};
          size_t read = 0;
          try {
            object.*(prop.member) = std::stof(str, &read);
            if (read != str.size()) {
              conversionOk = false;
            }
          } catch (...) {
            conversionOk = false;
          }
        } else {
          static_assert(!sizeof(objectType), "no conversion to objectType");
        }
      });

      if (!conversionOk) {
        return std::nullopt;
      }

      return object;
    }

    return std::nullopt;
  }
};

} // namespace details

template <typename T> std::optional<T> fromString(std::string_view vw) {
  return details::struct_string<T>{}.fromString(vw);
}

template <typename T> std::string toString(const T &object) {
  return details::struct_string<T>{}.toString(object);
}

} // namespace spie::reflect
