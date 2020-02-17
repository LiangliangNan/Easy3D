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


		// I thought it might be possible to avoid calling the initialize() at the begining of
		// the main function to initialize glob. But it seems unused static variables will not
		// be initialized (depends on compilers). I am not sure!!!
//        static class init_easy3d_logging
//        {
//        public:
//            init_easy3d_logging() {
//                // Ensures the library will be initialized only once.
//                if (!google::glog_internal_namespace_::IsGoogleLoggingInitialized())
//                    initialize(file_system::executable().c_str(), google::GLOG_INFO);
//            }
//            ~init_easy3d_logging() {
//            }
//        } init;



		void initialize(const char* argv0)
		{
			// Ensures the library will not be initialized more than once.
			if (google::glog_internal_namespace_::IsGoogleLoggingInitialized())
				return;

			std::string log_path = argv0;
#ifdef __APPLE__
			// macOS may put the executable file in an application bundle, e.g., "PolyFit.app/Contents/MacOS/PolyFit"
			std::string::size_type pos = log_path.find(".app");
			if (pos != std::string::npos)
				log_path = log_path.substr(0, pos);
#endif
			log_path = file_system::parent_directory(log_path) + "/logs/";

			if (!file_system::is_directory(log_path))
				file_system::create_directory(log_path);
			LOG_IF(ERROR, !file_system::is_directory(log_path)) << "could not create log directory: " << log_path;

			if (1) {
				google::SetLogDestination(google::INFO, log_path.c_str());
				std::string extension = file_system::base_name(argv0) + "-";
				google::SetLogFilenameExtension(extension.c_str());
			}
			else // create a log file for each severity level
				FLAGS_log_dir = log_path;

			FLAGS_stderrthreshold = google::INFO;
			FLAGS_colorlogtostderr = true;
			google::InitGoogleLogging(argv0);

			DLOG(INFO) << "logger initialized";
			DLOG(INFO) << "executable path: " << file_system::executable_directory();
			DLOG(INFO) << "current working dir: " << file_system::current_working_directory();
		}
	}

}

