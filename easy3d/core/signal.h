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


#include <unordered_map>
#include <functional>


namespace easy3d {

    /**
     * \brief Implementation of a simple signal-slot mechanism.
     * \details Multiple slots (functions, lambda functions, and member functions) can be connected to a signal object.
     *        You can connect functions to the signal which will be called when the send() method on the signal
     *        object is invoked. Any argument passed to send() will be passed to the given functions. A typical usage
     *        of Signal in Easy3D is camera manipulation. When the camera has been manipulated, the viewer should be
     *        notified (e.g., a repaint event should be triggered). This is done by calling to the viewer's update()
     *        function. So in Easy3D, the viewer's update function is connected to the camera.
     * \note Current implementation can hold only one single function of each owner.
     * \class Signal easy3d/core/signal.h
     * \example test_signal  \include test/test_signal.cpp
     */

    template<typename... Args>
    class Signal {

    public:
        Signal() = default;
        ~Signal() = default;

        // Copy constructor and assignment create a new signal.
        Signal(Signal const & /*unused*/) {}

        Signal &operator=(Signal const &other) {
            if (this != &other) {
                disconnect_all();
            }
            return *this;
        }

        // Move constructor and assignment operator work as expected.
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


        /// Connects a const std::function to the signal.
        /// The returned value can be used to disconnect the function again.
        int connect(std::function<void(Args...)> const &slot) const {
            slots_.insert(std::make_pair(++current_id_, slot));
            return current_id_;
        }

        /// Connects a member function of an object to this Signal.
        /// The returned value can be used to disconnect the function again.
        template<typename Class>
        int connect(Class *inst, void (Class::*func)(Args...)) {
            return connect([=](Args... args) {
                (inst->*func)(args...);
            });
        }

        /// Connects a const member function of an object to this Signal.
        /// The returned value can be used to disconnect the function again.
        template<typename Class>
        int connect(Class *inst, void (Class::*func)(Args...) const) {
            return connect([=](Args... args) {
                (inst->*func)(args...);
            });
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

    /// Connects an std::function to the signal.
    /// The returned value can be used to disconnect the function again.
    template<typename SIGNAL, typename FUNCTION>
    inline int connect(SIGNAL *signal, FUNCTION const &slot) {
        return signal->connect(slot);
    }

    /// Connect a member function of an object to this Signal.
    /// The returned value can be used to disconnect the function again.
    template<typename SIGNAL, typename CLASS, typename FUNCTION>
    inline int connect(SIGNAL *signal, CLASS *inst, FUNCTION &slot) {
        return signal->connect(inst, slot);
    }

    /// Connects a const member function of an object to this Signal.
    /// The returned value can be used to disconnect the function again.
    template<typename SIGNAL, typename CLASS, typename FUNCTION>
    inline int connect(SIGNAL *signal, CLASS *inst, FUNCTION const &slot) {
        return signal->connect(inst, slot);
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
}


#endif // EASY3D_CORE_SIGNAL_H
