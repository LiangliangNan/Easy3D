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

#ifndef EASY3D_UTIL_TIMER_H
#define EASY3D_UTIL_TIMER_H

#include <thread>
#include <chrono>
#include <functional>

namespace easy3d {

    /*
     * Timer functionalities are usually implemented in large libraries
     * (e.g., the [QTimer of Qt](http://doc.qt.io/archives/qt-5.5/qtimer.html)).
     * This Timer class provides a single-header implementation.
     *
     * With Timer, tasks (i.e., calling to functions) can be easily
     * scheduled at either constant intervals or after a specified
     * period. Timer supports any types of functions with any number
     * of arguments.
     *
     ** Code examples:
     *### Example 1: calling to non-member functions
     *   // the function you want to call at a constant interval
     *   void foo(int value);
     *
     *   // the function to be called when timeout.
     *   void timeout();
     *
     *   const int value = 333;
     *   // function "foo" will be executed every second.
     *   t.set_interval(1000, &foo, value);
     *
     *   // function "timeout()" will be executed after 3 seconds.
     *   t.set_timeout(3000, &timeout);
     *
     *### Example 2: calling to member functions
     *   // a trivial class
     *   class Car {
     *   public:
     *       Car(const std::string& name, float speed);
     *       void print_speed();
     *       void stop();
     *   };
     *
     *   // car 1 reports its speed every 2 seconds and it stops after 10 seconds
     *   Car car1("BMW", 180);
     *   t.set_interval(2000, &Car::print_speed, &car1);
     *   t.set_timeout(10000, &Car::stop, &car1);
     *
     *   // car 2 reports its speed every 3 seconds and it stops after 20 seconds
     *   Car car2("Chevrolet", 120);
     *  t.set_interval(3000, &Car::print_speed, &car2);
     *  t.set_timeout(20000, &Car::stop, &car2);
     *
     *### Example 3: calling to lambda functions
     *  const float value = 5;
     *  t.set_interval(3000, [&](float, const std::string&) {
     *      std::cout << "After every 3 sec. value: " << value << ", message: " << msg << std::endl;
     *  }, value, msg);
     *
     *  t.set_timeout(8000, [&]() {
     *      t.stop();
     *      std::cout << "After 8 sec, the timer is stopped!" << std::endl;
     *  });
     */

    class Timer {
    public:
        Timer() : stopped_(false) {}

        /* execute function 'func' after 'delay' milliseconds.
         * func:  the pointer to the function.
         * delay: the time to be delayed, in milliseconds.
         * NOTE: arguments must be "const", e.g.,
         *      const int taskID = 3;
         *      const std::string msg = "Finished";
         *      Timer::single_shot(5000, &reset, taskID, msg);
         * so this won't work
         *      Timer::single_shot(5000, &reset, 3, "Finished");
         */
        template < class Function, class... Args >
        static void single_shot(int delay, Function&& func, Args&&... args);

        /* execute member function 'func' of class 'owner' after 'delay' milliseconds.
         * owner: the pointer to 'Class' instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * func:  the pointer to the member function of 'Class', e.g., '&Class::foo()'.
         * delay: the time to be delayed, in milliseconds.
         * NOTE: arguments must be "const", e.g.,
         *      const int taskID = 3;
         *      const std::string msg = "Finished";
         *      Timer::single_shot(5000, &Viewer::reset, viewer, taskID, msg);
         * so this won't work
         *      Timer::single_shot(5000, &Viewer::reset, viewer, 3, "Finished");
         */
        template < class Class, class Function, class... Args >
        static void single_shot(int delay, Function&& func, Class&& owner, Args&&... args);

        /* the same as single_shot() except that it is not static.
         * execute function 'func' after 'delay' milliseconds.
         * func:  the pointer to the function.
         * delay: the time to be delayed, in milliseconds.
         * NOTE: arguments must be "const", e.g.,
         *      const int taskID = 3;
         *      const std::string msg = "Finished";
         *      timer.set_timeout(5000, &reset, taskID, msg);
         * so this won't work
         *      timer.set_timeout(5000, &reset, 3, "Finished");
         */
        template <class Function, class... Args >
        void set_timeout(int delay, Function&& func, Args&&... args);

        /* the same as single_shot() except that it is not static.
         * execute member function 'func' of class 'owner' after 'delay' milliseconds.
         * owner: the pointer to 'Class' instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * func:  the pointer to the member function of 'Class', e.g., '&Class::foo()'.
         * delay: the time to be delayed, in milliseconds.
         * NOTE: arguments must be "const", e.g.,
         *      const int taskID = 3;
         *      const std::string msg = "Finished";
         *      timer.set_timeout(5000, viewer, &Viewer::reset, taskID, msg);
         * so this won't work
         *      timer.set_timeout(5000, viewer, &Viewer::reset, 3, "Finished");
         */
        template < class Class, class Function, class... Args >
        void set_timeout(int delay, Function&& func, Class&& owner, Args&&... args);

        /* execute function ‘func' for every 'interval' milliseconds.
         * func:  the pointer to the function.
         * interval: the interval, in milliseconds.
         * NOTE: arguments must be "const", e.g.,
         *      const std::string speed = "Speed: ";
         *      timer.set_interval(10000, &print_speed, speed);
         * so this won't work
         *      timer.set_interval(10000, &print_speed, "Speed: ");
         */
        template <class Function, class... Args >
        void set_interval(int interval, Function&& func, Args&&... args);

        /* execute member function 'func' of class 'owner' for every 'interval' milliseconds.
         * owner: the pointer to 'Class' instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * func:  the pointer to the member function of 'Class', e.g., '&Class::foo()'.
         * interval: the interval, in milliseconds.
         * NOTE: arguments must be "const", e.g.,
         *      const std::string speed = "Speed: ";
         *      timer.set_interval(10000, &Car::print_speed, car, speed);
         * so this won't work
         *      timer.set_interval(10000, &Car::print_speed, car, "Speed: ");
         */
        template < class Class, class Function, class... Args >
        void set_interval(int interval, Function&& func, Class&& owner, Args&&... args);

        // stop the timer.
        void stop() { stopped_ = true; }

    private:
       bool stopped_;
    };



    //-------------------------- IMPLEMENTATION ---------------------------


    template <class Function, class... Args >
    void Timer::single_shot(int delay, Function&& func, Args&&... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            func(std::forward<Args>(args)...);
        });
        t.detach();
    }


    template < class Class, class Function, class... Args >
    void Timer::single_shot(int delay, Function&& func, Class&& owner, Args&&... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
#if 0 // this has ambiguities in determine which function
                (owner->*func)(std::forward<Args>(args)...);
#else
                // store a call to a member function and object ptr
                auto f = std::bind(func, owner, std::forward<Args>(args)...);
                f(std::forward<Args>(args)...);
#endif
        });
        t.detach();
    }


    template <class Function, class... Args >
    void Timer::set_timeout(int delay, Function&& func, Args&&... args) {
        stopped_ = false;
        std::thread t([=]() {
            if(stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if(stopped_) return;
            func(std::forward<Args>(args)...);
        });
        t.detach();
    }


    template < class Class, class Function, class... Args >
    void Timer::set_timeout(int delay, Function&& func, Class&& owner, Args&&... args) {
        stopped_ = false;
        std::thread t([=]() {
            if(stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if(stopped_) return;
#if 0 // this has ambiguities in determine which function
            (owner->*func)(std::forward<Args>(args)...);
#else
            // store a call to a member function and object ptr
            auto f = std::bind(func, owner, std::forward<Args>(args)...);
            f(std::forward<Args>(args)...);
#endif
        });
        t.detach();
    }


    template <class Function, class... Args >
    void Timer::set_interval(int interval, Function&& func, Args&&... args) {
        stopped_ = false;
        std::thread t([=]() {
            while(true) {
                if(stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(stopped_) return;
                func(std::forward<Args>(args)...);
            }
        });
        t.detach();
    }


    template < class Class, class Function, class... Args >
    void Timer::set_interval(int interval, Function&& func, Class&& owner, Args&&... args) {
        stopped_ = false;
        std::thread t([=]() {
            while(true) {
                if(stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(stopped_) return;
#if 0 // this has ambiguities in determine which function
                (owner->*func)(std::forward<Args>(args)...);
#else
                // store a call to a member function and object ptr
                auto f = std::bind(func, owner, std::forward<Args>(args)...);
                f(std::forward<Args>(args)...);
#endif
            }
        });
        t.detach();
    }


} // namespace easy3d


#endif  // EASY3D_UTIL_TIMER_H
