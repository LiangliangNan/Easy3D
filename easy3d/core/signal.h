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
     * \brief Implementation of a simple signal-slot mechanism. \par
     * \details Multiple slots (classes and their member functions) can be connected to a signal object.
     *        You can connect functions to the signal which will be called when the trigger() method on the signal
     *        object is invoked. Any argument passed to emit() will be passed to the given functions. A typical usage
     *        of Signal in Easy3D is camera manipulation. When the camera has been manipulated, the viewer should be
     *        notified (e.g., a repaint event should be triggered). This is done by calling to the viewer's update()
     *        function. So in Easy3D, the viewer's update function is connected to the camera.
     * \note Current implementation can hold only one single function of each owner.
     * \class Signal easy3d/core/signal.h
     * \todo Extend this class to accept multiple functions of a class.
     */
    class Signal {
	public:
        /**
         * \brief Connect an owned slot to this signal.
         *  - If no overloaded function:  \par
         *      \code
         *      camera()->connect(this, &Viewer::update);
         *      \endcode
         *  - If the function has been overloaded (of course you can always treat it this way):   \par
         *      \code
         *      camera()->connect(this, static_cast<void(Viewer::*)(void) const>(&Viewer::update));
         *      \endcode
         */
        template < class Class, class Function, class... Args >
        void connect(Class&& owner, Function&& func, Args&&... args)  {
            owned_slots_[owner] = std::bind(func, owner, std::forward<Args>(args)...);
        }

        /**
         * \brief Disconnect all the slots of a owner from this signal.
         */
        template < class Class >
        void disconnect(Class&& owner)  {
            owned_slots_.erase(owner);
        }

        /**
         * \brief Connect a free (i.e., not owned by any class) slot to this signal.
         * A unique index of the function must be provided (to differentiate with other free functions).
         */
        template < class Function, class... Args >
        void connect(int idx, Function&& func, Args&&... args) {
            free_slots_[idx] = std::bind(func, std::forward<Args>(args)...);
        }

        /**
         * \brief Disconnect a free slot function from this signal.
         * The slot is identified by its unique index.
         */
        void disconnect(int idx) { free_slots_.erase(idx); }

        /**
         * \brief Trigger all the execution of all connected slot functions.
         * \todo A better function name can be 'emit', but occupied by other software like Qt.
         *       Don't know how to still use 'emit'.
         */
        template < class ... Args >
        void send(Args&&... args) {
            for(auto it : owned_slots_) {
                it.second(std::forward<Args>(args)...);
            }
            for(auto it : free_slots_) {
                it.second(std::forward<Args>(args)...);
            }
        }

    private:
        // the key value is the pointer to the owner of the slot (to differentiate with slots of other owners)
        std::unordered_map<void*, std::function<void(void)> > owned_slots_;

        // the key value is the unique index of a slot function (to differentiate with other free functions)
        std::unordered_map<int, std::function<void(void)> > free_slots_;
	};


    template < class Class, class Function, class... Args >
    inline void connect(Signal* signal, Class&& owner, Function&& func, Args&&... args) {
        signal->connect(owner, std::bind(func, owner, std::forward<Args>(args)...));
    }

    template < class Function, class... Args >
    inline void connect(Signal* signal, int idx, Function&& func, Args&&... args) {
        signal->connect(idx, std::bind(func, std::forward<Args>(args)...));
    }

    template < class Class >
    inline void disconnect(Signal* signal, Class&& owner) {
        signal->disconnect(owner);
    }

    inline void disconnect(Signal* signal, int idx) {
        signal->disconnect(idx);
    }

}


#endif // EASY3D_CORE_SIGNAL_H
