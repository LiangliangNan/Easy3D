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

#ifndef EASY3D_RENDERER_OPENGL_ERROR_H
#define EASY3D_RENDERER_OPENGL_ERROR_H

#include <string>
#include <iostream>
#include <easy3d/util/logging.h>


namespace easy3d {

/// Checks if OpenGL error occurred. If so, the error message will be written to the std::cerr.
#define easy3d_gl_error {                                                   \
    opengl::check_gl_error(__FILE__, __FUNCTION__, __LINE__) ;              \
}

/// Checks if framebuffer error occurred. If so, the error message will be written to the std::cerr.
#define easy3d_frame_buffer_error {                                         \
    opengl::check_frame_buffer_error(__FILE__, __FUNCTION__, __LINE__) ;    \
}

/// Checks if OpenGL error occurred. If so, the error will be logged (by the default logger).
#define easy3d_log_gl_error {                                               \
    std::string msg;                                                        \
    LOG_IF(!opengl::gl_error(msg), ERROR) << "GL error: \n"                 \
    << "\tfile: " << __FILE__ << "\n"                                       \
    << "\tline: " << __LINE__ << "\n"                                       \
    << "\tfunction: " << __FUNCTION__ << "\n"                               \
    << "\tinfo: " << msg;                                                   \
}

/// Checks if framebuffer error occurred. If so, the error will be logged (by the default logger).
#define easy3d_log_frame_buffer_error {                                     \
    std::string msg;                                                        \
    LOG_IF(!opengl::frame_buffer_error(msg), ERROR) << "GL error: \n"       \
    << "\tfile: " << __FILE__ << "\n"                                       \
    << "\tline: " << __LINE__ << "\n"                                       \
    << "\tfunction: " << __FUNCTION__ << "\n"                               \
    << "\tinfo: " << msg;                                                   \
}


#ifndef NDEBUG
#define easy3d_debug_gl_error				easy3d_gl_error
#define easy3d_debug_frame_buffer_error		easy3d_frame_buffer_error
#define easy3d_debug_log_gl_error			easy3d_log_gl_error
#define easy3d_debug_log_frame_buffer_error	easy3d_log_frame_buffer_error
#else
#define easy3d_debug_gl_error
#define easy3d_debug_frame_buffer_error
#define easy3d_debug_log_gl_error
#define easy3d_debug_log_frame_buffer_error
#endif


    namespace opengl {

        /**
         * @brief Checks the last OpenGL error.
         * @param file The file name where the error occurred.
         * @param function The function name where the error occurred.
         * @param line The line number where the error occurred.
         * @param os The output stream to write the error message. Default is std::cerr.
         * @return false if an error indeed occurred.
         * @see check_frame_buffer_error.
         */
        bool check_gl_error(const std::string& file, const std::string& function, int line, std::ostream& os = std::cerr);

        /**
         * @brief Checks the last framebuffer error.
         * @param file The file name where the error occurred.
         * @param function The function name where the error occurred.
         * @param line The line number where the error occurred.
         * @param os The output stream to write the error message. Default is std::cerr.
         * @return false if an error indeed occurred.
         * @see check_gl_error.
         */
        bool check_frame_buffer_error(const std::string& file, const std::string& function, int line, std::ostream& os = std::cerr);

        /**
         * @brief Checks if there was an OpenGL error. If there was an error, the error message will be stored in "msg".
         * @param msg The error message.
         * @return false if an error occurred.
         * @see frame_buffer_error.
         */
        bool gl_error(std::string& msg);
        /**
         * @brief Checks if there was a framebuffer error. If there was an error, the error message will be stored in "msg".
         * @param msg The error message.
         * @return false if an error occurred.
         * @see frame_buffer_error.
         */
        bool frame_buffer_error(std::string& msg);

        /**
         * @brief Set up a debug callback for OpenGL.
         * @note This has effect only if the working OpenGL version is >= 4.3.
         */
        void setup_gl_debug_callback();

    }

}


#endif  // EASY3D_RENDERER_OPENGL_ERROR_H


