/**
 * @file testSignalSlot.cpp
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>
#include <memory>

//#include <catch2/catch.hpp>
#include <catch2/catch_all.hpp>
#include <signal/signal.h>


struct sigh_listener {
    static void f(int &v) {
        v = 42;
    }

    bool g(int) {
        k = !k;
        return true;
    }

    bool h(const int &) {
        return k;
    }

    // useless definition just because msvc does weird things if both are empty
    void l() {
        k = true && k;
    }

    bool k{false};
};

struct before_after {
    void add(int v) {
        value += v;
    }

    void mul(int v) {
        value *= v;
    }

    static void static_add(int v) {
        before_after::value += v;
    }

    static void static_mul(before_after &instance, int v) {
        instance.value *= v;
    }

    static inline int value{};
};

//struct SigH: ::testing::Test {
//    void SetUp() override {
//        before_after::value = 0;
//    }
//};

struct const_nonconst_noexcept {
    void f() {
        ++cnt;
    }

    void g() noexcept {
        ++cnt;
    }

    void h() const {
        ++cnt;
    }

    void i() const noexcept {
        ++cnt;
    }

    mutable int cnt{0};
};

TEST_CASE("SignalSlot_Lifetime", "[SignalSlot]") {
    using sig = escad::signal<void(void)>;

    REQUIRE_NOTHROW(sig{});

    sig src{}, other{};

    REQUIRE_NOTHROW(sig{src});
    REQUIRE_NOTHROW(sig{std::move(other)});
    REQUIRE_NOTHROW(src = other);
    REQUIRE_NOTHROW(src = std::move(other));

    REQUIRE_NOTHROW(delete new sig{});
}

TEST_CASE("SignalSlot_Clear", "[SignalSlot]") {
    escad::signal<void(int &)> sigh;
    escad::slot slot{sigh};

    slot.connect<&sigh_listener::f>();

    REQUIRE_FALSE(slot.empty());
    REQUIRE_FALSE(sigh.empty());

    slot.disconnect(static_cast<const void *>(nullptr));

    REQUIRE_FALSE(slot.empty());
    REQUIRE_FALSE(sigh.empty());

    slot.disconnect();

    REQUIRE(slot.empty());
    REQUIRE(sigh.empty());
}

TEST_CASE("SignalSlot_Swap", "[SignalSlot]") {
    escad::signal<void(int &)> sigh1;
    escad::signal<void(int &)> sigh2;
    escad::slot sink1{sigh1};
    escad::slot sink2{sigh2};

    sink1.connect<&sigh_listener::f>();

    REQUIRE_FALSE(sink1.empty());
    REQUIRE(sink2.empty());

    REQUIRE_FALSE(sigh1.empty());
    REQUIRE(sigh2.empty());

    sigh1.swap(sigh2);

    REQUIRE(sink1.empty());
    REQUIRE_FALSE(sink2.empty());

    REQUIRE(sigh1.empty());
    REQUIRE_FALSE(sigh2.empty());
}

TEST_CASE("SignalSlot_Functions", "[SignalSlot]") {
    escad::signal<void(int &)> sigh;
    escad::slot sink{sigh};
    int v = 0;

    sink.connect<&sigh_listener::f>();
    sigh.publish(v);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(1u == sigh.size());
    REQUIRE(42 == v);

    v = 0;
    sink.disconnect<&sigh_listener::f>();
    sigh.publish(v);

    REQUIRE(sigh.empty());
    REQUIRE(0u == sigh.size());
    REQUIRE(v == 0);
}

TEST_CASE("SignalSlot_FunctionsWithPayload", "[SignalSlot]") {
    escad::signal<void()> sigh;
    escad::slot sink{sigh};
    int v = 0;

    sink.connect<&sigh_listener::f>(v);
    sigh.publish();

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(1u == sigh.size());
    REQUIRE(42 == v);

    v = 0;
    sink.disconnect<&sigh_listener::f>(v);
    sigh.publish();

    REQUIRE(sigh.empty());
    REQUIRE(0u == sigh.size());
    REQUIRE(v == 0);

    sink.connect<&sigh_listener::f>(v);
    sink.disconnect(v);
    sigh.publish();

    REQUIRE(v == 0);
}

TEST_CASE("SignalSlot_Members", "[SignalSlot]") {
    sigh_listener l1, l2;
    escad::signal<bool(int)> sigh;
    escad::slot sink{sigh};

    sink.connect<&sigh_listener::g>(l1);
    sigh.publish(42);

    REQUIRE(l1.k);
    REQUIRE_FALSE(sigh.empty());
    REQUIRE(1u == sigh.size());

    sink.disconnect<&sigh_listener::g>(l1);
    sigh.publish(42);

    REQUIRE(l1.k);
    REQUIRE(sigh.empty());
    REQUIRE(0u == sigh.size());

    sink.connect<&sigh_listener::g>(&l1);
    sink.connect<&sigh_listener::h>(l2);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(2u == sigh.size());

    sink.disconnect(static_cast<const void *>(nullptr));

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(2u == sigh.size());

    sink.disconnect(&l1);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(1u == sigh.size());
}

TEST_CASE("SignalSlot_Collector", "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<bool(int)> sigh;
    escad::slot sink{sigh};
    int cnt = 0;

    sink.connect<&sigh_listener::g>(&listener);
    sink.connect<&sigh_listener::h>(listener);

    auto no_return = [&listener, &cnt](bool value) {
        REQUIRE(value);
        listener.k = true;
        ++cnt;
    };

    listener.k = true;
    sigh.collect(std::move(no_return), 42);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(cnt == 2);

    auto bool_return = [&cnt](bool value) {
        // gtest and its macro hell are sometimes really annoying...
        [](auto v) { REQUIRE(v); }(value);
        ++cnt;
        return true;
    };

    cnt = 0;
    sigh.collect(std::move(bool_return), 42);

    REQUIRE(cnt == 1);
}

TEST_CASE("SignalSlot_CollectorVoid" , "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    int cnt = 0;

    sink.connect<&sigh_listener::g>(&listener);
    sink.connect<&sigh_listener::h>(listener);
    sigh.collect([&cnt]() { ++cnt; }, 42);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(cnt == 2);

    auto test = [&cnt]() {
        ++cnt;
        return true;
    };

    cnt = 0;
    sigh.collect(std::move(test), 42);

    REQUIRE(cnt == 1);
}

TEST_CASE("SignalSlot_Connection", "[SignalSlot]") {
    escad::signal<void(int &)> sigh;
    escad::slot sink{sigh};
    int v = 0;

    auto conn = sink.connect<&sigh_listener::f>();
    sigh.publish(v);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(conn);
    REQUIRE(42 == v);

    v = 0;
    conn.release();
    sigh.publish(v);

    REQUIRE(sigh.empty());
    REQUIRE_FALSE(conn);
    REQUIRE(0 == v);
}

TEST_CASE("SignalSlot_ScopedConnection", "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};

    {
        REQUIRE_FALSE(listener.k);

        escad::scoped_connection conn = sink.connect<&sigh_listener::g>(listener);
        sigh.publish(42);

        REQUIRE_FALSE(sigh.empty());
        REQUIRE(listener.k);
        REQUIRE(conn);
    }

    sigh.publish(42);

    REQUIRE(sigh.empty());
    REQUIRE(listener.k);
}

TEST_CASE("SignalSlot_ScopedConnectionMove", "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};

    escad::scoped_connection outer{sink.connect<&sigh_listener::g>(listener)};

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(outer);

    {
        escad::scoped_connection inner{std::move(outer)};

        REQUIRE_FALSE(listener.k);
        REQUIRE_FALSE(outer);
        REQUIRE(inner);

        sigh.publish(42);

        REQUIRE(listener.k);
    }

    REQUIRE(sigh.empty());

    outer = sink.connect<&sigh_listener::g>(listener);

    REQUIRE_FALSE(sigh.empty());
    REQUIRE(outer);

    {
        escad::scoped_connection inner{};

        REQUIRE(listener.k);
        REQUIRE(outer);
        REQUIRE_FALSE(inner);

        inner = std::move(outer);

        REQUIRE_FALSE(outer);
        REQUIRE(inner);

        sigh.publish(42);

        REQUIRE_FALSE(listener.k);
    }

    REQUIRE(sigh.empty());
}

TEST_CASE("SignalSlot_ScopedConnectionConstructorsAndOperators", "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};

    {
        escad::scoped_connection inner{};

        REQUIRE(sigh.empty());
        REQUIRE_FALSE(listener.k);
        REQUIRE_FALSE(inner);

        inner = sink.connect<&sigh_listener::g>(listener);
        sigh.publish(42);

        REQUIRE_FALSE(sigh.empty());
        REQUIRE(listener.k);
        REQUIRE(inner);

        inner.release();

        REQUIRE(sigh.empty());
        REQUIRE_FALSE(inner);

        auto basic = sink.connect<&sigh_listener::g>(listener);
        inner = std::as_const(basic);
        sigh.publish(42);

        REQUIRE_FALSE(sigh.empty());
        REQUIRE_FALSE(listener.k);
        REQUIRE(inner);
    }

    sigh.publish(42);

    REQUIRE(sigh.empty());
    REQUIRE_FALSE(listener.k);
}

TEST_CASE("SignalSlot_ConstNonConstNoExcept", "[SignalSlot]") {
    escad::signal<void()> sigh;
    escad::slot sink{sigh};
    const_nonconst_noexcept functor;
    const const_nonconst_noexcept cfunctor;

    sink.connect<&const_nonconst_noexcept::f>(functor);
    sink.connect<&const_nonconst_noexcept::g>(&functor);
    sink.connect<&const_nonconst_noexcept::h>(cfunctor);
    sink.connect<&const_nonconst_noexcept::i>(&cfunctor);
    sigh.publish();

    REQUIRE(functor.cnt == 2);
    REQUIRE(cfunctor.cnt == 2);

    sink.disconnect<&const_nonconst_noexcept::f>(functor);
    sink.disconnect<&const_nonconst_noexcept::g>(&functor);
    sink.disconnect<&const_nonconst_noexcept::h>(cfunctor);
    sink.disconnect<&const_nonconst_noexcept::i>(&cfunctor);
    sigh.publish();

    REQUIRE(functor.cnt == 2);
    REQUIRE(cfunctor.cnt == 2);
}

TEST_CASE("SignalSlot_BeforeFunction", "[SignalSlot]") {
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    before_after functor;

    sink.connect<&before_after::add>(functor);
    sink.connect<&before_after::static_add>();
    sink.before<&before_after::static_add>().connect<&before_after::mul>(functor);
    sigh.publish(2);

    REQUIRE(functor.value == 6);
}

TEST_CASE("SignalSlot_BeforeMemberFunction", "[SignalSlot]") {
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    before_after functor;

    sink.connect<&before_after::static_add>();
    sink.connect<&before_after::add>(functor);
    sink.before<&before_after::add>(functor).connect<&before_after::mul>(functor);
    sigh.publish(2);

    REQUIRE(functor.value == 6);
}

TEST_CASE("SignalSlot_BeforeFunctionWithPayload", "[SignalSlot]") {
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    before_after functor;

    sink.connect<&before_after::static_add>();
    sink.connect<&before_after::static_mul>(functor);
    sink.before<&before_after::static_mul>(functor).connect<&before_after::add>(functor);
    sigh.publish(2);

    REQUIRE(functor.value == 8);
}

TEST_CASE("SignalSlot_BeforeInstanceOrPayload", "[SignalSlot]") {
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    before_after functor;

    sink.connect<&before_after::static_mul>(functor);
    sink.connect<&before_after::add>(functor);
    sink.before(functor).connect<&before_after::static_add>();
    sigh.publish(2);

    REQUIRE(functor.value == 6);
}

TEST_CASE("SignalSlot_BeforeAnythingElse", "[SignalSlot]") {
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    before_after functor;

    sink.connect<&before_after::add>(functor);
    sink.before().connect<&before_after::mul>(functor);
    sigh.publish(2);

    REQUIRE(functor.value == 2);
}

TEST_CASE("SignalSlot_BeforeListenerNotPresent", "[SignalSlot]") {
    escad::signal<void(int)> sigh;
    escad::slot sink{sigh};
    before_after functor;

    sink.connect<&before_after::mul>(functor);
    sink.before<&before_after::add>(&functor).connect<&before_after::add>(functor);
    sigh.publish(2);

    REQUIRE(functor.value == 2);
}

TEST_CASE("SignalSlot_UnboundDataMember", "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<bool &(sigh_listener &)> sigh;
    escad::slot sink{sigh};

    REQUIRE_FALSE(listener.k);

    sink.connect<&sigh_listener::k>();
    sigh.collect([](bool &value) { value = !value; }, listener);

    REQUIRE(listener.k);
}

TEST_CASE("SignalSlot_UnboundMemberFunction", "[SignalSlot]") {
    sigh_listener listener;
    escad::signal<void(sigh_listener *, int)> sigh;
    escad::slot sink{sigh};

    REQUIRE_FALSE(listener.k);

    sink.connect<&sigh_listener::g>();
    sigh.publish(&listener, 42);

    REQUIRE(listener.k);
}

TEST_CASE("SignalSlot_CustomAllocator", "[SignalSlot]") {
    std::allocator<void (*)(int)> allocator;
    escad::signal<void(int), decltype(allocator)> sigh{allocator};

    REQUIRE(sigh.get_allocator() == allocator);
    REQUIRE_FALSE(sigh.get_allocator() != allocator);
    REQUIRE(sigh.empty());

    escad::slot sink{sigh};
    sigh_listener listener;
    sink.template connect<&sigh_listener::g>(listener);

    decltype(sigh) copy{sigh, allocator};
    sink.disconnect(listener);

    REQUIRE(sigh.empty());
    REQUIRE_FALSE(copy.empty());

    sigh = copy;

    REQUIRE_FALSE(sigh.empty());
    REQUIRE_FALSE(copy.empty());

    decltype(sigh) move{std::move(copy), allocator};

    REQUIRE(copy.empty());
    REQUIRE_FALSE(move.empty());

    sink = escad::slot{move};
    sink.disconnect(&listener);

    REQUIRE(copy.empty());
    REQUIRE(move.empty());

    sink.template connect<&sigh_listener::g>(listener);
    copy.swap(move);

    REQUIRE_FALSE(copy.empty());
    REQUIRE(move.empty());

    sink = escad::slot{copy};
    sink.disconnect();

    REQUIRE(copy.empty());
    REQUIRE(move.empty());
}

