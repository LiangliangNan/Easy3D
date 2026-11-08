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
     *      Scheduled work runs on detached worker threads, not on an event loop or the calling thread.
     *      The class does not synchronize access to its state: concurrent calls, or destroying an object while
     *      its member-scheduled work may still be running, are unsafe. Member callbacks also require the target
     *      object to remain alive until invocation. These methods do not provide a way to join their worker threads.
     */

    template<class... Args>
    class Timer {
    public:
        /// Constructs a timer in the active, unpaused state.
        Timer() : stopped_(false), paused_(false) {}

        /// Signals active member-scheduled work to stop; does not wait for worker threads to finish.
        ~Timer() { stopped_ = true; }

        /**
         * \brief Executes \p func once after \p delay milliseconds on a detached worker thread.
         * \param func The pointer to the function.
         * \param delay The time to be delayed, in milliseconds.
         * \param args The arguments to be passed to the function \p func.
         * \note The timer object is not used by this static overload. The callback and arguments are copied
         *       into the worker thread.
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (*)(const std::string&, int)>(&print). \endcode
         */
        static void single_shot(int delay, std::function<void(Args...)> const &func, Args... args);

        /**
         * \brief Executes a member function of \p inst once after \p delay milliseconds on a detached worker thread.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \param args The arguments to be passed to the function \p func.
         * \note This static overload cannot be stopped through a Timer instance; \p inst must remain alive
         *       until the callback has run.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        static void single_shot(int delay, Class *inst, void (Class::*func)(Args...), Args... args);

        /**
         * \brief Executes a const member function of \p inst once after \p delay milliseconds on a detached worker thread.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \param args The arguments to be passed to the function \p func.
         * \note This static overload cannot be stopped through a Timer instance; \p inst must remain alive
         *       until the callback has run.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        static void single_shot(int delay, Class const *inst, void (Class::*func)(Args...) const, Args... args);

        /**
         * \brief Schedules \p func to run once after \p delay milliseconds.
         * \details The callback runs on a detached worker thread and may be suppressed if stop() is observed
         *          before invocation. Scheduling this work resets the stopped state.
         * \param delay The time to be delayed, in milliseconds.
         * \param func The pointer to the function.
         * \param args The arguments to be passed to the function \p func.
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (*)(const std::string&, int)>(&print). \endcode
         */
        void set_timeout(int delay, std::function<void(Args...)> const &func, Args... args) const;

        /**
         * \brief Schedules a member function of \p inst to run once after \p delay milliseconds.
         * \details The callback runs on a detached worker thread and may be suppressed if stop() is observed
         *          before invocation. Scheduling this work resets the stopped state.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \param args The arguments to be passed to the function \p func.
         * \note \p inst must remain alive until the callback completes.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_timeout(int delay, Class *inst, void (Class::*func)(Args...), Args... args) const;

        /**
         * \brief Schedules a const member function of \p inst to run once after \p delay milliseconds.
         * \details The callback runs on a detached worker thread and may be suppressed if stop() is observed
         *          before invocation. Scheduling this work resets the stopped state.
         * \param delay The time to be delayed, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \param args The arguments to be passed to the function \p func.
         * \note \p inst must remain alive until the callback completes.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_timeout(int delay, Class const *inst, void (Class::*func)(Args...) const, Args... args) const;

        /**
         * \brief Repeatedly executes \p func at intervals of \p interval milliseconds.
         * \details The callback runs on a detached worker thread. pause() suppresses interval callbacks while
         *          paused; stop() signals the worker to exit. Scheduling resets the stopped state.
         * \param interval The interval, in milliseconds.
         * \param func The pointer to the function.
         * \param args The arguments to be passed to the function \p func.
         * \note When a function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (*)(const std::string&, int)>(&print). \endcode
         */
        void set_interval(int interval, std::function<void(Args...)> const &func, Args... args) const;

        /**
         * \brief Repeatedly executes a member function of \p inst at intervals of \p interval milliseconds.
         * \details The callback runs on a detached worker thread. pause() suppresses interval callbacks while
         *          paused; stop() signals the worker to exit. Scheduling resets the stopped state.
         * \param interval The interval, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \param args The arguments to be passed to the function \p func.
         * \note \p inst must remain alive while the interval is active.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_interval(int interval, Class *inst, void (Class::*func)(Args...), Args... args) const;

        /**
         * \brief Repeatedly executes a const member function of \p inst at intervals of \p interval milliseconds.
         * \details The callback runs on a detached worker thread. pause() suppresses interval callbacks while
         *          paused; stop() signals the worker to exit. Scheduling resets the stopped state.
         * \param interval The interval, in milliseconds.
         * \param inst The pointer to \c Class instance, e.g., '&a' for 'Class a' or 'this' within Class.
         * \param func The pointer to the member function of \c inst, e.g., '&Class::foo'.
         * \param args The arguments to be passed to the function \p func.
         * \note \p inst must remain alive while the interval is active.
         * \note When a member function has overloads, explicitly cast the function to the right function type using
         *     e.g., \code static_cast<void (Viewer::*)(const std::string&, int)>(&Viewer::print). \endcode
         */
        template<class Class>
        void set_interval(int interval, Class const *inst, void (Class::*func)(Args...) const, Args... args) const;

        /**
         * \brief Signals active member-scheduled work to stop.
         * \details This does not wait for detached workers to exit. A later scheduling call resets the stopped
         *          state; static single_shot() overloads are unaffected.
         * \sa  is_stopped(), pause().
         */
        void stop() { stopped_ = true; }

        /**
         * \brief Returns the timer's stopped-state flag.
         * \return True if stop() has been called and no later scheduling call has reset the flag.
         * \sa stop(), is_paused().
         */
        bool is_stopped() const { return stopped_; }

        /**
         * \brief Pauses interval callbacks.
         * \details This does not suspend set_timeout() callbacks or static single_shot() calls. Call resume()
         *          to clear the pause flag; call stop() to signal interval workers to exit.
         * \sa is_paused(), resume(), stop().
         */
        void pause() { paused_ = true; }

        /**
         * \brief Returns the timer's paused-state flag.
         * \return True if pause() has been called and resume() has not cleared the flag.
         * \sa pause(), resume(), is_stopped().
         */
        bool is_paused() const { return paused_; }

        /**
         * \brief Clears the paused state if the timer is paused and not stopped.
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
