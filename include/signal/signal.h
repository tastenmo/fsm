/**
 * @file signal.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief Signals and Slots based on ENTT by Michele Caini
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "delegate.h"
#include "forwards.h"

namespace signal
{
    /**
     * @brief Slot class.
     *
     * Primary template isn't defined on purpose. All the specializations give a
     * compile-time error unless the template parameter is a function type.
     *
     * @tparam Type A valid signal handler type.
     */
    template <typename Type>
    class slot;

    /**
     * @brief Unmanaged signal handler.
     *
     * Primary template isn't defined on purpose. All the specializations give a
     * compile-time error unless the template parameter is a function type.
     *
     * @tparam Type A valid function type.
     * @tparam Allocator Type of allocator used to manage memory and elements.
     */
    template <typename Type, typename Allocator>
    class signal;

    /**
     * @brief Unmanaged signal handler.
     *
     * It works directly with references to classes and pointers to member functions
     * as well as pointers to free functions. Users of this class are in charge of
     * disconnecting instances before deleting them.
     *
     * This class serves mainly two purposes:
     *
     * * Creating signals to use later to notify a bunch of listeners.
     * * Collecting results from a set of functions like in a voting system.
     *
     * @tparam Ret Return type of a function type.
     * @tparam Args Types of arguments of a function type.
     * @tparam Allocator Type of allocator used to manage memory and elements.
     */
    template <typename Ret, typename... Args, typename Allocator>
    class signal<Ret(Args...), Allocator>
    {
        /*! @brief A slot is allowed to modify a signal. */
        friend class slot<signal<Ret(Args...), Allocator>>;

        using alloc_traits = std::allocator_traits<Allocator>;
        using container_type = std::vector<delegate<Ret(Args...)>, typename alloc_traits::template rebind_alloc<delegate<Ret(Args...)>>>;

    public:
        /*! @brief Allocator type. */
        using allocator_type = Allocator;
        /*! @brief Unsigned integer type. */
        using size_type = std::size_t;
        /*! @brief Slot type. */
        using slot_type = slot<signal<Ret(Args...), Allocator>>;

        /*! @brief Default constructor. */
        signal() noexcept(std::is_nothrow_default_constructible_v<allocator_type> &&std::is_nothrow_constructible_v<container_type, const allocator_type &>)
            : signal{allocator_type{}} {}

        /**
         * @brief Constructs a signal handler with a given allocator.
         * @param allocator The allocator to use.
         */
        explicit signal(const allocator_type &allocator) noexcept(std::is_nothrow_constructible_v<container_type, const allocator_type &>)
            : calls{allocator} {}

        /**
         * @brief Copy constructor.
         * @param other The instance to copy from.
         */
        signal(const signal &other) noexcept(std::is_nothrow_copy_constructible_v<container_type>)
            : calls{other.calls} {}

        /**
         * @brief Allocator-extended copy constructor.
         * @param other The instance to copy from.
         * @param allocator The allocator to use.
         */
        signal(const signal &other, const allocator_type &allocator) noexcept(std::is_nothrow_constructible_v<container_type, const container_type &, const allocator_type &>)
            : calls{other.calls, allocator} {}

        /**
         * @brief Move constructor.
         * @param other The instance to move from.
         */
        signal(signal &&other) noexcept(std::is_nothrow_move_constructible_v<container_type>)
            : calls{std::move(other.calls)} {}

        /**
         * @brief Allocator-extended move constructor.
         * @param other The instance to move from.
         * @param allocator The allocator to use.
         */
        signal(signal &&other, const allocator_type &allocator) noexcept(std::is_nothrow_constructible_v<container_type, container_type &&, const allocator_type &>)
            : calls{std::move(other.calls), allocator} {}

        /**
         * @brief Copy assignment operator.
         * @param other The instance to copy from.
         * @return This signal handler.
         */
        signal &operator=(const signal &other) noexcept(std::is_nothrow_copy_assignable_v<container_type>)
        {
            calls = other.calls;
            return *this;
        }

        /**
         * @brief Move assignment operator.
         * @param other The instance to move from.
         * @return This signal handler.
         */
        signal &operator=(signal &&other) noexcept(std::is_nothrow_move_assignable_v<container_type>)
        {
            calls = std::move(other.calls);
            return *this;
        }

        /**
         * @brief Exchanges the contents with those of a given signal handler.
         * @param other Signal handler to exchange the content with.
         */
        void swap(signal &other) noexcept(std::is_nothrow_swappable_v<container_type>)
        {
            using std::swap;
            swap(calls, other.calls);
        }

        /**
         * @brief Returns the associated allocator.
         * @return The associated allocator.
         */
        [[nodiscard]] constexpr allocator_type get_allocator() const noexcept
        {
            return calls.get_allocator();
        }

        /**
         * @brief Number of listeners connected to the signal.
         * @return Number of listeners currently connected.
         */
        [[nodiscard]] size_type size() const noexcept
        {
            return calls.size();
        }

        /**
         * @brief Returns false if at least a listener is connected to the signal.
         * @return True if the signal has no listeners connected, false otherwise.
         */
        [[nodiscard]] bool empty() const noexcept
        {
            return calls.empty();
        }

        /**
         * @brief Triggers a signal.
         *
         * All the listeners are notified. Order isn't guaranteed.
         *
         * @param args Arguments to use to invoke listeners.
         */
        void publish(Args... args) const
        {
            for (auto &&call : std::as_const(calls))
            {
                call(args...);
            }
        }

        /**
         * @brief Collects return values from the listeners.
         *
         * The collector must expose a call operator with the following properties:
         *
         * * The return type is either `void` or such that it's convertible to
         *   `bool`. In the second case, a true value will stop the iteration.
         * * The list of parameters is empty if `Ret` is `void`, otherwise it
         *   contains a single element such that `Ret` is convertible to it.
         *
         * @tparam Func Type of collector to use, if any.
         * @param func A valid function object.
         * @param args Arguments to use to invoke listeners.
         */
        template <typename Func>
        void collect(Func func, Args... args) const
        {
            for (auto &&call : calls)
            {
                if constexpr (std::is_void_v<Ret>)
                {
                    if constexpr (std::is_invocable_r_v<bool, Func>)
                    {
                        call(args...);
                        if (func())
                        {
                            break;
                        }
                    }
                    else
                    {
                        call(args...);
                        func();
                    }
                }
                else
                {
                    if constexpr (std::is_invocable_r_v<bool, Func, Ret>)
                    {
                        if (func(call(args...)))
                        {
                            break;
                        }
                    }
                    else
                    {
                        func(call(args...));
                    }
                }
            }
        }

    private:
        container_type calls;
    };

    /**
     * @brief Connection class.
     *
     * Opaque object the aim of which is to allow users to release an already
     * estabilished connection without having to keep a reference to the signal or
     * the slot that generated it.
     */
    class connection
    {
        /*! @brief A slot is allowed to create connection objects. */
        template <typename>
        friend class slot;

        connection(delegate<void(void *)> fn, void *ref)
            : disconnect{fn}, signal{ref} {}

    public:
        /*! @brief Default constructor. */
        connection()
            : disconnect{},
              signal{} {}

        /**
         * @brief Checks whether a connection is properly initialized.
         * @return True if the connection is properly initialized, false otherwise.
         */
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return static_cast<bool>(disconnect);
        }

        /*! @brief Breaks the connection. */
        void release()
        {
            if (disconnect)
            {
                disconnect(signal);
                disconnect.reset();
            }
        }

    private:
        delegate<void(void *)> disconnect;
        void *signal;
    };

    /**
     * @brief Scoped connection class.
     *
     * Opaque object the aim of which is to allow users to release an already
     * estabilished connection without having to keep a reference to the signal or
     * the slot that generated it.<br/>
     * A scoped connection automatically breaks the link between the two objects
     * when it goes out of scope.
     */
    struct scoped_connection
    {
        /*! @brief Default constructor. */
        scoped_connection() = default;

        /**
         * @brief Constructs a scoped connection from a basic connection.
         * @param other A valid connection object.
         */
        scoped_connection(const connection &other)
            : conn{other} {}

        /*! @brief Default copy constructor, deleted on purpose. */
        scoped_connection(const scoped_connection &) = delete;

        /**
         * @brief Move constructor.
         * @param other The scoped connection to move from.
         */
        scoped_connection(scoped_connection &&other) noexcept
            : conn{std::exchange(other.conn, {})} {}

        /*! @brief Automatically breaks the link on destruction. */
        ~scoped_connection()
        {
            conn.release();
        }

        /**
         * @brief Default copy assignment operator, deleted on purpose.
         * @return This scoped connection.
         */
        scoped_connection &operator=(const scoped_connection &) = delete;

        /**
         * @brief Move assignment operator.
         * @param other The scoped connection to move from.
         * @return This scoped connection.
         */
        scoped_connection &operator=(scoped_connection &&other) noexcept
        {
            conn = std::exchange(other.conn, {});
            return *this;
        }

        /**
         * @brief Acquires a connection.
         * @param other The connection object to acquire.
         * @return This scoped connection.
         */
        scoped_connection &operator=(connection other)
        {
            conn = std::move(other);
            return *this;
        }

        /**
         * @brief Checks whether a scoped connection is properly initialized.
         * @return True if the connection is properly initialized, false otherwise.
         */
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return static_cast<bool>(conn);
        }

        /*! @brief Breaks the connection. */
        void release()
        {
            conn.release();
        }

    private:
        connection conn;
    };

    /**
     * @brief Slot class.
     *
     * A slot is used to connect listeners to signals and to disconnect them.<br/>
     * The function type for a listener is the one of the signal to which it
     * belongs.
     *
     * The clear separation between a signal and a slot permits to store the former
     * as private data member without exposing the publish functionality to the
     * users of the class.
     *
     * @warning
     * Lifetime of a slot must not overcome that of the signal to which it refers.
     * In any other case, attempting to use a slot results in undefined behavior.
     *
     * @tparam Ret Return type of a function type.
     * @tparam Args Types of arguments of a function type.
     * @tparam Allocator Type of allocator used to manage memory and elements.
     */
    template <typename Ret, typename... Args, typename Allocator>
    class slot<signal<Ret(Args...), Allocator>>
    {
        using signal_type = signal<Ret(Args...), Allocator>;
        using difference_type = typename signal_type::container_type::difference_type;

        template <auto Candidate, typename Type>
        static void release(Type value_or_instance, void *signal)
        {
            slot{*static_cast<signal_type *>(signal)}.disconnect<Candidate>(value_or_instance);
        }

        template <auto Candidate>
        static void release(void *signal)
        {
            slot{*static_cast<signal_type *>(signal)}.disconnect<Candidate>();
        }

    public:
        /**
         * @brief Constructs a slot that is allowed to modify a given signal.
         * @param ref A valid reference to a signal object.
         */
        slot(signal<Ret(Args...), Allocator> &ref) noexcept
            : _offset{},
              _signal{&ref} {}

        /**
         * @brief Returns false if at least a listener is connected to the slot.
         * @return True if the slot has no listeners connected, false otherwise.
         */
        [[nodiscard]] bool empty() const noexcept
        {
            return _signal->calls.empty();
        }

        /**
         * @brief Returns a slot that connects before a given free function or an
         * unbound member.
         * @tparam Function A valid free function pointer.
         * @return A properly initialized slot object.
         */
        template <auto Function>
        [[nodiscard]] slot before()
        {
            delegate<Ret(Args...)> call{};
            call.template connect<Function>();

            const auto &calls = _signal->calls;
            const auto it = std::find(calls.cbegin(), calls.cend(), std::move(call));

            slot other{*this};
            other._offset = calls.cend() - it;
            return other;
        }

        /**
         * @brief Returns a slot that connects before a free function with payload
         * or a bound member.
         * @tparam Candidate Member or free function to look for.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid object that fits the purpose.
         * @return A properly initialized slot object.
         */
        template <auto Candidate, typename Type>
        [[nodiscard]] slot before(Type &&value_or_instance)
        {
            delegate<Ret(Args...)> call{};
            call.template connect<Candidate>(value_or_instance);

            const auto &calls = _signal->calls;
            const auto it = std::find(calls.cbegin(), calls.cend(), std::move(call));

            slot other{*this};
            other._offset = calls.cend() - it;
            return other;
        }

        /**
         * @brief Returns a slot that connects before a given instance or specific
         * payload.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid object that fits the purpose.
         * @return A properly initialized slot object.
         */
        template <typename Type>
        [[nodiscard]] slot before(Type &value_or_instance)
        {
            return before(&value_or_instance);
        }

        /**
         * @brief Returns a slot that connects before a given instance or specific
         * payload.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid pointer that fits the purpose.
         * @return A properly initialized slot object.
         */
        template <typename Type>
        [[nodiscard]] slot before(Type *value_or_instance)
        {
            slot other{*this};

            if (value_or_instance)
            {
                const auto &calls = _signal->calls;
                const auto it = std::find_if(calls.cbegin(), calls.cend(), [value_or_instance](const auto &delegate)
                                             { return delegate.data() == value_or_instance; });

                other._offset = calls.cend() - it;
            }

            return other;
        }

        /**
         * @brief Returns a slot that connects before anything else.
         * @return A properly initialized slot object.
         */
        [[nodiscard]] slot before()
        {
            slot other{*this};
            other._offset = _signal->calls.size();
            return other;
        }

        /**
         * @brief Connects a free function (with or without payload), a bound or an
         * unbound member to a signal.
         *
         * The signal isn't responsible for the connected object or the payload, if
         * any. Users must guarantee that the lifetime of the instance overcomes the
         * one of the signal. On the other side, the signal handler performs
         * checks to avoid multiple connections for the same function.<br/>
         * When used to connect a free function with payload, its signature must be
         * such that the instance is the first argument before the ones used to
         * define the signal itself.
         *
         * @tparam Candidate Function or member to connect to the signal.
         * @tparam Type Type of class or type of payload, if any.
         * @param value_or_instance A valid object that fits the purpose, if any.
         * @return A properly initialized connection object.
         */
        template <auto Candidate, typename... Type>
        connection connect(Type &&...value_or_instance)
        {
            disconnect<Candidate>(value_or_instance...);

            delegate<Ret(Args...)> call{};
            call.template connect<Candidate>(value_or_instance...);
            _signal->calls.insert(_signal->calls.end() - _offset, std::move(call));

            delegate<void(void *)> conn{};
            conn.template connect<&release<Candidate, Type...>>(value_or_instance...);
            return {std::move(conn), _signal};
        }

        /**
         * @brief Disconnects a free function (with or without payload), a bound or
         * an unbound member from a signal.
         * @tparam Candidate Function or member to disconnect from the signal.
         * @tparam Type Type of class or type of payload, if any.
         * @param value_or_instance A valid object that fits the purpose, if any.
         */
        template <auto Candidate, typename... Type>
        void disconnect(Type &&...value_or_instance)
        {
            auto &calls = _signal->calls;
            delegate<Ret(Args...)> call{};
            call.template connect<Candidate>(value_or_instance...);
            calls.erase(std::remove(calls.begin(), calls.end(), std::move(call)), calls.end());
        }

        /**
         * @brief Disconnects free functions with payload or bound members from a
         * signal.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid object that fits the purpose.
         */
        template <typename Type>
        void disconnect(Type &value_or_instance)
        {
            disconnect(&value_or_instance);
        }

        /**
         * @brief Disconnects free functions with payload or bound members from a
         * signal.
         * @tparam Type Type of class or type of payload.
         * @param value_or_instance A valid object that fits the purpose.
         */
        template <typename Type>
        void disconnect(Type *value_or_instance)
        {
            if (value_or_instance)
            {
                auto &calls = _signal->calls;
                auto predicate = [value_or_instance](const auto &delegate)
                { return delegate.data() == value_or_instance; };
                calls.erase(std::remove_if(calls.begin(), calls.end(), std::move(predicate)), calls.end());
            }
        }

        /*! @brief Disconnects all the listeners from a signal. */
        void disconnect()
        {
            _signal->calls.clear();
        }

    private:
        difference_type _offset;
        signal_type *_signal;
    };

    /**
     * @brief Deduction guide.
     *
     * It allows to deduce the signal handler type of a slot directly from the
     * signal it refers to.
     *
     * @tparam Ret Return type of a function type.
     * @tparam Args Types of arguments of a function type.
     * @tparam Allocator Type of allocator used to manage memory and elements.
     */
    template <typename Ret, typename... Args, typename Allocator>
    slot(signal<Ret(Args...), Allocator> &) -> slot<signal<Ret(Args...), Allocator>>;

} // namespace signal