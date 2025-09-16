#include <catch2/catch_test_macros.hpp>
#include <fsm/context.h>

using namespace spie::fsm;

struct Context {
   int value;
   Context() = default;
   Context(int v) : value(v) {}
   friend std::ostream &operator<<(std::ostream &os, const Context &ctx) {
      os << ctx.value;
      return os;
   }
};

struct NonDefaultContext {
   int value;
   NonDefaultContext() = delete;
   NonDefaultContext(int v) : value(v) {}
   friend std::ostream &operator<<(std::ostream &os,
                                   const NonDefaultContext &ctx) {
      os << ctx.value;
      return os;
   }
};

TEST_CASE("ContextWrapper lvalue construction", "[ContextWrapper]") {
   Context ctx{42};
   ContextWrapper<Context> wrapper(ctx);
   CHECK(&ctx == &wrapper.get());

   NonDefaultContext ndctx{55};
   ContextWrapper<NonDefaultContext> ndwrapper(ndctx);
   CHECK(&ndctx == &ndwrapper.get());
}

TEST_CASE("ContextWrapper rvalue construction", "[ContextWrapper]") {
   ContextWrapper<Context> wrapper(Context{99});
   CHECK(wrapper.get().value == 99);

   ContextWrapper<NonDefaultContext> ndwrapper(NonDefaultContext{66});
   CHECK(ndwrapper.get().value == 66);
}

TEST_CASE("ContextWrapper move construction from lvalue", "[ContextWrapper]") {
   Context ctx{123};
   ContextWrapper<Context> wrapper(ctx);
   ContextWrapper<Context> moved(std::move(wrapper));
   CHECK(&ctx == &wrapper.get());
   CHECK(&ctx == &moved.get());

   NonDefaultContext ndctx{77};
   ContextWrapper<NonDefaultContext> ndwrapper(ndctx);
   ContextWrapper<NonDefaultContext> ndmoved(std::move(ndwrapper));
   CHECK(&ndctx == &ndwrapper.get());
   CHECK(&ndctx == &ndmoved.get());
}

TEST_CASE("ContextWrapper move construction from rvalue", "[ContextWrapper]") {
   ContextWrapper<Context> wrapper(Context{321});
   ContextWrapper<Context> moved(std::move(wrapper));
   CHECK(moved.get().value == 321);

   ContextWrapper<NonDefaultContext> ndwrapper(NonDefaultContext{88});
   ContextWrapper<NonDefaultContext> ndmoved(std::move(ndwrapper));
   CHECK(ndmoved.get().value == 88);
}

TEST_CASE("ContextWrapper move assignment from lvalue", "[ContextWrapper]") {
   Context ctx{77};
   ContextWrapper<Context> wrapper(ctx);
   ContextWrapper<Context> target(Context{88});
   target = std::move(wrapper);
   CHECK(&ctx == &target.get());
   CHECK(&ctx == &wrapper.get());

   NonDefaultContext ndctx{99};
   ContextWrapper<NonDefaultContext> ndwrapper(ndctx);
   ContextWrapper<NonDefaultContext> ndtarget(NonDefaultContext{100});
   ndtarget = std::move(ndwrapper);
   CHECK(&ndctx == &ndtarget.get());
   CHECK(&ndctx == &ndwrapper.get());
}

TEST_CASE("ContextWrapper move assignment from rvalue", "[ContextWrapper]") {
   ContextWrapper<Context> wrapper(Context{101});
   ContextWrapper<Context> target(Context{202});
   target = std::move(wrapper);
   CHECK(target.get().value == 101);
   CHECK(wrapper.get().value == 101);

   ContextWrapper<NonDefaultContext> ndwrapper(NonDefaultContext{111});
   ContextWrapper<NonDefaultContext> ndtarget(NonDefaultContext{222});
   ndtarget = std::move(ndwrapper);
   CHECK(ndtarget.get().value == 111);
   CHECK(ndwrapper.get().value == 111);
}
