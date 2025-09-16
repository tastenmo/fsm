#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <type_traits>

namespace escad {
namespace new_fsm {

enum class ContextKind { LValue, RValue };

template <class Context> class ContextWrapper {
 private:
   std::optional<Context> owned_context_;
   std::reference_wrapper<Context> ref_;

 public:
   /**
    * @brief Constructs a ContextWrapper from an lvalue reference to a Context
    * object.
    */
   constexpr ContextWrapper(Context &ctx) noexcept
       : owned_context_(std::nullopt), ref_(ctx) {}

   /**
    * @brief Constructs a ContextWrapper from an rvalue Context object (takes
    * ownership).
    */
   constexpr ContextWrapper(Context &&ctx) noexcept(
       std::is_nothrow_move_constructible_v<Context>)
       : owned_context_(std::move(ctx)), ref_(*owned_context_) {}

   /**
    * @brief Move constructor: preserves reference or ownership as appropriate.
    */
   ContextWrapper(ContextWrapper &&other) noexcept
       : owned_context_(std::move(other.owned_context_)),
         ref_(other.owned_context_.has_value() ? *owned_context_
                                               : other.ref_.get()) {}

   ContextWrapper &operator=(ContextWrapper &&other) noexcept {
      if (this != &other) {
         owned_context_ = std::move(other.owned_context_);
         if (owned_context_.has_value()) {
            ref_ = *owned_context_;
         } else {
            ref_ = other.ref_.get();
         }
      }
      return *this;
   }

   ContextWrapper(const ContextWrapper &) = delete;
   ContextWrapper &operator=(const ContextWrapper &) = delete;

   constexpr auto &get() noexcept { return ref_.get(); }
   constexpr const auto &get() const noexcept { return ref_.get(); }
};

} // namespace new_fsm
} // namespace escad