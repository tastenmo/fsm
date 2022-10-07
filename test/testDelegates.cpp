/**
 * @file testDelegates.cpp
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2022-10-07
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <iostream>
#include <memory>

//#include <catch2/catch.hpp>
#include <catch2/catch_all.hpp>
#include <signal/delegate.h>

using namespace signal;

int delegate_function(const int &i)
{
    return i * i;
}

int curried_by_ref(const int &i, int j)
{
    return i + j;
}

int curried_by_ptr(const int *i, int j)
{
    return (*i) * j;
}

int non_const_reference(int &i)
{
    return i *= i;
}

int move_only_type(std::unique_ptr<int> ptr)
{
    return *ptr;
}

struct delegate_functor
{
    int operator()(int i)
    {
        return i + i;
    }

    int identity(int i) const
    {
        return i;
    }

    static const int static_value = 3;
    const int data_member = 42;
};

struct const_nonconst_noexcept
{
    void f()
    {
        ++cnt;
    }

    void g() noexcept
    {
        ++cnt;
    }

    void h() const
    {
        ++cnt;
    }

    void i() const noexcept
    {
        ++cnt;
    }

    int u{};
    const int v{};
    mutable int cnt{0};
};

TEST_CASE("DelegateFunctionalities", "[delegates]") {
    delegate<int(int)> ff_del;
    delegate<int(int)> mf_del;
    delegate<int(int)> lf_del;
    delegate_functor functor;

    REQUIRE_FALSE(ff_del);
    REQUIRE_FALSE(mf_del);
    REQUIRE(ff_del == mf_del);

    ff_del.connect<&delegate_function>();
    mf_del.connect<&delegate_functor::operator()>(functor);
    lf_del.connect([](const void *ptr, int value) { return static_cast<const delegate_functor *>(ptr)->identity(value); }, &functor);

    REQUIRE(ff_del);
    REQUIRE(mf_del);
    REQUIRE(lf_del);

    REQUIRE(ff_del(3) == 9);
    REQUIRE(mf_del(3) == 6);
    REQUIRE(lf_del(3) == 3);

    ff_del.reset();

    REQUIRE_FALSE(ff_del);

    REQUIRE(ff_del == delegate<int(int)>{});
    REQUIRE(mf_del != delegate<int(int)>{});
    REQUIRE(lf_del != delegate<int(int)>{});

    REQUIRE(ff_del != mf_del);
    REQUIRE(ff_del != lf_del);
    REQUIRE(mf_del != lf_del);

    mf_del.reset();

    REQUIRE_FALSE(ff_del);
    REQUIRE_FALSE(mf_del);
    REQUIRE(lf_del);

    REQUIRE(ff_del == delegate<int(int)>{});
    REQUIRE(mf_del == delegate<int(int)>{});
    REQUIRE(lf_del != delegate<int(int)>{});

    REQUIRE(ff_del == mf_del);
    REQUIRE(ff_del != lf_del);
    REQUIRE(mf_del != lf_del);
}

template< typename T>
struct Foo {
    size_t size() {
        return 0;
    }
};

TEMPLATE_PRODUCT_TEST_CASE("A Template product test case", "[template][product]", (std::vector, Foo), (int, float)) {
    TestType x;
    REQUIRE(x.size() == 0);
}