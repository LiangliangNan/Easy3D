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

#ifndef EASY3D_UTIL_TIMER_H
#define EASY3D_UTIL_TIMER_H

#include <thread>
#include <chrono>
#include <functional>

namespace easy3d {

    /**
     * \brief A light-weight implementation of the timer mechanism.
     * \class Timer easy3d/util/timer.h
     * \details Timer functionalities are usually implemented in large libraries
     *      (e.g., the [QTimer of Qt](http://doc.qt.io/archives/qt-5.5/qtimer.html)).
     *      This Timer class provides a single-header implementation.
     *      With Timer, tasks (i.e., calling to functions) can be easily scheduled at either constant intervals
     *      or after a specified period. Timer supports any types of functions with any number of arguments.
     *
     * \example Test_Timer  \include{lineno} test_timer.cpp
     */

    template<class... Args>
    class Timer {
    public:
        Timer() : stopped_(false), paused_(false) {}

        ~Timer() { stopped_ = true; }

        /**
         * \brief Executes function \p func after \p delay milliseconds.
         * \param func The pointer to the function.
         * \param delay The time to be delayed, in milliseconds.
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (*)(const std::string&, int)>(&print). \endcode
         */
        static void single_shot(int delay, std::function<void(Args...)> const &func, Args... args);

        /**
         * \brief Executes a member function \p func of \p Class \p inst after \p delay milliseconds.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        static void single_shot(int delay, Class *inst, void (Class::*func)(Args...), Args... args);

        /**
         * \brief Executes a const member function \p func of \p Class \p inst after \p delay milliseconds.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        static void single_shot(int delay, Class const *inst, void (Class::*func)(Args...) const, Args... args);

        /**
         * \brief Executes function \p func after \p delay milliseconds. 
         * \details This is the same as single_shot() except that it is not static.
         * \param delay The time to be delayed, in milliseconds.
         * \param func The pointer to the function.    
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (*)(const std::string&, int)>(&print). \endcode
         */
        void set_timeout(int delay, std::function<void(Args...)> const &func, Args... args) const;

        /**
         * \brief Executes a member function \p func of \p Class \p inst after \p delay milliseconds.
         * \details This is the same as single_shot() except that it is not static.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_timeout(int delay, Class *inst, void (Class::*func)(Args...), Args... args) const;

        /**
         * \brief Executes a const member function \p func of \p Class \p inst after \p delay milliseconds.
         * \details This is the same as single_shot() except that it is not static.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.    
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_timeout(int delay, Class const *inst, void (Class::*func)(Args...) const, Args... args) const;

        /**
         * \brief Executes function \p func for every \p interval milliseconds.
         * \param interval The interval, in milliseconds.
         * \param func The pointer to the function.
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (*)(const std::string&, int)>(&print). \endcode
         */
        void set_interval(int interval, std::function<void(Args...)> const &func, Args... args) const;

        /**
         * \brief Executes a member function \p func of \p Class \p inst for every \p interval milliseconds.
         * \param interval The interval, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_interval(int interval, Class *inst, void (Class::*func)(Args...), Args... args) const;

        /**
         * \brief Executes a const member function \p func of \p Class \p inst for every \p interval milliseconds.
         * \param interval The interval, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_interval(int interval, Class const *inst, void (Class::*func)(Args...) const, Args... args) const;

        /**
         * \brief Stops the timer. After a timer is stopped, it cannot be restarted again. If you want to
         *      temporarily pause a timer, call pause().
         * \sa  is_stopped(), pause().
         */
        void stop() { stopped_ = true; }

        /**
         * \brief Returns whether the timer has been stopped. If a timer is stopped, it cannot be restarted again.
         * \sa stop(), is_paused().
         */
        bool is_stopped() const { return stopped_; }

        /**
         * \brief Pauses the timer. After a timer is paused, it can be resumed by calling resume(). You can
         *      permanently stop the timer by stop().
         * \sa is_paused(), resume(), stop().
         */
        void pause() { paused_ = true; }

        /**
         * \brief Returns whether the timer has been paused. After a timer is paused, it can be resumed by
         *      calling resume()
         * \sa pause(), resume(), is_stopped().
         */
        bool is_paused() const { return paused_; }

        /**
         * \brief Resumes the timer. This will be effective only when the timer has been paused and not stopped.
         * \sa pause(), is_paused(), stop().
         */
        void resume() { if (!stopped_ && paused_) paused_ = false; }

    private:
        bool stopped_;
        bool paused_;
    };



    //-------------------------- IMPLEMENTATION ---------------------------


    template<class... Args>
    void Timer<Args...>::single_shot(int delay, std::function<void(Args...)> const &func, Args... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            func(args...);
        });
        t.detach();
    }


    template<class... Args>
    template<class Class>
    void Timer<Args...>::single_shot(int delay, Class *inst, void (Class::*func)(Args...), Args... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            (inst->*func)(args...);
        });
        t.detach();
    }


    template<class... Args>
    template<class Class>
    void Timer<Args...>::single_shot(int delay, Class const *inst, void (Class::*func)(Args...) const, Args... args) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            (inst->*func)(args...);
        });
        t.detach();
    }


    template<class... Args>
    void Timer<Args...>::set_timeout(int delay, std::function<void(Args...)> const &func, Args... args) const {
        const_cast<Timer<Args...>*>(this)->stopped_ = false;
        std::thread t([=]() {
            if (stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if (stopped_) return;
            func(args...);
        });
        t.detach();
    }


    template<class... Args>
    template<class Class>
    void Timer<Args...>::set_timeout(int delay, Class *inst, void (Class::*func)(Args...), Args... args) const {
        const_cast<Timer<Args...>*>(this)->stopped_ = false;
        std::thread t([=]() {
            if (stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if (stopped_) return;
            (inst->*func)(args...);
        });
        t.detach();
    }


    template<class... Args>
    template<class Class>
    void Timer<Args...>::set_timeout(int delay, Class const *inst, void (Class::*func)(Args...) const, Args... args) const {
        const_cast<Timer<Args...>*>(this)->stopped_ = false;
        std::thread t([=]() {
            if (stopped_) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            if (stopped_) return;
            (inst->*func)(args...);
        });
        t.detach();
    }


    template<class... Args>
    void Timer<Args...>::set_interval(int interval, std::function<void(Args...)> const &func, Args... args) const {
        const_cast<Timer<Args...>*>(this)->stopped_ = false;
        std::thread t([=]() {
            while (true) {
                if (stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if (paused_) continue;
                else if (stopped_) return;
                func(args...);
            }
        });
        t.detach();
    }


    template<class... Args>
    template<class Class>
    void Timer<Args...>::set_interval(int interval, Class *inst, void (Class::*func)(Args...), Args... args) const {
        const_cast<Timer<Args...>*>(this)->stopped_ = false;
        std::thread t([=]() {
            while (true) {
                if (stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if (paused_) continue;
                else if (stopped_) return;
                (inst->*func)(args...);
            }
        });
        t.detach();
    }

    template<class... Args>
    template<class Class>
    void Timer<Args...>::set_interval(int interval, Class const *inst, void (Class::*func)(Args...) const, Args... args) const {
        const_cast<Timer<Args...>*>(this)->stopped_ = false;
        std::thread t([=]() {
            while (true) {
                if (stopped_) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if (paused_) continue;
                else if (stopped_) return;
                (inst->*func)(args...);
            }
        });
        t.detach();
    }


} // namespace easy3d


#endif  // EASY3D_UTIL_TIMER_H
