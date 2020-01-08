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


#ifndef EASY3D_LOGGING_H
#define EASY3D_LOGGING_H


/* Make it easier for including the Google glog header file.
 *
 * Always include this file, i.e.,
 *      #include <easy3d/util/logging.h>
 * Instead of:
 *      #include <3rd_party/glog/glog/logging.h>
 * or any other header file, because the third party library may change.
 */

#include <3rd_party/glog/glog/logging.h>


namespace easy3d {

    namespace logging {

    /**
         * @brief initialize Google's logging library.
         * @param argv0     The argv[0] of your main function, which is basically the
         *                  full path of the executable file.
         * @param threshold Log messages at a level >= this flag are automatically sent
         *                  to stderr in addition to log files.
         */
        void initialize(const char* argv0, int threshold = google::GLOG_INFO);

    }

}


#endif // EASY3D_LOGGING_H
