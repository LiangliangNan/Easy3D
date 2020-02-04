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

#ifndef EASY3D_SIGNAL_H
#define EASY3D_SIGNAL_H


#include <unordered_map>
#include <functional>


namespace easy3d {

    /**
     * \brief Implementation of a simple signal-slot mechanism.
     *        Multiple slots (a class and it member function) can be connected
     *        to a signal object.
     *        You can connect functions to the signal which will be called when
     *        the trigger() method on the signal object is invoked. Any argument
     *        passed to emit() will be passed to the given functions.
     *        A typical usage of Signal in Easy3D is camera manipulation. When
     *        the camera has been manipulated, the viewer should be notified (e.g.,
     *        a repaint event should be triggered). This is done by calling
     *        to the viewer's update() function. So in Easy3D, the viewer's update
     *        function is connected to the camera.
     * \attention Current implementation can hold only one single function of each owner.
     *            (can be easily extended to multiple ones if needed).
     * \todo Extend this class to accept non-member functions and multiple functions of a class.
     */
    class Signal {
	public:
        /**
         * \brief Connect a slot to this signal.
         * \example
         *        - If no overloaded function:
         *              camera()->connect(this, &Viewer::update);
         *        - If the function has been overloaded (of course you can always treat it this way):
         *              camera()->connect(this, static_cast<void(Viewer::*)(void) const>(&Viewer::update));
         */
        template < class Class, class Function, class... Args >
        void connect(Class&& owner, Function&& func, Args&&... args)  {
            if (owner && func)
                slots_[owner] = std::bind(func, owner, std::forward<Args>(args)...);
        }

        /**
         * \brief Disconnect a slot from this signal.
         */
        template < class Class >
        void disconnect(Class&& owner)  {
            slots_.erase(owner);
        }

        /**
         * Trigger all the connected slots.
         * \todo A better function name can be 'emit', but occupied by other software like Qt.
         *       Don't know how to still use 'emit'.
         */
        template < class ... Args >
        void trigger(Args&&... args) {
            for(auto it : slots_) {
                it.second(std::forward<Args>(args)...);
            }
        }

    private:
        std::unordered_map<void*, std::function<void(void)>> slots_;
	};

}

#endif // EASY3D_SIGNAL_H
