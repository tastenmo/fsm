
#include <new_fsm/initial_state.h>

using namespace escad::new_fsm;

namespace flat {

class Context {

public:
  Context() : value_() { std::cout << "Context::Context()" << std::endl; }

  Context(int val) : value_(val) {
    std::cout << "Context::Context(int val)" << std::endl;
  }

  Context(const Context &other)
      : is_valid_(other.is_valid()), value_(other.value()) {
    std::cout << "Context::Context(const Context &other)" << std::endl;
  }

  Context(Context &&other) noexcept
      : is_valid_(std::move(other.is_valid())),
        value_(std::move(other.value())) {
    std::cout << "Context::Context(Context &&other)" << std::endl;
  }

  ~Context() { std::cout << "Context::~Context()" << std::endl; }

  int value() const { return value_; }

  void value(int val) { value_ = val; }

  bool is_valid() const { return is_valid_; }

  void is_valid(bool val) { is_valid_ = val; }

private:
  bool is_valid_;

  int value_;
};

struct event1 {};
struct event2 {

  event2(int val) : value_(val) {}

  int value_;
};
struct event3 {
  std::string msg;
};

struct StateInitial;
struct StateSecond;
struct StateThird;

using States = states<StateInitial, StateSecond, StateThird>;

using StateContainer = state_variant<States, Context &>;

struct StateInitial : initial_state<StateInitial, StateContainer, Context> {

  // using state<StateInitial, StateContainer>::state;

  StateInitial(Context &ctx) noexcept;

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return sibling<StateSecond>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;

  Context &ctx_;
};

struct StateSecond : state<StateSecond, Context> {

  //  using state<StateSecond, StateContainer>::state;

  StateSecond(Context &ctx) noexcept;

  void onEnter() {
    count1++;
    ctx_.is_valid(true);
    ctx_.value(ctx_.value() + 1);
  }

  auto transitionTo(const event2 &event) const
      -> transitions<StateInitial, StateSecond, StateThird> {
    if (event.value_ == 1) {
      return sibling<StateInitial>();
    } else if (event.value_ == 2) {
      return sibling<StateThird>();
    }
    // handled() does not work yet
    return sibling<StateSecond>();
  }

  // auto transitionTo(const event1 &) const { return handled(); }

  int count1;

  Context &ctx_;
};

struct StateThird : state<StateThird, Context> {

  StateThird(Context &ctx) noexcept;

  void onEnter(const event2 &ev) {
    count1++;
    ctx_.is_valid(false);
    if (ev.value_ == 2) {
      ctx_.value(10);
      // state_container_.emplace<StateInitial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // StateThird() : count1(0) {}

  int count1;
  Context &ctx_;
};

StateInitial::StateInitial(Context &ctx) noexcept
    : initial_state(ctx), count1(0), value2(0), ctx_(ctx) {}

StateSecond::StateSecond(Context &ctx) noexcept
    : state(ctx), count1(0), ctx_(ctx) {}

StateThird::StateThird(Context &ctx) noexcept
    : state(ctx), count1(0), ctx_(ctx) {}

} // namespace flat