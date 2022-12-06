#include <functional>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <signal/emitter.h>

#define ASSERT_EQ(EXPR1, EXPR2) REQUIRE(EXPR1 == EXPR2)
#define ASSERT_NE(EXPR1, EXPR2) REQUIRE_FALSE(EXPR1 == EXPR2)
#define ASSERT_TRUE(EXPR) REQUIRE(EXPR) 
#define ASSERT_FALSE(EXPR) REQUIRE_FALSE(EXPR) 


#define ASSERT_LT(EXPR1, EXPR2) REQUIRE(EXPR1 < EXPR2)
#define ASSERT_LE(EXPR1, EXPR2) REQUIRE(EXPR1 <= EXPR2)
#define ASSERT_GT(EXPR1, EXPR2) REQUIRE(EXPR1 > EXPR2)
#define ASSERT_GE(EXPR1, EXPR2) REQUIRE(EXPR1 >= EXPR2)

struct test_emitter: signal::emitter<test_emitter> {
    using signal::emitter<test_emitter>::emitter;
};

struct foo_event {
    int i;
};

struct bar_event {};
struct quux_event {};

TEST_CASE("Emitter_Move", "[Emitter]") {
    test_emitter emitter;
    emitter.on<foo_event>([](auto &, const auto &) {});

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<foo_event>());

    test_emitter other{std::move(emitter)};

    ASSERT_FALSE(other.empty());
    ASSERT_TRUE(other.contains<foo_event>());
    ASSERT_TRUE(emitter.empty());

    emitter = std::move(other);

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<foo_event>());
    ASSERT_TRUE(other.empty());
}

TEST_CASE("Emitter_Swap", "[Emitter]") {
    test_emitter emitter;
    test_emitter other;
    int value{};

    emitter.on<foo_event>([&value](auto &event, const auto &) {
        value = event.i;
    });

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(other.empty());

    emitter.swap(other);
    emitter.publish(foo_event{42});

    ASSERT_EQ(value, 0);
    ASSERT_TRUE(emitter.empty());
    ASSERT_FALSE(other.empty());

    other.publish(foo_event{42});

    ASSERT_EQ(value, 42);
}

TEST_CASE("Emitter_Clear", "[Emitter]") {
    test_emitter emitter;

    ASSERT_TRUE(emitter.empty());

    emitter.on<foo_event>([](auto &, const auto &) {});
    emitter.on<quux_event>([](const auto &, const auto &) {});

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<foo_event>());
    ASSERT_TRUE(emitter.contains<quux_event>());
    ASSERT_FALSE(emitter.contains<bar_event>());

    emitter.erase<bar_event>();

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<foo_event>());
    ASSERT_TRUE(emitter.contains<quux_event>());
    ASSERT_FALSE(emitter.contains<bar_event>());

    emitter.erase<foo_event>();

    ASSERT_FALSE(emitter.empty());
    ASSERT_FALSE(emitter.contains<foo_event>());
    ASSERT_TRUE(emitter.contains<quux_event>());
    ASSERT_FALSE(emitter.contains<bar_event>());

    emitter.on<foo_event>([](auto &, const auto &) {});
    emitter.on<bar_event>([](const auto &, const auto &) {});

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<foo_event>());
    ASSERT_TRUE(emitter.contains<quux_event>());
    ASSERT_TRUE(emitter.contains<bar_event>());

    emitter.clear();

    ASSERT_TRUE(emitter.empty());
    ASSERT_FALSE(emitter.contains<foo_event>());
    ASSERT_FALSE(emitter.contains<bar_event>());
}

TEST_CASE("Emitter_ClearFromCallback", "[Emitter]") {
    test_emitter emitter;

    ASSERT_TRUE(emitter.empty());

    emitter.on<foo_event>([](auto &, auto &owner) {
        owner.template on<foo_event>([](auto &, auto &) {});
        owner.template erase<foo_event>();
    });

    emitter.on<bar_event>([](const auto &, auto &owner) {
        owner.template on<bar_event>([](const auto &, auto &) {});
        owner.template erase<bar_event>();
    });

    ASSERT_FALSE(emitter.empty());

    emitter.publish(foo_event{});
    emitter.publish(bar_event{});

    ASSERT_TRUE(emitter.empty());
}

TEST_CASE("Emitter_On", "[Emitter]") {
    test_emitter emitter;
    int value{};

    emitter.on<foo_event>([&value](auto &event, const auto &) {
        value = event.i;
    });

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<foo_event>());
    ASSERT_EQ(value, 0);

    emitter.publish(foo_event{42});

    ASSERT_EQ(value, 42);
}

TEST_CASE("Emitter_OnAndErase", "[Emitter]" ) {
    test_emitter emitter;
    std::function<void(bar_event &, test_emitter &)> func{};

    emitter.on(func);

    ASSERT_FALSE(emitter.empty());
    ASSERT_TRUE(emitter.contains<bar_event>());

    emitter.erase<bar_event>();

    ASSERT_TRUE(emitter.empty());
    ASSERT_FALSE(emitter.contains<bar_event>());
}

TEST_CASE("Emitter_CustomAllocator", "[Emitter]") {
    std::allocator<void> allocator;
    test_emitter emitter{allocator};

    ASSERT_EQ(emitter.get_allocator(), allocator);
    ASSERT_FALSE(emitter.get_allocator() != allocator);

    emitter.on<foo_event>([](auto &, const auto &) {});
    decltype(emitter) other{std::move(emitter), allocator};

    ASSERT_TRUE(emitter.empty());
    ASSERT_FALSE(other.empty());
}
