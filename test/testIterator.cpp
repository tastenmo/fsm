#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <base/iterator.h>

struct clazz {
    int value{0};
};

TEST_CASE("Iterator_InputIteratorPointer", "[Iterator]") {
    clazz instance{};
    base::input_iterator_pointer ptr{std::move(instance)};
    ptr->value = 42;

    REQUIRE(instance.value == 0);
    REQUIRE(ptr->value == 42);
    REQUIRE(ptr->value == (*ptr).value);
    REQUIRE(ptr.operator->() == &ptr.operator*());
}

TEST_CASE("Iterator_IotaIterator", "[Iterator]") {
    base::iota_iterator<std::size_t> first{};
    const base::iota_iterator<std::size_t> last{2u};

    REQUIRE_FALSE(first == last);
    REQUIRE(first != last);

    REQUIRE(*first++ == 0u);
    REQUIRE(*first == 1u);
    REQUIRE(*++first == *last);
    REQUIRE(*first == 2u);
}

TEST_CASE("Iterator_IterableAdaptor", "[Iterator]") {
    std::vector<int> vec{1, 2};
    base::iterable_adaptor iterable{vec.begin(), vec.end()};
    decltype(iterable) other{};

    REQUIRE_NOTHROW(other = iterable);
    REQUIRE_NOTHROW(std::swap(other, iterable));

    REQUIRE(iterable.begin()== vec.begin());
    REQUIRE(iterable.end() == vec.end());

    REQUIRE(*iterable.cbegin() == 1);
    REQUIRE(*++iterable.cbegin() == 2);
    REQUIRE(++iterable.cbegin() == --iterable.end());

    for(auto value: base::iterable_adaptor<const int *, const void *>{vec.data(), vec.data() + 1u}) {
        REQUIRE(value == 1);
    }
}
