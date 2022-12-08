#pragma once

#include <functional>
#include <memory>

namespace escad {

template<
    typename Key,
    typename Type,
    typename = std::hash<Key>,
    typename = std::equal_to<Key>,
    typename = std::allocator<std::pair<const Key, Type>>>
class dense_map;

}