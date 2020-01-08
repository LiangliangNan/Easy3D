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


#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>


namespace easy3d {

    namespace logging {

        void initialize(const char* argv0, int threshold)
        {
            // Initialize Google's logging library.
            std::string full_path(argv0);
#ifdef __APPLE__
            std::string::size_type pos = full_path.find(".app");
            if (pos != std::string::npos)
                full_path = full_path.substr(0, pos);
#endif

            full_path = file_system::dir_name(full_path) + "/logs/";
            if (!easy3d::file_system::is_directory(full_path))
                easy3d::file_system::create_directory(full_path);
            google::SetLogDestination(google::GLOG_INFO, full_path.c_str());

            std::string file_ext = file_system::stripped_name(argv0) + "-";
            google::SetLogFilenameExtension(file_ext.c_str());

            FLAGS_stderrthreshold = threshold;
            FLAGS_colorlogtostderr = true;
            google::InitGoogleLogging(argv0);

            LOG(INFO) << "Current working directory: " << file_system::current_working_directory();
        }
    }

}

