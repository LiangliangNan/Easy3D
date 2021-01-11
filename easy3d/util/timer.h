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

    /**
     * \brief An light-weight implementation of the timer mechanism.
     *
     * \class Timer easy3d/util/timer.h
     *
     * \details Timer functionalities are usually implemented in large libraries
     * (e.g., the [QTimer of Qt](http://doc.qt.io/archives/qt-5.5/qtimer.html)).
     * This Timer class provides a single-header implementation.
     *
     * With Timer, tasks (i.e., calling to functions) can be easily scheduled at either constant intervals or after
     * a specified period. Timer supports any types of functions with any number of arguments.
     *
     ** Code examples:
     *### Example 1: calling to non-member functions
     *   // the function you want to call at a constant interval
     *      \code
     *          void foo(int value);
     *      
     *          // the function to be called when timeout.
     *          void timeout();
     *      
     *          const int value = 333;
     *          // function "foo" will be executed every second.
     *          t.set_interval(1000, &foo, value);
     *      
     *          // function "timeout()" will be executed after 3 seconds.
     *          t.set_timeout(3000, &timeout);
     *      \endcode
     *
     *### Example 2: calling to member functions
     *      \code
     *          // a trivial class
     *          class Car {
     *          public:
     *              Car(const std::string& name, float speed);
     *              void print_speed();
     *              void stop();
     *          };
     *      
     *          // car 1 reports its speed every 2 seconds and it stops after 10 seconds
     *          Car car1("BMW", 180);
     *          t.set_interval(2000, &car1, &Car::print_speed);
     *          t.set_timeout(10000, &car1, &Car::stop);
     *      
     *          // car 2 reports its speed every 3 seconds and it stops after 20 seconds
     *          Car car2("Chevrolet", 120);
     *          t.set_interval(3000, &car2, &Car::print_speed);
     *          t.set_timeout(20000, &car2, &Car::stop);
     *      \endcode
     *
     *### Example 3: calling to lambda functions
     *      \code
     *          const float value = 5;
     *          t.set_interval(3000, [&](float, const std::string&) {
     *              std::cout << "After every 3 sec. value: " << value << ", message: " << msg << std::endl;
     *          }, value, msg);
     *      
     *          t.set_timeout(8000, [&]() {
     *              t.stop();
     *              std::cout << "After 8 sec, the timer is stopped!" << std::endl;
     *          });
     *      \endcode
     */

    template < class... Args >
    class Timer {
    public:
        Timer() : stopped_(false) {}

        /**
         * \brief Executes function \p func after \p delay milliseconds.
         * \param func The pointer to the function.
         * \param delay The time to be delayed, in milliseconds.
         * \note Arguments must be \c const. When an argument is a pointer, it has to be of type "Class* const".
         *      \code
         *          class MyClass {};
         *          MyClass* const data = new MyClass;   // MyClass* data = new MyClass; will not work
         *          const int taskID = 3;
         *          const std::string msg = "Finished";
         *          Timer::single_shot(5000, &reset, taskID, msg, data);
         *      \endcode
         * So this won't work
         *      \code
         *          Timer::single_shot(5000, &reset, 3, "Finished", data);
         *      \endcode
         */
        static void single_shot(int delay, std::function<void(Args...)> const &func, Args... args);

        /**
         * \brief Executes member function \p func of class \p owner after \p delay milliseconds.
         * \param delay The time to be delayed, in milliseconds.
         * \param owner The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c owner, e.g., '&Class::foo()'.
         * \note Arguments must be \c const. When an argument is a pointer, it has to be of type "Class* const".
         *      \code
         *          class MyClass {};
         *          MyClass* const data = new MyClass;   // MyClass* data = new MyClass; will not work
         *          const int taskID = 3;
         *          const std::string msg = "Finished";
         *          Timer::single_shot(5000, viewer, &Viewer::reset, taskID, msg, data);
         *      \endcode    
         * so this won't work
         *      \code
         *          Timer::single_shot(5000, viewer, &Viewer::reset, 3, "Finished", data);
         *      \endcode
         */
        template < class Class >
        static void single_shot(int delay, Class* inst, void (Class::*func)(Args...), Args... args);
        template < class Class >
        static void single_shot(int delay, Class* inst, void (Class::*func)(Args...) const, Args... args);

        /**
         * \brief Executes function \p func after \p delay milliseconds. 
         * \details This is the same as single_shot() except that it is not static.
         * \param delay The time to be delayed, in milliseconds.
         * \param func The pointer to the function.
         * \note Arguments must be \c const. When an argument is a pointer, it has to be of type "Class* const".
         *      \code
         *          class MyClass {};
         *          MyClass* const data = new MyClass;   // MyClass* data = new MyClass; will not work
         *          const int taskID = 3;
         *          const std::string msg = "Finished";
         *          timer.set_timeout(5000, &reset, taskID, msg, data);
         *      \endcode    
         * so this won't work
         *      \code
         *          timer.set_timeout(5000, &reset, 3, "Finished", data);
         *      \endcode       
         */
        void set_timeout(int delay, std::function<void(Args...)> const &func, Args... args) const;

        /**
         * \brief Executes member function \p func of class 'owner' after \p delay milliseconds.
         * \details This is the same as single_shot() except that it is not static.
         * \param delay The time to be delayed, in milliseconds.
         * \param owner The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c owner, e.g., '&Class::foo()'.
         * \note Arguments must be \c const. When an argument is a pointer, it has to be of type "Class* const".
         *      \code
         *          class MyClass {};
         *          MyClass* const data = new MyClass;   // MyClass* data = new MyClass; will not work
         *          const int taskID = 3;
         *          const std::string msg = "Finished";
         *          timer.set_timeout(5000, viewer, &Viewer::reset, taskID, msg, data);
         *      \endcode    
         * so this won't work
         *      \code
         *          timer.set_timeout(5000, viewer, &Viewer::reset, 3, "Finished", data);
         *      \endcode       
         */
        template < class Class >
        void set_timeout(int delay, Class* inst, void (Class::*func)(Args...), Args... args) const;

        /**
         * \brief Executes function ‘func' for every 'interval' milliseconds.
         * \param interval The interval, in milliseconds.
         * \param func The pointer to the function.
         * \note Arguments must be \c const. When an argument is a pointer, it has to be of type "Class* const".
         *      \code
         *          class MyClass {};
         *          MyClass* const data = new MyClass;   // MyClass* data = new MyClass; will not work
         *          const std::string speed = "Speed: ";
         *          timer.set_interval(10000, &print_speed, speed, data);
         *      \endcode    
         * so this won't work
         *      \code
         *          timer.set_interval(10000, &print_speed, "Speed: ", data);
         *      \endcode       
         */
        void set_interval(int interval, std::function<void(Args...)> const &func, Args... args);

        /**
         * \brief Executes member function \p func of class 'owner' for every 'interval' milliseconds.
         * \param interval The interval, in milliseconds.
         * \param owner The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c owner, e.g., '&Class::foo()'.
         * \note Arguments must be \c const. When an argument is a pointer, it has to be of type "Class* const".
         *      \code
         *          class MyClass {};
         *          MyClass* const data = new MyClass;   // MyClass* data = new MyClass; will not work
         *          const std::string speed = "Speed: ";
         *          timer.set_interval(10000, car, &Car::print_speed, speed, data);
         *      \endcode    
         * so this won't work
         *      \code
         *          timer.set_interval(10000, car, &Car::print_speed, "Speed: ", data);
         *      \endcode       
         */
        template < class Class >
        void set_interval(int interval, Class* inst, void (Class::*func)(Args...), Args... args);

        /** \brief Stops the timer. */
        void stop() { stopped_ = true; }

        /** \brief Returns whether the timer has been stopped. */
        bool is_stopped() const { return stopped_; }

    private:
       mutable bool stopped_;
    };



    //-------------------------- IMPLEMENTATION ---------------------------


    template < class... Args >
    void Timer<Args...>::single_shot(int delay, std::function<void(Args...)> const &func, Args... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            func(args...);
        });
        t.detach();
    }


    template < class... Args >
    template < class Class >
    void Timer<Args...>::single_shot(int delay, Class* inst, void (Class::*func)(Args...), Args... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            (inst->*func)(args...);
        });
        t.detach();
    }


    template < class... Args >
    template < class Class >
    void Timer<Args...>::single_shot(int delay, Class* inst, void (Class::*func)(Args...) const, Args... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            (inst->*func)(args...);
        });
        t.detach();
    }


    template < class... Args >
    void Timer<Args...>::set_timeout(int delay, std::function<void(Args...)> const &func, Args... args) const {
        stopped_ = false;
        std::thread t([=]() {
            if(stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if(stopped_) return;
            func(args...);
        });
        t.detach();
    }


    template < class... Args >
    template < class Class >
    void Timer<Args...>::set_timeout(int delay, Class* inst, void (Class::*func)(Args...), Args... args) const {
        stopped_ = false;
        std::thread t([=]() {
            if(stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if(stopped_) return;
            (inst->*func)(args...);
        });
        t.detach();
    }


    template < class... Args >
    void Timer<Args...>::set_interval(int interval, std::function<void(Args...)> const &func, Args... args) {
        stopped_ = false;
        std::thread t([=]() {
            while(true) {
                if(stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(stopped_) return;
                func(args...);
            }
        });
        t.detach();
    }


    template < class... Args >
    template < class Class >
    void Timer<Args...>::set_interval(int interval, Class* inst, void (Class::*func)(Args...), Args... args) {
        stopped_ = false;
        std::thread t([=]() {
            while(true) {
                if(stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if(stopped_) return;
                (inst->*func)(args...);
            }
        });
        t.detach();
    }


} // namespace easy3d


#endif  // EASY3D_UTIL_TIMER_H
