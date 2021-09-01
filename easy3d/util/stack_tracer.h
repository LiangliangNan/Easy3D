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

#ifndef EASY3D_UTIL_STACK_TRACER_H
#define EASY3D_UTIL_STACK_TRACER_H

#include <string>
#include <vector>

namespace easy3d {

    /**
     * \brief Tracks stack frames.
     * \class StackTracer easy3d/util/stack_tracer.h
     * Read recent stack activities example:
     *      \code
     *          std::cout << StackTracer::back_trace_string();
     *      \endcode
     */
    class StackTracer {
    public:

        /// A record of a stack frame
        struct StackEntry {
            StackEntry(const std::string &object, const std::string &function)
                    : object_name(object), function_name(function) {}
            std::string object_name;
            std::string function_name;
        };

        /// This function produces a stack backtrace with demangled function & method names.
        /// \param amount The number of frames to retrieve. Default value is 32. A negative value retrieve all record.
        /// \param skip The number of the most recent frames to skip.
        static std::vector<StackEntry> back_trace(int amount = 32, int skip = 2);

        /// Returns the back_trace() record as a string, line by line with an index and most recent one first.
        static std::string back_trace_string(int amount = 32, int skip = 2);

        /// Converts the back_trace() record to a string.
        static std::string to_string(const std::vector<StackEntry>& record);
    };

}


#endif  // EASY3D_UTIL_STACK_TRACER_H
