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

#ifndef EASY3D_UTIL_STOP_WATCH_H
#define EASY3D_UTIL_STOP_WATCH_H

#include <string>

#ifdef _WIN32
#include <cstdint>
#else
#include <sys/time.h>
#endif // _WIN32


namespace easy3d {

    /* High resolution stop watch/timer.
     * This timer is able to measure the elapsed time with 1 micro-second accuracy
     * in both Windows, Linux, and Unix system.
     *
     * Usage example:
     *
     *      StopWatch w ;
     *      // do task_1 ...
     *      LOG(INFO) << "task_1 done. time: " << w.elapsed() << " seconds";
     *	   w.start();
     *      // do task_2 ...
     *      LOG(INFO) << "task_2 done. time: " << w.elapsed() << " seconds";
     *
     */

    class StopWatch {
    public:
        StopWatch(); // the watch will automatically start in construction
        ~StopWatch();

        void start();

        void restart(); // the same as start()

        // returns user elapsed time since the construction / start in seconds.
        double elapsed_seconds(int num_digits = 1) const;

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

