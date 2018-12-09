/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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


#ifndef _OPENGL_GL_ERROR_H_
#define _OPENGL_GL_ERROR_H_

#include <string>


namespace easy3d {


#define mpl_gl_error {\
	check_gl_error(__FILE__, __LINE__) ;\
}

#define mpl_frame_buffer_error {\
	check_frame_buffer_error(__FILE__, __LINE__) ;\
}


#ifndef NDEBUG
#define mpl_debug_gl_error				mpl_gl_error
#define mpl_debug_frame_buffer_error	mpl_frame_buffer_error
#else
#define mpl_debug_gl_error
#define mpl_debug_frame_buffer_error
#endif


	// Prints the last GL error to the Logger.
	bool check_gl_error(const std::string& file, int line);
	bool check_frame_buffer_error(const std::string& file, int line);

	// Check if there was an error. The error message will be stored in "log" if provided.
	bool gl_error(std::string& log);
	bool frame_buffer_error(std::string& log);

}


#endif


