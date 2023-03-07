#pragma once

#include <variant>

namespace escad {
namespace new_fsm {

template <typename T, typename... Args> struct concatenator;

template <typename... Args0, typename... Args1>
struct concatenator<std::variant<Args0...>, Args1...> {
  using type = std::variant<Args0..., Args1...>;
};


class machine{

    public:

    template<class... S>
    using states = mpl::type_list<S...>;

    using StateVariant = std::variant<std::monostate>;


    private:

    StateVariant state_;

    
};

} // namespace new_fsm
} // namespace escad
