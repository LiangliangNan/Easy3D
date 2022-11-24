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

#ifndef EASY3D_UTIL_STOP_WATCH_H
#define EASY3D_UTIL_STOP_WATCH_H

#include <string>

#ifdef _WIN32
#include <cstdint>
#else
#include <ctime>
#endif // _WIN32


namespace easy3d {

    /**
     * \brief A high resolution stop watch/timer.
     * \details This timer is able to measure the elapsed time with 1 micro-second accuracy on Windows, Linux, and Unix.
     *  Usage example:
     *      \code
     *          StopWatch w ;
     *          // do task_1 ...
     *          LOG(INFO) << "task_1 done. Time: " << w.time_string() << " seconds";
     *	        w.start();
     *          // do task_2 ...
     *          LOG(INFO) << "task_2 done. Time: " << w.time_string() << " seconds";
     *      \endcode
     * \class StopWatch easy3d/util/stop_watch.h
     */
    class StopWatch {
    public:
        /// default constructor. The watch will automatically start after construction.
        StopWatch();
        /// destructor.
        ~StopWatch() = default;

        /// starts the timer
        void start();
        /// restarts the timer. It has the same effect as start()
        void restart();

        /// returns user elapsed time (in seconds) since the construction / start.
        double elapsed_seconds(int num_digits = 1) const;

        /// the elapsed time string, e.g., 88ms, 2.3s, 1.7m, 0.1h. This function automatically determines the best unit.
        std::string time_string(int num_digits = 1) const;

    private:
        double seconds() const;

#ifdef _WIN32
        int64_t  freq_;
        int64_t  start_count_;
#else
        timeval start_time_;
#endif

    };

} // namespace easy3d


#endif  // EASY3D_UTIL_STOP_WATCH_H

