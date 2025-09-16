#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <type_traits>

namespace spie {
namespace fsm {

enum class ContextKind { LValue, RValue };

/**
 * @brief A wrapper class for managing ownership and reference semantics of a
 * Context object.
 *
 * ContextWrapper allows you to either reference an existing Context object
 * (lvalue) or take ownership of a Context object (rvalue). Internally, it
 * stores either a reference to an external Context or owns a Context instance,
 * providing unified access via get().
 *
 * @tparam Context The type of the context object to wrap.
 *
 * Usage:
 * - Construct with lvalue reference: ContextWrapper<Context> wrapper(ctx);
 * - Construct with rvalue: ContextWrapper<Context> wrapper(std::move(ctx));
 *
 * Copy operations are deleted; only move semantics are supported.
 */
template <class Context> class ContextWrapper {
 private:
   /**
    * @brief Holds the owned context if constructed from an rvalue, otherwise
    * empty.
    */
   std::optional<Context> owned_context_;

   /**
    * @brief Reference to the context object (owned or external).
    */
   std::reference_wrapper<Context> ref_;

 public:
   /**
    * @brief Construct from an lvalue reference to Context.
    *
    * @param ctx Reference to an external context object.
    */
   constexpr ContextWrapper(Context &ctx) noexcept
       : owned_context_(std::nullopt), ref_(ctx) {}

   /**
    * @brief Construct from an rvalue Context, taking ownership.
    *
    * @param ctx Rvalue context object to be owned.
    */
   constexpr ContextWrapper(Context &&ctx) noexcept(
       std::is_nothrow_move_constructible_v<Context>)
       : owned_context_(std::move(ctx)), ref_(*owned_context_) {}

   /**
    * @brief Move constructor. Preserves reference/ownership semantics.
    *
    * @param other The ContextWrapper to move from.
    */
   ContextWrapper(ContextWrapper &&other) noexcept
       : owned_context_(std::move(other.owned_context_)),
         ref_(other.owned_context_.has_value() ? *owned_context_
                                               : other.ref_.get()) {}

   /**
    * @brief Move assignment operator. Preserves reference/ownership semantics.
    *
    * @param other The ContextWrapper to move from.
    * @return Reference to this object.
    */
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

   /**
    * @brief Copy construction and assignment are deleted to prevent accidental
    * shallow copies.
    */
   ContextWrapper(const ContextWrapper &) = delete;
   ContextWrapper &operator=(const ContextWrapper &) = delete;

   /**
    * @brief Get a reference to the context object (owned or external).
    *
    * @return Reference to the context object.
    */
   constexpr auto &get() noexcept { return ref_.get(); }

   /**
    * @brief Get a const reference to the context object (owned or external).
    *
    * @return Const reference to the context object.
    */
   constexpr const auto &get() const noexcept { return ref_.get(); }
};

} // namespace fsm
} // namespace spie