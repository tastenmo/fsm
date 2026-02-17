#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace spie {

/*! @brief Alias declaration for type identifiers. */
using id_type = std::uint32_t;

template <typename Key, typename Type, typename = std::hash<Key>,
          typename = std::equal_to<Key>,
          typename = std::allocator<std::pair<const Key, Type>>>
class dense_map;

} // namespace spie