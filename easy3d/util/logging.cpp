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

#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>

#include <3rd_party/glog/src/utilities.h>

#include <iostream>


namespace easy3d
{

	namespace logging
	{

		void initialize(bool severity_dependent, const std::string& log_dir, int stderr_threshold)
		{
			// Ensures the library will not be initialized more than once.
			if (google::glog_internal_namespace_::IsGoogleLoggingInitialized())
				return;

			// use "static" to make sure the string still exist outside this function (glog is multi threaded).
            static std::string app_path = file_system::executable();

			std::string log_path = log_dir;
			if (log_path.empty())
			{
				log_path = app_path;
#ifdef __APPLE__
				// macOS may put the executable file in an application bundle, e.g., "PolyFit.app/Contents/MacOS/PolyFit"
				std::string::size_type pos = log_path.find(".app");
				if (pos != std::string::npos)
					log_path = log_path.substr(0, pos);
#endif
				log_path = file_system::parent_directory(log_path) + "/logs";
			}
            else    // always append a separator in case the user forgot it
                log_path = log_dir + file_system::native_path_separator();

			if (!file_system::is_directory(log_path))
				file_system::create_directory(log_path);
			LOG_IF(ERROR, !file_system::is_directory(log_path)) << "could not create log directory: " << log_path;

			if (severity_dependent) // create a log file for each severity level
				FLAGS_log_dir = log_path;
			else {
			    const std::string& dest = log_path + "/" + file_system::base_name(app_path) + "-";
                google::SetLogDestination(google::GLOG_INFO, dest.c_str());
			}

			FLAGS_stderrthreshold = stderr_threshold;
			FLAGS_colorlogtostderr = true;
			google::InitGoogleLogging(app_path.c_str());

//            auto failure_dump = [](const char* data, int size) -> void {
//                LOG(ERROR) << std::string(data, size)
//                << "\n-----------------------------------------\n"
//                   "Oh, Easy3D crashed. Please contact me (liangliang.nan@gmail.com) for more information.";
//            };
//            google::InstallFailureWriter(failure_dump);

			DLOG(INFO) << "logger initialized";
			DLOG(INFO) << "executable path: " << file_system::executable_directory();
			DLOG(INFO) << "current working dir: " << file_system::current_working_directory();
		}



        LogClient::LogClient() {
		    google::AddLogSink(this);
		}


        void LogClient::send(google::LogSeverity severity, const char *full_filename,
                  const char *base_filename, int line,
                  const struct ::tm *tm_time,
                  const char *message, size_t message_len)
        {
		    if (message && message_len > 0)
                send(severity, std::string(message, message + message_len));
		}
	}

}

