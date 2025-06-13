#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <base/compressed_pair.h>

struct empty_type {};

struct move_only_type {
    move_only_type()
        : value{new int{99}} {}

    move_only_type(int v)
        : value{new int{v}} {}

    ~move_only_type() {
        delete value;
    }

    move_only_type(const move_only_type &) = delete;
    move_only_type &operator=(const move_only_type &) = delete;

    move_only_type(move_only_type &&other) noexcept
        : value{std::exchange(other.value, nullptr)} {}

    move_only_type &operator=(move_only_type &&other) noexcept {
        delete value;
        value = std::exchange(other.value, nullptr);
        return *this;
    }

    int *value;
};

struct non_default_constructible {
    non_default_constructible(int v)
        : value{v} {}

    int value;
};

TEST_CASE("CompressedPair_Size", "[CompressedPair]") {
    struct local {
        int value;
        empty_type empty;
    };

    static_assert(sizeof(escad::compressed_pair<int, int>) == sizeof(int[2u]));
    static_assert(sizeof(escad::compressed_pair<empty_type, int>) == sizeof(int));
    static_assert(sizeof(escad::compressed_pair<int, empty_type>) == sizeof(int));
    static_assert(sizeof(escad::compressed_pair<int, empty_type>) < sizeof(local));
    static_assert(sizeof(escad::compressed_pair<int, empty_type>) < sizeof(std::pair<int, empty_type>));
}

TEST_CASE("CompressedPair_ConstructCopyMove", "[CompressedPair]") {
    static_assert(!std::is_default_constructible_v<escad::compressed_pair<non_default_constructible, empty_type>>);
    static_assert(std::is_default_constructible_v<escad::compressed_pair<move_only_type, empty_type>>);

    static_assert(std::is_copy_constructible_v<escad::compressed_pair<non_default_constructible, empty_type>>);
    static_assert(!std::is_copy_constructible_v<escad::compressed_pair<move_only_type, empty_type>>);
    static_assert(std::is_copy_assignable_v<escad::compressed_pair<non_default_constructible, empty_type>>);
    static_assert(!std::is_copy_assignable_v<escad::compressed_pair<move_only_type, empty_type>>);

    static_assert(std::is_move_constructible_v<escad::compressed_pair<move_only_type, empty_type>>);
    static_assert(std::is_move_assignable_v<escad::compressed_pair<move_only_type, empty_type>>);

    escad::compressed_pair copyable{non_default_constructible{42}, empty_type{}};
    auto by_copy{copyable};

    REQUIRE(by_copy.first().value == 42);

    by_copy.first().value = 3;
    copyable = by_copy;

    REQUIRE(copyable.first().value == 3);

    escad::compressed_pair<empty_type, move_only_type> movable{};
    auto by_move{std::move(movable)};

    REQUIRE(*by_move.second().value == 99);
    REQUIRE(movable.second().value == nullptr);

    *by_move.second().value = 3;
    movable = std::move(by_move);

    REQUIRE(*movable.second().value == 3);
    REQUIRE(by_move.second().value == nullptr);
}

TEST_CASE("CompressedPair_PiecewiseConstruct", "[CompressedPair]") {
    std::vector<int> vec{42};
    escad::compressed_pair<empty_type, empty_type> empty{std::piecewise_construct, std::make_tuple(), std::make_tuple()};
    escad::compressed_pair<std::vector<int>, std::size_t> pair{std::piecewise_construct, std::forward_as_tuple(std::move(vec)), std::make_tuple(sizeof(empty))};

    REQUIRE(pair.first().size() == 1u);
    REQUIRE(pair.second() == sizeof(empty));
    REQUIRE(vec.size() == 0u);
}

TEST_CASE("CompressedPair_DeductionGuide", "[CompressedPair]") {
    int value = 42;
    empty_type empty{};
    escad::compressed_pair pair{value, 3};

    static_assert(std::is_same_v<decltype(escad::compressed_pair{empty_type{}, empty}), escad::compressed_pair<empty_type, empty_type>>);

    STATIC_REQUIRE(std::is_same_v<decltype(pair), escad::compressed_pair<int, int>>);
    REQUIRE(pair.first() == 42);
    REQUIRE(pair.second() == 3);
}

TEST_CASE("CompressedPair_Getters", "[CompressedPair]") {
    escad::compressed_pair pair{3, empty_type{}};
    const auto &cpair = pair;

    static_assert(std::is_same_v<decltype(pair.first()), int &>);
    static_assert(std::is_same_v<decltype(pair.second()), empty_type &>);

    static_assert(std::is_same_v<decltype(cpair.first()), const int &>);
    static_assert(std::is_same_v<decltype(cpair.second()), const empty_type &>);

    REQUIRE(pair.first() == cpair.first());
    REQUIRE(&pair.second() == &cpair.second());
}

TEST_CASE("CompressedPair_Swap", "[CompressedPair]") {
    escad::compressed_pair pair{1, 2};
    escad::compressed_pair other{3, 4};

    swap(pair, other);

    REQUIRE(pair.first() == 3);
    REQUIRE(pair.second() == 4);
    REQUIRE(other.first() == 1);
    REQUIRE(other.second() == 2);

    pair.swap(other);

    REQUIRE(pair.first() == 1);
    REQUIRE(pair.second() == 2);
    REQUIRE(other.first() == 3);
    REQUIRE(other.second() == 4);
}

TEST_CASE("CompressedPair_Get", "[CompressedPair]") {
    escad::compressed_pair pair{1, 2};

    REQUIRE(pair.get<0>() == 1);
    REQUIRE(pair.get<1>() == 2);

    REQUIRE(&pair.get<0>() == &pair.first());
    REQUIRE(&pair.get<1>() == &pair.second());

    auto &&[first, second] = pair;

    REQUIRE(first == 1);
    REQUIRE(second == 2);

    first = 3;
    second = 4;

    REQUIRE(pair.first() == 3);
    REQUIRE(pair.second() == 4);

    auto &[cfirst, csecond] = std::as_const(pair);

    REQUIRE(cfirst == 3);
    REQUIRE(csecond == 4);

    static_assert(std::is_same_v<decltype(cfirst), const int>);
    static_assert(std::is_same_v<decltype(csecond), const int>);

    auto [tfirst, tsecond] = escad::compressed_pair{9, 99};

    REQUIRE(tfirst == 9);
    REQUIRE(tsecond == 99);

    static_assert(std::is_same_v<decltype(cfirst), const int>);
    static_assert(std::is_same_v<decltype(csecond), const int>);
}
