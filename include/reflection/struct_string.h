#pragma once

#include <charconv>
#include <iostream>
#include <sstream>

#include "base/type_traits.h"
#include "property.h"

#include <ctre.hpp>

namespace escad {

namespace reflection {

namespace details {

/**
 * @brief
 *
 * @tparam StructType
 */
template <typename StructType> struct struct_string {
  constexpr struct_string() {}

  // number of properties
  constexpr static auto nbProperties =
      std::tuple_size<decltype(StructType::properties)>::value;

  constexpr auto match(std::string_view sv) noexcept {
    return ctre::match<StructType::pattern>(sv);
  }

  std::string toString(const StructType &object) {
    std::stringstream ss;

    // We iterate on the index sequence of size `nbProperties`
    mpl::for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
      // get the property
      constexpr auto property = std::get<i>(StructType::properties);

      // set the value to the member
      ss << property.name << " = " << object.*(property.member) << ";"
         << std::endl;
    });

    return ss.str();
  }

  std::optional<StructType> fromString(std::string_view vw) {
    StructType object;

    std::string_view capture;

    if (auto m = match(vw)) {

      mpl::for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
        // get the property
        constexpr auto property = std::get<i>(StructType::properties);

        // get the capture view
        capture = m.template get<i + 1>().to_view();

        // get the type of the property
        using objectType = typename decltype(property)::Type;

        // objectType is std::string or std::string_view
        if constexpr (std::is_convertible_v<std::string_view, objectType>) {
          object.*(property.member) = capture;
        }

        else if constexpr (std::is_same_v<objectType, std::string>) {
          std::string str{capture};
          object.*(property.member) = str;
        }

        // objectType is convertible by std::from_chars
        else if constexpr (mpl::is_from_chars_convertible<objectType>::value) {
          [[maybe_unused]]auto [ptr, ec] =
              std::from_chars(capture.data(), capture.data() + capture.size(), object.*(property.member));

        }

        else if constexpr (std::is_same_v<objectType, double>) {
          std::string str{capture};
          size_t read = 0;
          object.*(property.member) = std::stod(str, &read);
        } else if constexpr (std::is_same_v<objectType, float>) {
          std::string str{capture};
          size_t read = 0;
          object.*(property.member) = std::stof(str, &read);
        }

        else
          static_assert(!sizeof(objectType), "no conversion to ObjectType");
      });

      return object;
    } else {
      return std::nullopt;
    }
  }
};

} // namespace details

// unserialize function
template <typename T> std::optional<T> fromString(std::string_view vw) {
  return details::struct_string<T>{}.fromString(vw);
}

// unserialize function
template <typename T> std::string toString(const T &object) {
  return details::struct_string<T>{}.toString(object);
}

} // namespace reflection
} // namespace escad
