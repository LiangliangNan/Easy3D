/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_CORE_SIGNAL_H
#define EASY3D_CORE_SIGNAL_H


#include <functional>
#include <unordered_map>


namespace easy3d {

    /**
     * \brief Implementation of a simple signal-slot mechanism.
     * \class Signal easy3d/core/signal.h
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
     * \see A more powerful implementation [sigslot](https://github.com/palacaze/sigslot) based on C++14.
     *
     * \example Test_Signal  \include test/test_signal.cpp
     */

    template<typename... Args>
    class Signal {

    public:
        Signal() = default;

        ~Signal() = default;

        /// Copy constructor and assignment create a new signal.
        Signal(Signal const & /*unused*/) {}

        Signal &operator=(Signal const &other) {
            if (this != &other) {
                disconnect_all();
            }
            return *this;
        }

        /// Move constructor and assignment operator.
        Signal(Signal &&other) noexcept:
                slots_(std::move(other.slots_)),
                current_id_(other.current_id_) {}

        Signal &operator=(Signal &&other) noexcept {
            if (this != &other) {
                slots_ = std::move(other.slots_);
                current_id_ = other.current_id_;
            }

            return *this;
        }


        /**
         * Connects a function to this signal.
         * The returned value can be used to disconnect the function from this signal.
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., <tt> static_cast<void (*)(const std::string&, int)>(&print). </tt> Or use the helper function
         *     \fn overload for a lighter syntax.
         */
        int connect(std::function<void(Args...)> const &slot) const {
            slots_.insert(std::make_pair(++current_id_, slot));
            return current_id_;
        }

        /**
         * Connects a member function of an object to this signal.
         * The returned value can be used to disconnect the function from this signal.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., <tt> static_cast<void (Viewer::*)(const std::string&, int)>(Viewer::print). </tt>
         *     Or use the helper function \fn overload for a lighter syntax.
         */
        template<typename Class>
        int connect(Class *inst, void (Class::*func)(Args...)) {
            return connect([=](Args... args) {
                (inst->*func)(args...);
            });
        }

        /**
         * Connects a const member function of an object to this Signal.
         * The returned value can be used to disconnect the function from this signal.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., <tt> static_cast<void (Viewer::*)(const std::string&, int)>(Viewer::print). </tt>
         *     Or use the helper function \fn overload for a lighter syntax.
         */
        template<typename Class>
        int connect(Class *inst, void (Class::*func)(Args...) const) {
            return connect([=](Args... args) {
                (inst->*func)(args...);
            });
        }

        /**
         * Connects this signal to another signal \p receiver.
         * Upon return, the emission of this signal will trigger \p receiver to emit.
         * The returned value can be used to disconnect the connected signal.
         */
        int connect(Signal<Args...> *receiver) {
            return connect(receiver, &Signal<Args...>::send);
        }

        /// Disconnects a previously connected function.
        void disconnect(int id) const {
            slots_.erase(id);
        }

        /// Disconnects all previously connected functions.
        void disconnect_all() const {
            slots_.clear();
        }

        /// Calls all connected functions.
        void send(Args... p) {
            for (auto const &it : slots_) {
                it.second(p...);
            }
        }

        /// Calls all connected functions except for one.
        void send_for_all_but_one(int excludedConnectionID, Args... p) {
            for (auto const &it : slots_) {
                if (it.first != excludedConnectionID) {
                    it.second(p...);
                }
            }
        }

        /// Calls only one connected function.
        void emit_for(int connectionID, Args... p) {
            auto const &it = slots_.find(connectionID);
            if (it != slots_.end()) {
                it->second(p...);
            }
        }

    private:
        mutable std::unordered_map<int, std::function<void(Args...)>> slots_;
        mutable int current_id_{0};
    };


    /// \name  Global methods for connection and disconnection.
    //\{

    /**
     * Connects a function to the signal.
     * The returned value can be used to disconnect the function from this signal.
     * \note When a function has overloads, explicitly cast the function to the right function type using
     *     e.g., <tt> static_cast<void (*)(const std::string&, int)>(&print). </tt> Or use the helper function
     *     \fn overload for a lighter syntax.
     */
    template<typename SIGNAL, typename FUNCTION>
    inline int connect(SIGNAL *signal, FUNCTION const &slot) {
        return signal->connect(slot);
    }

    /**
     * Connects a member function of an object to this Signal.
     * The returned value can be used to disconnect the function from this signal.
     * \note When a member function has overloads, explicitly cast the function to the right function type using
     *     e.g., <tt> static_cast<void (Viewer::*)(const std::string&, int)>(Viewer::print). </tt>
     *     Or use the helper function \fn overload for a lighter syntax.
     */
    template<typename SIGNAL, typename CLASS, typename FUNCTION>
    inline int connect(SIGNAL *signal, CLASS *inst, FUNCTION const &slot) {
        return signal->connect(inst, slot);
    }

    /**
     * Connects this signal to \p another signal.
     * Upon return, the emission of this signal will trigger \p another to be emitted.
     * The returned value can be used to disconnect the connected signal.
     */
    template<typename... Args>
    int connect(Signal<Args...> *sender, Signal<Args...> *receiver) {
        return sender->connect(receiver);
    }

    /// Disconnects a previously connected function.
    template<typename SIGNAL>
    inline void disconnect(SIGNAL *signal, int id) {
        signal->disconnect(id);
    }

    /// Disconnects all previously connected functions.
    template<typename SIGNAL>
    inline void disconnect_all(SIGNAL *signal) {
        signal->disconnect_all();
    }

    //\}


    /// \name  Helper functions for resolving overloaded functions.
    //\{

    /**
     * Helper function for resolving overloaded non-member functions.
     * In case of overloading, the pointer to a function does not map to a unique symbol, so the compiler
     * won't be able to pick the right symbol. One way of resolving the right symbol is to explicitly cast
     * the function pointer to the right function type, e.g., static_cast<void (*)(const std::string&, int)>(func),
     * which is a bit heavy syntax. This helper function does just that for a lighter syntax.
     * \example
     *     \code
     *         void foo() {}       // function #1
     *         void foo(int) {}    // function #2
     *         sig_1.connect(overload<>(&foo));    // sig_1 connects to function #1
     *         sig_2.connect(overload<int>(&foo)); // sig_2 connects to function #2
     *     \endcode
     * \sa overload(void (C::*func)(Args...))
     */
    template <typename... Args>
    inline constexpr auto overload(void (*func)(Args...)) -> void (*)(Args...) {
        return func;
    }

    /**
     * Helper function for resolving overloaded member functions.
     * In case of overloading, the pointer to a member function does not map to a unique symbol, so the compiler
     * won't be able to pick the right symbol. One way of resolving the right symbol is to explicitly cast
     * the function pointer to the right function type, e.g.,
     * static_cast<void (Viewer::*)(const std::string&, int)>(Viewer::print), which is a bit heave syntax.
     * This helper function does just that for a lighter syntax.
     * \example
     *     \code
     *         struct foo {
     *             void bar() {}       // function #1
     *             void bar(int) {}    // function #2
     *         };
     *         sig_1.connect(overload<>(&foo::bar));    // sig_1 connects to function #1
     *         sig_2.connect(overload<int>(&foo::bar)); // sig_2 connects to function #2
     *     \endcode
     * \sa overload(void (*func)(Args...))
     */
    template <typename C, typename... Args>
    inline constexpr auto overload(void (C::*func)(Args...)) -> void (C::*)(Args...) {
        return func;
    }

    /**
     * About default arguments.
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
     * But with C++11, \c auto is not allowed in lambda parameters.
     */

    //\}
}


#endif // EASY3D_CORE_SIGNAL_H
