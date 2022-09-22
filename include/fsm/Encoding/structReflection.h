/**
 * @file structReflection.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2022-01-25
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <iostream>
#include <sstream>

#include "Reflection.h"

#include <ctre.hpp>

namespace reflection {


/**
 * @brief 
 * 
 * @tparam StructType 
 */
template <typename StructType> struct StructStringImpl {
  constexpr StructStringImpl() {}

  // number of properties
  constexpr static auto nbProperties =
      std::tuple_size<decltype(StructType::properties)>::value;

  constexpr auto match(std::string_view sv) noexcept {
    return ctre::match<StructType::pattern>(sv);
  }

  std::string toString(const StructType &object) {
    std::stringstream ss;

    // We iterate on the index sequence of size `nbProperties`
    for_sequence(std::make_index_sequence<nbProperties>{}, [&](auto i) {
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

      for_sequence(std::make_index_sequence<nbProperties>{}, [&, this](auto i) {
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

        // objectType is convertible by std::from_chars
        else if constexpr (is_from_chars_convertible<objectType>::value) {
          const auto last = capture.data() + capture.size();
          const auto res =
              std::from_chars(capture.data(), last, object.*(property.member));
          // check howto do this
          // if (res.ec == std::errc{} && res.ptr == last)
          //  return value;
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
    }
    else {
      return std::nullopt;
    }

  }
};

// unserialize function
template <typename T>
std::optional<T> fromString(std::string_view vw) {
  return StructStringImpl<T>{}.fromString(vw);
}

// unserialize function
template <typename T> std::string toString(const T &object) {
  return StructStringImpl<T>{}.toString(object);
}

} // namespace reflection
