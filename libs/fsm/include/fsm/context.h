#pragma once

#include <optional>
#include <type_traits>

namespace escad {

namespace new_fsm {


    template <class Context> struct ContextWrapper {
        ContextWrapper(Context& ctx) : context({}), ref(ctx) {

            std::cout << "ContextWrapper(Context& ctx) lvalue at: " << &ref << std::endl;
        }
        ContextWrapper(Context&& ctx) : context(std::move(ctx)), ref(context) {
            std::cout << "ContextWrapper(Context&& ctx) rvalue at: " << &ref << std::endl;
        }

        ContextWrapper(ContextWrapper && other) noexcept
            : context(std::move(other.context)), ref(context) {
            std::cout << "ContextWrapper(ContextWrapper && other) at: " << &ref << std::endl;
        }

        ContextWrapper(ContextWrapper const&) = delete;
        ContextWrapper& operator=(ContextWrapper const&) = delete;
        ContextWrapper& operator=(ContextWrapper &&) = delete;



        Context context;
        std::reference_wrapper<Context> ref;


        auto& get() { return ref.get(); }
        auto const& get() const { return ref.get(); }
    };

    


}

}