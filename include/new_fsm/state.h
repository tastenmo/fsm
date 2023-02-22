/**
 *
 */

#pragma once

namespace escad {
namespace new_fsm {

template <typename Derived>
struct state {

  template <typename Target = Derived, typename Event>
  auto enter(const Event &event)
      -> decltype(std::declval<Target>().onEnter(event), void()) {
    static_cast<Target *>(this)->onEnter(event);
  }

  template <typename Target = Derived>
  auto enter() -> decltype(std::declval<Target>().onEnter(), void()) {
    static_cast<Target *>(this)->onEnter();
  }

  void enter(...) const noexcept {}
};

}  // namespace new_fsm
}  // namespace escad