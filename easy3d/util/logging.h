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

#ifndef EASY3D_UTIL_LOGGING_H
#define EASY3D_UTIL_LOGGING_H


#include <3rd_party/easyloggingpp/easylogging++.h>


/// To have LOG_IF_EVERY_N
/// Example: LOG_IF_EVERY_N(5, i < 20, WARNING) << "Log if (i < 10) for every 5, i = " << i << ", " << COUNTER;
#define LOG_IF_EVERY_N(n, condition, LEVEL)  if (condition) \
    CLOG_EVERY_N(n, LEVEL, ELPP_CURR_FILE_LOGGER_ID)

/// To have LOG_IF_FIRST_N
/// Example: LOG_IF_FIRST_N(5, i < 20, WARNING) << "Log if (i < 10) for first 5, i = " << i << ", " << COUNTER;
#define LOG_IF_FIRST_N(n, condition, LEVEL)  if (condition) \
    CLOG_N_TIMES(n, LEVEL, ELPP_CURR_FILE_LOGGER_ID)

/// For logging the counter number
#define COUNTER     ELPP_COUNTER->hitCounts()


namespace easy3d {

    /// \brief The logging mechanism.
    /// \namespace easy3d::logging
    namespace logging {

        /**
         * @brief Initializes the logging module.
         * @param info_to_stdout \c true to log messages at a the \c INFO level to standard output.
         * @param warning_to_stdcout \c true to log messages at a the \c WARNING level to standard output.
         * @param error_to_stdcout \c true to log messages at a the \c ERROR (including \c FATAL) level to standard output.
         * @param verbose_to_stdcout \c true to log messages at a the \c ERROR (including \c FATAL) level to standard output.
		 * @param log_file A string specifying the name of log file. Three different values are accepted:
         *    - "": an empty string, indicating that no log file will be created.
		 *    - "default": create a log file with a title in the form "AppName.log" next to the executable file.
		 *    - any non-empty string: (if it is a valid path) a log file with the same name will be created and log
         *      messages will be written to this file.
         * @param verbosity_threshold A \c VLOG(level) with \c level <= \c verbosity_threshold will be written into the
         *      log file (if specified). Value must be in the range [0, 9]. Default is 0.
         * @note This initialization is optional. If not called, log messages will be written to standard output only.
         */
        void initialize(
                bool info_to_stdout = false,
                bool warning_to_stdcout = true,
                bool error_to_stdcout = true,
                bool verbose_to_stdcout = false,
                const std::string &log_file = "default",
                int verbosity_threshold = 9
        );

        /// Returns whether the logging has been initialized.
        bool is_initialized();

        /// Returns the full path of the log file (empty if no log file has been created).
        std::string log_file();

        /// Base class for a logger (that can log messages to whatever)
        /// Users should subclass Logger and override send() to do whatever they want.
        /// \class Logger easy3d/util/logging.h
        class Logger {
        public:
            Logger();
            /// writes the log message \p msg (and may also other given information).
            virtual void send(el::Level level, const std::string& msg) = 0;
        };

    }

}



#endif  // EASY3D_UTIL_LOGGING_H
