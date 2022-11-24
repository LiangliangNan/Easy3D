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

#ifndef EASY3D_RENDERER_OPENGL_UTIL_H
#define EASY3D_RENDERER_OPENGL_UTIL_H

#include <string>
#include <iostream>
#include <unordered_map>


namespace easy3d {

    /**
     * \brief Utilities for OpenGL initialization and states query.
     * \class OpenglUtil easy3d/renderer/opengl_util.h
     */
    class OpenglUtil {
    public:
        static bool init();

        static bool is_initialized();

        // -------------- general information --------------

        /// returns either GL_CONTEXT_CORE_PROFILE_BIT or GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
        static int gl_profile();

        static bool is_supported(const std::string &name);  // e.g., "GL_VERSION_3_2", "GL_ARB_vertex_array_object"
        static bool has_extension(const std::string &name); // e.g., "GL_EXT_framebuffer_object"

        static std::string glew_version();

        static std::string gl_vendor();

        static std::string gl_renderer();

        static std::string gl_version();

        static std::string gl_extensions();

        static std::string glsl_version();

        static int gl_major_version();

        static int gl_minor_version();

        static float glew_version_number();

        static float gl_version_number();

        static float glsl_version_number();

        static int samples();

        /// Query the OpenGL viewport. The parameters are the same as in glViewport(x, y, width, height).
        /// x, y: the lower left corner of the viewport rectangle, in pixels.
        /// width, height: the width and height of the viewport.
        static void viewport(int &x, int &y, int &width, int &height);

        // ----------------- GPU memory  -------------------

        /// in MB.
        /// returns 0 if the query fails.
        /// NOTE: (1) OpenGL >= 2.0 is required.
        ///		  (2) currently only NVidia GPUs are supported
        static int total_gpu_memory();

        static int available_gpu_memory();

        // ----------------- print information  -------------------

        /// sets the output stream for the messages.
        /// if null, LOG(INFO) is the default output stream
        static void set_output(std::ostream *out);

        // --------------- buffer information ----------------

        /// display current bound buffer info
        static void get_current_buffer_info();

        /// display the buffer information
        static void get_buffer_info(unsigned int target, int bufferName);

        // --------------- GLSL information (GLSL version 4.2 is supported )----------------

        /// display VAO information, including its attributes
        static void get_vao_info(unsigned int buffer);

        /// display detailed info for a program
        static void get_program_info(unsigned int program);

        /// display detailed info for attributes in a program
        static void get_attributes_info(unsigned int program);

        /// display info for all active uniforms in a program
        static void get_uniforms_info(unsigned int program);

        /// display a uniform's value(s)
        static void get_uniform_info(unsigned int program, const std::string &uniName);

        /// display the values for a uniform in a named block
        static void
        get_uniform_in_block_info(unsigned int program, const std::string &blockName, const std::string &uniName);

    private:
        /// printf() style + plus a newline at the end
        static void _add_message(std::string format, ...);

        static bool _init();

        static int _get_rows(unsigned int type);

        static int _get_columns(unsigned int type);

        static void _display_uniformf(float *f, int rows, int columns);

        static void _display_uniformi(int *f, int rows, int columns);

        static void _display_uniformui(unsigned int *f, int rows, int columns);

        static void _display_uniformd(double *f, int rows, int columns);

        static int _get_uniform_byte_size(int size, int uniType, int arrayStride, int matStride);

        enum Types {
            DONT_KNOW, INT, UNSIGNED_INT, FLOAT, DOUBLE
        };

        static Types _get_type(unsigned int type);

    private:
        static bool _glew_initialized;

        static bool _spInit;

        static std::ostream *_output_stream;

        static std::unordered_map<int, std::string> spBufferAccess;
        static std::unordered_map<int, std::string> spBufferUsage;
        static std::unordered_map<int, std::string> spBufferBinding;
        static std::unordered_map<int, int> spBufferBound;
        static std::unordered_map<int, int> spBoundBuffer;
        static std::unordered_map<int, std::string> spDataF;
        static std::unordered_map<int, std::string> spGLSLType;
        static std::unordered_map<int, int> spGLSLTypeSize;
        static std::unordered_map<int, std::string> spShaderType;
        static std::unordered_map<int, std::string> spTransFeedBufferMode;
        static std::unordered_map<int, std::string> spGLSLPrimitives;
        static std::unordered_map<int, std::string> spTessGenSpacing;
        static std::unordered_map<int, std::string> spVertexOrder;
    };


}

#endif  // EASY3D_RENDERER_OPENGL_UTIL_H


