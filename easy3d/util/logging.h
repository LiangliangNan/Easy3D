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


/* Make it easier for including the Google glog header file.
 *
 * Always include this file, i.e.,
 *      #include <easy3d/util/logging.h>
 * instead of including any other header files, because the third parties may change.
 */

#include <3rd_party/glog/glog/logging.h>
#include <3rd_party/glog/glog/stl_logging.h>    // to allow logging STL containers


/** If failed, a fatal error will be logged */
#define CHECK_TRUE(val)     CHECK_EQ((val), true)
#define CHECK_FALSE(val)    CHECK_EQ((val), false)

namespace easy3d
{

    /// \brief The logging mechanism.
    /// \namespace easy3d::logging
	namespace logging
	{

		/**
		 * @brief Initializes Google's logging library. The log will be writen to log files.
		 * @param severity_dependent True to create a log file for each severity, otherwise a single log file each run.
		 * @param log_dir The directory where the log files will be created. If it is empty (default value), the log
		 * 		  files will be created in the "logs" directory next the executable file.
		 * @param stderr_threshold Log messages at a level >= this flag are automatically sent to standard error
		 *        (stderr) in addition to log files. Acceptable values are
		 *          - google::GLOG_INFO
		 *          - google::GLOG_WARNING
		 *          - google::GLOG_ERROR
		 * @note Initialization of Google's logging library is not mandatory. Logging before initialize() will be
		 * 		 written to stderr.
		 */
		void initialize(bool severity_dependent = false, const std::string& log_dir = "", int stderr_threshold = google::GLOG_WARNING);


        // Base class for a log client.
        // Users should subclass LogClient and override output to do whatever they want.
        class LogClient : public google::LogSink {
        public:
            LogClient();

            // severity
            //  0: INFO
            //  1: WARNING
            //  2: ERROR
            //  3: FATAL
            virtual void send(int severity, const std::string &message) = 0;

            // This method implements the output of the logger. It internally calls the above output() method.
            void send(google::LogSeverity severity, const char *full_filename,
                      const char *base_filename, int line,
                      const struct ::tm *tm_time,
                      const char *message, size_t message_len) override;

        };

    };

}

#endif // EASY3D_UTIL_LOGGING_H
