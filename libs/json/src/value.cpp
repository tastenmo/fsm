
#include <json/array.h>
#include <json/object.h>
#include <json/value.h>


namespace escad::json::value {

Object::Object(Context &ctx) noexcept
    : recursive_state(
          ctx, object::StateContainer(mpl::type_identity<object::States>{},
                                      object::Context(ctx.view_))) {
  nested_emplace<object::Initial>();
}

auto Object::transitionInternalTo() -> transitions<Finished, Error> const {
  if (nested_in<object::Finished>()) {
    std::cout << "object found: " << nested()->context().value() << std::endl;

    jsonValue val = nested()->context().values();
    context_.addValue(val);

    return sibling<Finished>();
  };

  return sibling<Error>();
}

Array::Array(Context &ctx) noexcept
    : recursive_state(ctx,
                      array::StateContainer(mpl::type_identity<array::States>{},
                                            array::Context(ctx.view_))) {
  nested_emplace<array::Initial>();
}

auto Array::transitionInternalTo() -> transitions<Finished, Error> const {
  if (nested_in<array::Finished>()) {
    std::cout << "object found: " << nested()->context().value() << std::endl;

    jsonValue val = nested()->context().values();
    context_.addValue(val);

    return sibling<Finished>();
  };

  return sibling<Error>();
}
} // namespace escad::json::value