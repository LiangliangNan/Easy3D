/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <easy3d/util/stop_watch.h>

#include <cmath>
#include <sstream>
#include <iomanip>

#include <easy3d/core/types.h>

// The windows.h has to come after <easy3d/core/types.h>. Otherwise the compiler
// will be confused by the min/max micros and the std::min, std::max.
#ifdef _WIN32
#include <Windows.h>	
#endif // _WIN32


namespace easy3d {

    StopWatch::StopWatch() {
        start();
    }

    StopWatch::~StopWatch() {}


    void StopWatch::start() {
    #ifdef _WIN32
        LARGE_INTEGER  largeInteger;
        QueryPerformanceFrequency(&largeInteger);
        freq_ = largeInteger.QuadPart;
        QueryPerformanceCounter(&largeInteger);
        start_count_ = largeInteger.QuadPart;
    #else
        gettimeofday(&start_time_, nullptr);
    #endif // _WIN32
    }


    void StopWatch::restart() {
        start();
    }


    double StopWatch::seconds() const {
    #ifdef _WIN32
        LARGE_INTEGER  largeInteger;
        QueryPerformanceCounter(&largeInteger);
        LONGLONG now_count = largeInteger.QuadPart;
        return static_cast<double>((now_count - start_count_) / static_cast<double>(freq_));
    #else
        timeval now;
        gettimeofday(&now, nullptr);
        return (now.tv_sec - start_time_.tv_sec) + (now.tv_usec - start_time_.tv_usec) / 1.0e6;
    #endif  // _WIN32
    }


    double StopWatch::elapsed_seconds(int num_digits /* = 1*/) const {
        return geom::truncate_digits(seconds(), num_digits);
    }


    std::string StopWatch::time_string(int num_digits /* = 1*/) const {
        double time = seconds() * 1000;
        if (std::isnan(time) || std::isinf(time))
            return "inf";

        std::string suffix = "ms";
        if (time > 1000) {
            time /= 1000; suffix = "s";
            if (time > 60) {
                time /= 60; suffix = "m";
                if (time > 60) {
                    time /= 60; suffix = "h";
                    if (time > 12) {
                        time /= 12; suffix = "d";
                    }
                }
            }
        }

        std::ostringstream os;
        os << std::setprecision(num_digits)
            << std::fixed << time << suffix;

        return os.str();
    }

} // namespace easy3d
