#pragma once

#include <reflect/struct_string.h>

namespace spie {

namespace reflection {

namespace details {
template <typename StructType>
using struct_string = ::spie::reflect::details::struct_string<StructType>;

} // namespace details

// unserialize function
template <typename T> std::optional<T> fromString(std::string_view vw) {
  return ::spie::reflect::fromString<T>(vw);
}

// unserialize function
template <typename T> std::string toString(const T &object) {
  return ::spie::reflect::toString<T>(object);
}

} // namespace reflection
} // namespace spie
