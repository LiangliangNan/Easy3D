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

#ifndef EASY3D_UTIL_CHRONO_WATCH_H
#define EASY3D_UTIL_CHRONO_WATCH_H


#include <chrono>
#include <ostream>
#include <string>

namespace easy3d {

    /**
     * \brief A timer based on <code> std::chrono::high_resolution_clock </code>.
     *
     * \class ChronoWatch easy3d/util/chrono_watch.h
     * \see StopWatch
     *
     * \note \c std::chrono::high_resolution_clock has disappointing accuracy on windows (on windows, one
     * should use the WinAPI \c QueryPerformanceFrequency()). If you care much about accuracy, use the
     * StopWatch class instead.
     *
     * Usage example:
     *      \code
     *      ChronoWatch t;
     *	    t.start();
     *      // do task_1 ...
     *      LOG(INFO) << "task_1 done. Time: " << t.time_string();
     *	    t.reset(), t.start();
     *      // do task_2 ...
     *      LOG(INFO) << "task_1 done. Time: " << t.time_string();
     *      \endcode
     */
    class ChronoWatch
    {
    public:
        /// default constructor
        ChronoWatch();
        /// destructor
        ~ChronoWatch() {}

        /// starts the timer
        void start();
        /// restarts the timer
        void restart();

        /// pauses the timer
        void pause();
        /// resumes the timer
        void resume();

        /// resets the timer
        void reset();

        /// the elapsed time in milliseconds
        float elapsed_milliseconds(int num_digits = 1) const;
        /// the elapsed time in seconds
        float elapsed_seconds(int num_digits = 1) const;
        /// the elapsed time in minutes
        float elapsed_minutes(int num_digits = 1) const;
        /// the elapsed time in hours
        float elapsed_hours(int num_digits = 1) const;

        /// prints the elapsed seconds to \c output stream.
        void print_seconds(std::ostream& output) const;
        /// prints the elapsed minutes to \c output stream.
        void print_minutes(std::ostream& output) const;
        /// prints the elapsed hours to \c output stream.
        void print_hours(std::ostream& output) const;

        /// the elapsed time string, e.g., 88ms, 2.3s, 1.7m, 0.1h. This function automatically determines the best unit.
        std::string time_string(int num_digits = 1) const;

    private:
        std::size_t duration_count() const;

    private:
        bool started_;
        std::chrono::high_resolution_clock::time_point start_time_;

        bool paused_;
        std::chrono::high_resolution_clock::time_point pause_time_;
    };

}

#endif  // EASY3D_UTIL_CHRONO_WATCH_H
