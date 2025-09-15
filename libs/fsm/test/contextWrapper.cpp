#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "core/utils.h"

#include <fsm/context.h>


using namespace escad::new_fsm;



TEST_CASE("lvalue reference context", "[new_fsm]") {

  struct Context {
    Context(int v) : value_(v), is_valid_(false) {}
    int value_;
    bool is_valid_;
  };

    Context ctx(42);

  ContextWrapper<Context> ctx_(ctx);

  
    REQUIRE(std::is_same_v<decltype(ctx), Context>);

    REQUIRE(std::is_same_v<decltype(ctx_.get()), Context&>);

}

TEST_CASE("rvalue context", "[new_fsm]") {

  struct Context {
    Context(int v) : value_(v), is_valid_(false) {}
    int value_;
    bool is_valid_;
  };

  ContextWrapper<Context> ctx_(Context{42});

  REQUIRE(std::is_same_v<decltype(ctx_.get()), Context&>);

}




