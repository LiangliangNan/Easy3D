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

#ifndef EASY3D_UTIL_THREAD_H
#define EASY3D_UTIL_THREAD_H

#include <thread>


namespace easy3d {


    class Thread
    {
    public:
        Thread() {}

        // run member function 'func' of class 'owner' in a separate thread.
        // owner: the pointer to 'Class' instance, e.g., '&a' for 'Class a' or 'this'
        //        within the class.
        // func:  the pointer to the member function of 'Class', e.g., '&Class::foo()'.
        /* NOTE: arguments must be "const", e.g.,
         * Example:
         *      const std::string file_name = "./building.ply";
         *      Thread::run(viewer, &Viewer::open_file, file_name);
         * so this won't work
         *      Thread::run(viewer, &Viewer::open_file, "./building.ply");
         */
        template < class Class, class Function, class... Args >
        static void run(Class&& owner, Function&& func, Args&&... args);
    };



    //-------------------------- IMPLEMENTATION ---------------------------



    template < class Class, class Function, class... Args >
    void Thread::run(Class&& owner, Function&& func, Args&&... args) {
        std::thread t([=]() {
            auto f = std::bind(func, owner, std::forward<Args>(args)...);
            f(std::forward<Args>(args)...);
        });
        t.detach();
    }


}

#endif // EASY3D_UTIL_THREAD_H
