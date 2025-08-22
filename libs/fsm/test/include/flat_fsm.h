
#include <fsm/state.h>
#include <fsm/state_machine.h>

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
  bool is_valid_ = false;

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

struct Initial;
struct Second;
struct Third;

using States = states<Initial, Second, Third>;

using Machine = StateMachine<States, Context &>;

using MachineWithOwnContext = StateMachine<States, Context>;

struct Initial : state<Initial, Machine, Context> {

  // using state<StateInitial, StateContainer>::state;

  Initial(Machine &sm, Context &ctx) noexcept
      : state(sm, ctx), count1(0), value2(0) {}

  void onEnter(const event1 &) { count1++; }

  void onEnter(const event2 &ev) { value2 += ev.value_; }

  /**
   * @brief simple transition
   *
   * @return auto
   */
  auto transitionTo(const event1 &) { return sibling<Second>(); }
  // auto transitionTo(const event2 &) const { return not_handled(); }

  // template<>
  // auto transitionTo<StateSecond>(const event1 &);

  int count1;
  int value2;

};

struct Second : state<Second, Machine, Context> {

  //using state<Second, StateContainer>::state;

  Second(Machine &sm, Context &ctx) noexcept : state(sm, ctx), count1(0) {}

  void onEnter() {
    count1++;
    context_.is_valid(true);
    context_.value(context_.value() + 1);
  }

  auto transitionTo(const event2 &event) const
      -> transitions<Initial, Second, Third> {
    if (event.value_ == 1) {
      return sibling<Initial>();
    } else if (event.value_ == 2) {
      return sibling<Third>();
    }
    // handled() does not work yet
    return sibling<Second>();
  }

  // auto transitionTo(const event1 &) const { return handled(); }

  int count1;

};

struct Third : state<Third, Machine, Context> {

  Third(Machine &sm,Context &ctx) noexcept : state(sm, ctx), count1(0) {}

  void onEnter(const event2 &ev) {
    count1++;
    context_.is_valid(false);
    if (ev.value_ == 2) {
      context_.value(10);
      // state_container_.emplace<StateInitial>();
    }
  }

  // auto transitionTo(const event2 &) const { return handled(); }
  // auto transitionTo(const event1 &) const { return handled(); }

  // StateThird() : count1(0) {}

  int count1;

};



} // namespace flat