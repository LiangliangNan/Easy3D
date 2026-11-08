/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#ifndef EASY3D_UTIL_SIGNAL_H
#define EASY3D_UTIL_SIGNAL_H


#include <functional>
#include <unordered_map>


namespace easy3d {

    /**
     * \brief A light-weight implementation of the simple signal-slot mechanism.
     * \class Signal easy3d/util/signal.h
     * \details Signal supports any types of functions (functions, lambda functions, and member functions) with any
     *      number of arguments. Connected functions will be called when the send() method of the signal object is
     *      invoked. Any argument passed to send() will be passed to the given functions.
     *      Multiple slots can be connected to the same signal object. A signal (i.e., the sender) can be connected
     *      to another signal (e.g., the receiver).
     *      A typical usage of Signal in Easy3D is camera manipulation. When the camera has been manipulated, the
     *      viewer should be notified (e.g., a repaint event should be triggered). This is done by calling to the
     *      viewer's update() function. So in Easy3D, the viewer's update function is connected to the camera's
     *      corresponding signal.
     *
     * \note Slots are invoked synchronously in unspecified order. This class does not provide synchronization;
     *       connecting, disconnecting, or emitting concurrently requires external coordination.
     * \see A more powerful implementation [sigslot](https://github.com/palacaze/sigslot) based on C++14.
     */

    template<typename... Args>
    class Signal {

    public:
        /// Construct a signal with no connected slots.
        Signal() = default;
        ~Signal() { disconnect_all(); }

        /**
         * \brief Copy construction creates a fresh signal with no slots.
         */
        Signal(Signal const & /*other*/) {}

        /**
         * \brief Move a signal and its connections.
         * \param other Signal whose connections are transferred.
         */
        Signal(Signal &&other) noexcept:
                slots_(std::move(other.slots_)),
                current_id_(other.current_id_) {
        }

        /**
         * \brief Move-assign a signal and its connections.
         * \param other Signal whose connections are transferred.
         * \return Reference to this signal.
         */
        Signal &operator=(Signal &&other) noexcept {
            if (this != &other) {
                slots_ = std::move(other.slots_);
                current_id_ = other.current_id_;
            }
            return *this;
        }

        /**
         * \brief Copy-assign a signal by disconnecting this signal's current slots.
         * \param other Source signal; its connections are not copied.
         * \return Reference to this signal.
         */
        Signal &operator=(Signal const &other) {
            if (this != &other) {
                disconnect_all();
            }
            return *this;
        }

        /**
         * \brief Connects a function to this signal.
         * \details The returned value can be used to disconnect the function from this signal.
         * \note When a function has overloads, explicitly cast the function to the right function type, e.g.,
         *      \code
         *          static_cast<void (*)(const std::string&, int)>(&print)
         *      \endcode
         *     Or use the helper function \c overload for a lighter syntax.
         * \param slot Callable invoked when the signal is sent.
         * \return Connection identifier, usable with disconnect().
         */
        int connect(std::function<void(Args...)> const &slot) {
            slots_.insert(std::make_pair(++current_id_, slot));
            return current_id_;
        }

        /**
         * \brief Connects a member function of an object to this signal.
         * \details The returned value can be used to disconnect the function from this signal, e.g.,
         *    \code
         *       signal.connect(viewer, &Viewer::update);
         *    \endcode
         * \note When a member function has overloads, explicitly cast the function to the right function type, e.g.,
         *      \code
         *          static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print)
         *      \endcode
         *     Or use the helper function \c overload for a lighter syntax.
         * \param inst Object on which the member function is called; it must outlive the connection.
         * \param func Non-const member function to invoke.
         * \return Connection identifier, usable with disconnect().
         */
        template<typename Class>
        int connect(Class *inst, void (Class::*func)(Args...)) {
            return connect([=](Args... args) {
                (inst->*func)(args...);
            });
        }

        /**
         * \brief Connects a const member function of an object to this signal.
         * \details The returned value can be used to disconnect the function from this signal, e.g.,
         *    \code
         *       signal.connect(viewer, &Viewer::update);
         *    \endcode
         * \note When a member function has overloads, explicitly cast the function to the right function type, e.g.,
         *      \code
         *          static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print)
         *      \endcode
         *     Or use the helper function \c overload for a lighter syntax.
         * \param inst Object on which the member function is called; it must outlive the connection.
         * \param func Const member function to invoke.
         * \return Connection identifier, usable with disconnect().
         */
        template<typename Class>
        int connect(Class *inst, void (Class::*func)(Args...) const) {
            return connect([=](Args... args) {
                (inst->*func)(args...);
            });
        }

        /**
         * \brief Connects this signal to another signal \p receiver.
         * \details Upon return, the emission of this signal will trigger \p receiver to emit.
         * The returned value can be used to disconnect the connected signal.
         * \param receiver Signal to emit when this signal is sent; it must outlive the connection.
         * \return Connection identifier, usable with disconnect().
         */
        int connect(Signal<Args...> *receiver) {
            return connect(receiver, &Signal<Args...>::send);
        }

        /// Disconnect a slot by the identifier returned from connect().
        /// \param id Connection identifier.
        void disconnect(int id) {
            slots_.erase(id);
        }

        /// Disconnect all slots.
        void disconnect_all() {
            slots_.clear();
        }

        /// Invoke all connected slots synchronously.
        /// \param p Arguments forwarded to each slot.
        void send(Args... p) {
            for (auto const &it : slots_) {
                it.second(p...);
            }
        }

        /// Invoke every connected slot except the specified connection.
        /// \param excludedConnectionID Connection identifier to skip.
        /// \param p Arguments forwarded to each invoked slot.
        void send_for_all_but_one(int excludedConnectionID, Args... p) {
            for (auto const &it : slots_) {
                if (it.first != excludedConnectionID) {
                    it.second(p...);
                }
            }
        }

        /// Invoke the slot with the specified connection identifier, if it is still connected.
        /// \param connectionID Connection identifier to invoke.
        /// \param p Arguments forwarded to the slot.
        void emit_for(int connectionID, Args... p) {
            auto const &it = slots_.find(connectionID);
            if (it != slots_.end()) {
                it->second(p...);
            }
        }

    private:
        std::unordered_map<int, std::function<void(Args...)>> slots_;
        int current_id_{0};
    };


    /// \name  Global methods for connection and disconnection.
    //@{

    /**
     * \brief Connects a function to the signal.
     * \details The returned value can be used to disconnect the function from this signal.
     * \note When a function has overloads, explicitly cast the function to the right function type, e.g.,
     *      \code
     *          static_cast<void (*)(const std::string&, int)>(&print)
     *      \endcode
     *     Or use the helper function \c overload for a lighter syntax.
     * \param signal Signal receiving the connection.
     * \param slot Callable to connect.
     * \return Connection identifier, usable with disconnect().
     */
    template<typename SIGNAL, typename FUNCTION>
    inline int connect(SIGNAL *signal, FUNCTION const &slot) {
        return signal->connect(slot);
    }

    /**
     * \brief Connects a member function of an object to this Signal.
     * \details The returned value can be used to disconnect the function from this signal.
     *    \code
     *       easy3d::connect(&kfi_->interpolation_stopped, viewer, &Viewer::update);
     *    \endcode
     * \note When a member function has overloads, explicitly cast the function to the right function type, e.g.,
     *      \code
     *          static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print)
     *      \endcode
     *     Or use the helper function \c overload for a lighter syntax.
     * \param signal Signal receiving the connection.
     * \param inst Object whose member function is connected; it must outlive the connection.
     * \param slot Member function to connect.
     * \return Connection identifier, usable with disconnect().
     */
    template<typename SIGNAL, typename CLASS, typename FUNCTION>
    inline int connect(SIGNAL *signal, CLASS *inst, FUNCTION const &slot) {
        return signal->connect(inst, slot);
    }

    /**
     * \brief Connects this signal to \p another signal.
     * \details Upon return, the emission of this signal will trigger \p another to be emitted.
     * The returned value can be used to disconnect the connected signal.
     * \param sender Signal whose emission is forwarded.
     * \param receiver Signal emitted by the sender.
     * \return Connection identifier, usable with disconnect().
     */
    template<typename... Args>
    int connect(Signal<Args...> *sender, Signal<Args...> *receiver) {
        return sender->connect(receiver);
    }

    /// Disconnect a slot by the identifier returned from connect().
    /// \param signal Signal to update.
    /// \param id Connection identifier.
    template<typename SIGNAL>
    inline void disconnect(SIGNAL *signal, int id) {
        signal->disconnect(id);
    }

    /// Disconnect all slots from a signal.
    /// \param signal Signal to clear.
    template<typename SIGNAL>
    inline void disconnect_all(SIGNAL *signal) {
        signal->disconnect_all();
    }
    //@}

    /// \name  Helper functions for resolving overloaded functions.
    //@{
    /**
     * \brief Helper function for resolving overloaded non-member functions.
     * \details In case of overloading, the pointer to a function does not map to a unique symbol, so the compiler
     * won't be able to pick the right symbol. One way of resolving the right symbol is to explicitly cast
     * the function pointer to the right function type, e.g., static_cast<void (*)(const std::string&, int)>(func),
     * which is a bit heavy syntax. This helper function does just that for a lighter syntax.
     * Example usage:
     *     \code
     *         void foo() {}       // function #1
     *         void foo(int) {}    // function #2
     *         sig_1.connect(overload<>(&foo));    // sig_1 connects to function #1
     *         sig_2.connect(overload<int>(&foo)); // sig_2 connects to function #2
     *     \endcode
     * \sa overload(void (C::*func)(Args...))
     * \tparam Args Function parameter types.
     * \param func Overloaded function pointer to disambiguate.
     * \return The same function pointer with its parameter types fixed.
     */
    template <typename... Args>
    inline constexpr auto overload(void (*func)(Args...)) -> void (*)(Args...) {
        return func;
    }

    /**
     * \brief Helper function for resolving overloaded member functions.
     * \details In case of overloading, the pointer to a member function does not map to a unique symbol, so the compiler
     *      won't be able to pick the right symbol. One way of resolving the right symbol is to explicitly cast
     *      the function pointer to the right function type, e.g.,
     *      static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print), which is a bit heave syntax.
     *      This helper function does just that for a lighter syntax.
     * Example usage:
     *     \code
     *         struct foo {
     *             void bar() {}       // function #1
     *             void bar(int) {}    // function #2
     *         };
     *         sig_1.connect(overload<>(&foo::bar));    // sig_1 connects to function #1
     *         sig_2.connect(overload<int>(&foo::bar)); // sig_2 connects to function #2
     *     \endcode
     * \sa overload(void (*func)(Args...))
     * \tparam C Class declaring the member function.
     * \tparam Args Function parameter types.
     * \param func Overloaded member-function pointer to disambiguate.
     * \return The same member-function pointer with its parameter types fixed.
     */
    template <typename C, typename... Args>
    inline constexpr auto overload(void (C::*func)(Args...)) -> void (C::*)(Args...) {
        return func;
    }
    //@}


    /// \name About default arguments
    //@{
    /**
     * Default arguments are not part of the function type signature, and can be redefined, so they are really
     * difficult to deal with. When connecting a slot to a signal, the \c Signal determines if the supplied callable
     * can be invoked with the signal argument types, but at this point the existence of default arguments is unknown,
     * so there might be a mismatch in the number of arguments. Below is an example,
     *      \code
     *          void foo(int &i, int b = 1) {}  // a function with an default argument
     *          Signal<int &> signal;           // a signal with one argument
     *          signal.connect(foo);            // this will never work
     *          // but the following work
     *          Signal<int &, int> signal2;     // a signal handles all the arguments
     *          signal2.connect(foo);           // this works
     *      \endcode
     *
     * Starting from C++14, we can have a simple workaround, to create a bind adapter. In fact we can even make it
     * quite generic like so:
     *      \code
     *          #define ADAPT(func) \
     *                  [=]( auto && ...a) { (func)(std::forward<decltype(a)>(a)...); }
     *          Signal<int &> signal;           // a signal with one argument
     *          signal.connect(ADAPT(foo));     // this will work for >= C++14
     *      \endcode
     * But with C++11, \c auto is not allowed in lambda parameters.
     */
    //@}
}


#endif // EASY3D_UTIL_SIGNAL_H
