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

#ifndef EASY3D_UTIL_STACK_TRACER_H
#define EASY3D_UTIL_STACK_TRACER_H

#include <string>
#include <vector>

namespace easy3d {


    /**
     * \brief Tracks and dumps the stack frames when program fails.
     * \class StackTracer easy3d/util/stack_tracer.h
     *
     * Track failure example:
     *      \code
     *          #include <easy3d/util/stack_tracer.h>
     *          int main() {
     *              easy3d::StackTracer st;
     *              int a = 1;
     *              a /= 0;
     *              return 0;
     *          }
     *      \endcode
     *
     * Read recent stack activities example:
     *      \code
     *          std::cout << StackTracer::back_trace_string();
     *      \endcode
     */
    class StackTracer {
    public:
        StackTracer();
        ~StackTracer();

        class StackEntry {
        public:
            StackEntry(
                    const std::string &object,
                    const std::string &function,
                    const std::string &file,
                    unsigned int line,
                    const void *addr
            ) : object_name(object), function_name(function), file_name(file), line_number(line), address(addr) {}
            std::string object_name;
            std::string function_name;
            std::string file_name;
            unsigned int line_number;
            const void* address;
        private:
            StackEntry(void);
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
