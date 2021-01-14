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

#ifndef EASY3D_UTIL_LOGGING_H
#define EASY3D_UTIL_LOGGING_H


#include <easy3d/util/easylogging++.h>


// to have the same syntax as glog
#define LOG_FIRST_N LOG_N_TIMES

// to have LOG_IF_EVERY_N
#define LOG_IF_EVERY_N(n, condition, LEVEL)  if (condition) \
    CLOG_EVERY_N(n, LEVEL, ELPP_CURR_FILE_LOGGER_ID)

// for logging the counter number
#define COUNTER     ELPP_COUNTER->hitCounts()


namespace easy3d {

    /// \brief The logging mechanism.
    /// \namespace easy3d::logging
    namespace logging {

        /**
         * @brief Initializes the logging module.
         * @param verbose \c ture to log messages at a the INFO level to stderr.
         *      \c false only log at WARNING, ERROR, FATAL levels.
         * @param log_file A string specifying the full path to the log file.
         *      If valid, the log messages will be written to this file in addition to logging to stderr.
         *      No log file will be created if \p log_file is empty.
         *      Passing "default" allows to creat a log file with a title "ApplicationName.log" in a
         *      directory "logs" next to the executable file.
         * @note This initialization is optional. If not called, log messages will be written to stderr only.
         */
        void initialize(bool verbose = false, const std::string &log_file = "default");


        /// Base class for a logger (that can log messages to whatever)
        /// Users should subclass Logger and override send() to do whatever they want.
        /// \class Logger easy3d/util/logging.h
        class Logger {
        public:
            Logger();
            /// writes the log message \p msg (and may also other given information).
            virtual void send(el::Level level, const std::string& msg) = 0;
        };

    };

}



#endif  // EASY3D_UTIL_LOGGING_H
