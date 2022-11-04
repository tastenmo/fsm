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

using namespace escad;

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

TEST_CASE("InvokeEmpty", "[delegates]") {
    delegate<int(int)> del;

    REQUIRE_FALSE(del);
    //ASSERT_DEATH(del(42), "");
    //ASSERT_DEATH(std::as_const(del)(42), "");
}

TEST_CASE("DataMembers", "[delegates]") {
    delegate<double()> delegate;
    delegate_functor functor;

    delegate.connect<&delegate_functor::data_member>(functor);

    REQUIRE(delegate() == 42);
}

TEST_CASE("Comparison", "[delegates]") {
    delegate<int(int)> lhs;
    delegate<int(int)> rhs;
    delegate_functor functor;
    delegate_functor other;
    const int value = 0;

    REQUIRE(lhs == delegate<int(int)>{});
    REQUIRE_FALSE(lhs != rhs);
    REQUIRE(lhs == rhs);
    
    lhs.connect<&delegate_function>();

    REQUIRE(lhs == delegate<int(int)>{connect_arg<&delegate_function>});
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);
    

    rhs.connect<&delegate_function>();

    REQUIRE(rhs == delegate<int(int)>{connect_arg<&delegate_function>});
    REQUIRE_FALSE(lhs != rhs);
    REQUIRE(lhs == rhs);
    
    lhs.connect<&curried_by_ref>(value);

    REQUIRE(lhs == (delegate<int(int)>{connect_arg<&curried_by_ref>, value}));
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);

    rhs.connect<&curried_by_ref>(value);

    REQUIRE(rhs == (delegate<int(int)>{connect_arg<&curried_by_ref>, value}));
    REQUIRE_FALSE(lhs != rhs);
    REQUIRE(lhs == rhs);

    lhs.connect<&curried_by_ptr>(&value);

    REQUIRE(lhs == (delegate<int(int)>{connect_arg<&curried_by_ptr>, &value}));
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);
   
    rhs.connect<&curried_by_ptr>(&value);

    REQUIRE(rhs == (delegate<int(int)>{connect_arg<&curried_by_ptr>, &value}));
    REQUIRE_FALSE(lhs != rhs);
    REQUIRE(lhs == rhs);
    
    lhs.connect<&delegate_functor::operator()>(functor);

    REQUIRE(lhs == (delegate<int(int)>{connect_arg<&delegate_functor::operator()>, functor}));
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);
 
    rhs.connect<&delegate_functor::operator()>(functor);

    REQUIRE(rhs == (delegate<int(int)>{connect_arg<&delegate_functor::operator()>, functor}));
    REQUIRE_FALSE(lhs != rhs);
    REQUIRE(lhs == rhs);
    
    lhs.connect<&delegate_functor::operator()>(other);

    REQUIRE(lhs == (delegate<int(int)>{connect_arg<&delegate_functor::operator()>, other}));
    REQUIRE_FALSE(lhs.data() == rhs.data());
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);
   
    lhs.connect([](const void *ptr, int val) { return static_cast<const delegate_functor *>(ptr)->identity(val) * val; }, &functor);

    //REQUIRE_FALSE(lhs == (delegate<int(int)>{[](const void *, int val) { return val + val; }, &functor}));
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);

    rhs.connect([](const void *ptr, int val) { return static_cast<const delegate_functor *>(ptr)->identity(val) + val; }, &functor);

    //REQUIRE_FALSE(rhs == (delegate<int(int)>{[](const void *, int val) { return val * val; }, &functor}));
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);
    

    lhs.reset();

    REQUIRE(lhs == (delegate<int(int)>{}));
    REQUIRE(lhs != rhs);
    REQUIRE_FALSE(lhs == rhs);
    

    rhs.reset();

    REQUIRE(rhs == (delegate<int(int)>{}));
    REQUIRE_FALSE(lhs != rhs);
    REQUIRE(lhs == rhs);
   
}

TEST_CASE("ConstNonConstNoExcept", "[delegates]") {
    delegate<void()> delegate;
    const_nonconst_noexcept functor;

    delegate.connect<&const_nonconst_noexcept::f>(functor);
    delegate();

    delegate.connect<&const_nonconst_noexcept::g>(functor);
    delegate();

    delegate.connect<&const_nonconst_noexcept::h>(functor);
    delegate();

    delegate.connect<&const_nonconst_noexcept::i>(functor);
    delegate();

    REQUIRE(functor.cnt == 4);
}

TEST_CASE("DeductionGuide", "[delegates]") {
    const_nonconst_noexcept functor;
    int value = 0;

    delegate func{connect_arg<&delegate_function>};
    delegate curried_func_with_ref{connect_arg<&curried_by_ref>, value};
    delegate curried_func_with_const_ref{connect_arg<&curried_by_ref>, std::as_const(value)};
    delegate curried_func_with_ptr{connect_arg<&curried_by_ptr>, &value};
    delegate curried_func_with_const_ptr{connect_arg<&curried_by_ptr>, &std::as_const(value)};
    delegate member_func_f{connect_arg<&const_nonconst_noexcept::f>, functor};
    delegate member_func_g{connect_arg<&const_nonconst_noexcept::g>, functor};
    delegate member_func_h{connect_arg<&const_nonconst_noexcept::h>, &functor};
    delegate member_func_h_const{connect_arg<&const_nonconst_noexcept::h>, &std::as_const(functor)};
    delegate member_func_i{connect_arg<&const_nonconst_noexcept::i>, functor};
    delegate member_func_i_const{connect_arg<&const_nonconst_noexcept::i>, std::as_const(functor)};
    delegate data_member_u{connect_arg<&const_nonconst_noexcept::u>, functor};
    delegate data_member_v{connect_arg<&const_nonconst_noexcept::v>, &functor};
    delegate data_member_v_const{connect_arg<&const_nonconst_noexcept::v>, &std::as_const(functor)};
    delegate lambda{+[](const void *, int) { return 0; }};

    STATIC_REQUIRE(std::is_same_v<typename decltype(func)::type, int(const int &)>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(curried_func_with_ref)::type, int(int)>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(curried_func_with_const_ref)::type, int(int)>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(curried_func_with_ptr)::type, int(int)>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(curried_func_with_const_ptr)::type, int(int)>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(member_func_f)::type, void()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(member_func_g)::type, void()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(member_func_h)::type, void()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(member_func_h_const)::type, void()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(member_func_i)::type, void()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(member_func_i_const)::type, void()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(data_member_u)::type, int()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(data_member_v)::type, const int()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(data_member_v_const)::type, const int()>);
    STATIC_REQUIRE(std::is_same_v<typename decltype(lambda)::type, int(int)>);

    REQUIRE(func);
    REQUIRE(curried_func_with_ref);
    REQUIRE(curried_func_with_const_ref);
    REQUIRE(curried_func_with_ptr);
    REQUIRE(curried_func_with_const_ptr);
    REQUIRE(member_func_f);
    REQUIRE(member_func_g);
    REQUIRE(member_func_h);
    REQUIRE(member_func_h_const);
    REQUIRE(member_func_i);
    REQUIRE(member_func_i_const);
    REQUIRE(data_member_u);
    REQUIRE(data_member_v);
    REQUIRE(data_member_v_const);
    REQUIRE(lambda);
}

TEST_CASE("ConstInstance", "[delegates]") {
    delegate<int(int)> delegate;
    const delegate_functor functor;

    REQUIRE_FALSE(delegate);

    delegate.connect<&delegate_functor::identity>(functor);

    REQUIRE(delegate);
    REQUIRE(delegate(3)== 3);

    delegate.reset();

    REQUIRE_FALSE(delegate);
    //ToDo check REQUIRE(delegate == delegate<int(int)>{});
}

TEST_CASE("NonConstReference", "[delegates]") {
    delegate<int(int &)> delegate;
    delegate.connect<&non_const_reference>();
    int value = 3;

    REQUIRE(delegate(value)== 9);
    REQUIRE(value== 9);
}

TEST_CASE("MoveOnlyType", "[delegates]") {
    delegate<int(std::unique_ptr<int>)> delegate;
    auto ptr = std::make_unique<int>(3);
    delegate.connect<&move_only_type>();

    REQUIRE(delegate(std::move(ptr))== 3);
    REQUIRE_FALSE(ptr);
}

TEST_CASE("CurriedFunction", "[delegates]") {
    delegate<int(int)> delegate;
    const auto value = 3;

    delegate.connect<&curried_by_ref>(value);

    REQUIRE(delegate);
    REQUIRE(delegate(1)== 4);

    delegate.connect<&curried_by_ptr>(&value);

    REQUIRE(delegate);
    REQUIRE(delegate(2)== 6);
}

TEST_CASE("Constructors", "[delegates]") {
    delegate_functor functor;
    const auto value = 2;

    delegate<int(int)> empty{};
    delegate<int(int)> func{connect_arg<&delegate_function>};
    delegate<int(int)> ref{connect_arg<&curried_by_ref>, value};
    delegate<int(int)> ptr{connect_arg<&curried_by_ptr>, &value};
    delegate<int(int)> member{connect_arg<&delegate_functor::operator()>, functor};

    REQUIRE_FALSE(empty);

    REQUIRE(func);
    REQUIRE(9== func(3));

    REQUIRE(ref);
    REQUIRE(5== ref(3));

    REQUIRE(ptr);
    REQUIRE(6== ptr(3));

    REQUIRE(member);
    REQUIRE(6== member(3));
}

TEST_CASE("VoidVsNonVoidReturnType", "[delegates]") {
    delegate_functor functor;

    delegate<void(int)> func{connect_arg<&delegate_function>};
    delegate<void(int)> member{connect_arg<&delegate_functor::operator()>, &functor};
    delegate<void(int)> cmember{connect_arg<&delegate_functor::identity>, &std::as_const(functor)};

    REQUIRE(func);
    REQUIRE(member);
    REQUIRE(cmember);
}

TEST_CASE("UnboundDataMember", "[delegates]") {
    delegate<int(const delegate_functor &)> delegate;
    delegate.connect<&delegate_functor::data_member>();
    delegate_functor functor;

    REQUIRE(delegate(functor)== 42);
}

TEST_CASE("UnboundMemberFunction", "[delegates]") {
    delegate<int(delegate_functor *, const int &i)> delegate;
    delegate.connect<&delegate_functor::operator()>();
    delegate_functor functor;

    REQUIRE(delegate(&functor, 3)== 6);
}

TEST_CASE("TheLessTheBetter", "[delegates]") {
    delegate<int(int, char)> bound;
    delegate<int(delegate_functor &, int, char)> unbound;
    delegate_functor functor;

    // int delegate_function(const int &);
    bound.connect<&delegate_function>();

    REQUIRE(bound(3, 'c')== 9);

    // int delegate_functor::operator()(int);
    bound.connect<&delegate_functor::operator()>(functor);

    REQUIRE(bound(3, 'c')== 6);

    // int delegate_functor::operator()(int);
    bound.connect<&delegate_functor::identity>(&functor);

    REQUIRE(bound(3, 'c')== 3);

    // int delegate_functor::operator()(int);
    unbound.connect<&delegate_functor::operator()>();

    REQUIRE(unbound(functor, 3, 'c')== 6);
}
