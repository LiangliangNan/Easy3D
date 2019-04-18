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


#ifndef EASY3D_OPENGL_ERROR_H
#define EASY3D_OPENGL_ERROR_H

#include <string>


namespace easy3d {


#define easy3d_gl_error {\
    opengl::check_gl_error(__FILE__, __FUNCTION__, __LINE__) ;\
}

#define easy3d_frame_buffer_error {\
    opengl::check_frame_buffer_error(__FILE__, __FUNCTION__, __LINE__) ;\
}


#ifndef NDEBUG
#define easy3d_debug_gl_error				easy3d_gl_error
#define easy3d_debug_frame_buffer_error		easy3d_frame_buffer_error
#else
#define easy3d_debug_gl_error
#define easy3d_debug_frame_buffer_error
#endif


    namespace opengl {

        // Prints the last GL error to the Logger.
        bool check_gl_error(const std::string& file, int line);
        bool check_gl_error(const std::string& file, const std::string& function, int line);
        bool check_frame_buffer_error(const std::string& file, int line);
        bool check_frame_buffer_error(const std::string& file, const std::string& function, int line);

        // Check if there was an error. The error message will be stored in "log" if provided.
        bool gl_error(std::string& log);
        bool frame_buffer_error(std::string& log);

        void setup_gl_debug_callback();

    }

}


#endif  // EASY3D_OPENGL_ERROR_H


