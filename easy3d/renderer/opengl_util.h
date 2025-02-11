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
        /**
         * \brief Initialize OpenGL.
         * \return \c true if initialization is successful, \c false otherwise.
         */
        static bool init();
        /**
         * \brief Check if OpenGL is initialized.
         * \return \c true if OpenGL is initialized, \c false otherwise.
         */
        static bool is_initialized();

        // -------------- general information --------------

        /**
         * \brief Get the OpenGL profile.
         * \return Either GL_CONTEXT_CORE_PROFILE_BIT or GL_CONTEXT_COMPATIBILITY_PROFILE_BIT.
         */
        static int gl_profile();

        /**
         * \brief Check if a specific OpenGL feature is supported.
         * \param name The name of the feature, e.g., "GL\_VERSION\_3\_2", "GL\_ARB\_vertex\_array\_object".
         * \return \c true if the feature is supported, \c false otherwise.
         */
        static bool is_supported(const std::string &name);  // e.g., "GL_VERSION_3_2", "GL_ARB_vertex_array_object"
        /**
         * \brief Check if a specific OpenGL extension is available.
         * \param name The name of the extension, e.g., "GL\_EXT\_framebuffer\_object".
         * \return \c true if the extension is available, \c false otherwise.
         */
        static bool has_extension(const std::string &name); // e.g., "GL_EXT_framebuffer_object"
        /**
         * \brief Get the GLEW version.
         * \return The GLEW version as a string.
         */
        static std::string glew_version();
        /**
         * \brief Get the OpenGL vendor.
         * \return The OpenGL vendor as a string.
         */
        static std::string gl_vendor();
        /**
         * \brief Get the OpenGL renderer.
         * \return The OpenGL renderer as a string.
         */
        static std::string gl_renderer();
        /**
         * \brief Get the OpenGL version.
         * \return The OpenGL version as a string.
         */
        static std::string gl_version();
        /**
         * \brief Get the OpenGL extensions.
         * \return The OpenGL extensions as a string.
         */
        static std::string gl_extensions();
        /**
         * \brief Get the GLSL version.
         * \return The GLSL version as a string.
         */
        static std::string glsl_version();
        /**
         * \brief Get the major version of OpenGL.
         * \return The major version of OpenGL.
         */
        static int gl_major_version();
        /**
         * \brief Get the minor version of OpenGL.
         * \return The minor version of OpenGL.
         */
        static int gl_minor_version();
        /**
         * \brief Get the GLEW version number.
         * \return The GLEW version number.
         */
        static float glew_version_number();
        /**
         * \brief Get the OpenGL version number.
         * \return The OpenGL version number.
         */
        static float gl_version_number();
        /**
         * \brief Get the GLSL version number.
         * \return The GLSL version number.
         */
        static float glsl_version_number();
        /**
         * \brief Get the number of samples.
         * \return The number of samples.
         */
        static int samples();

        /**
         * \brief Query the OpenGL viewport. The parameters are the same as in glViewport(x, y, width, height)
         * \param x The lower left corner x-coordinate of the viewport rectangle, in pixels.
         * \param y The lower left corner y-coordinate of the viewport rectangle, in pixels.
         * \param width The width of the viewport.
         * \param height The height of the viewport.
         */
        static void viewport(int &x, int &y, int &width, int &height);

        // ----------------- GPU memory  -------------------

        /**
         * \brief Get the total GPU memory.
         * \return The total GPU memory in MB. Returns 0 if the query fails.
         * \note OpenGL >= 2.0 is required. Currently, only NVidia GPUs are supported.
         */
        static int total_gpu_memory();
        /**
         * \brief Get the available GPU memory.
         * \return The available GPU memory in MB.
         */
        static int available_gpu_memory();

        // ----------------- print information  -------------------

        /**
         * \brief Set the output stream for the messages.
         * \param out The output stream. If null, LOG(INFO) is the default output stream.
         */
        static void set_output(std::ostream *out);

        // --------------- buffer information ----------------

        /**
         * \brief Get current bound buffer info.
         */
        static void get_current_buffer_info();

        /**
         * \brief Get the buffer information.
         * \param target The target buffer.
         * \param bufferName The name of the buffer.
         */
        static void get_buffer_info(unsigned int target, int bufferName);

        // --------------- GLSL information (GLSL version 4.2 is supported )----------------

        /**
         * \brief Get VAO information, including its attributes.
         * \param buffer The VAO buffer.
         */
        static void get_vao_info(unsigned int buffer);

        /**
         * \brief Get detailed info for a program.
         * \param program The program ID.
         */
        static void get_program_info(unsigned int program);

        /**
         * \brief Get detailed info for attributes in a program.
         * \param program The program ID.
         */
        static void get_attributes_info(unsigned int program);

        /**
         * \brief Get info for all active uniforms in a program.
         * \param program The program ID.
         */
        static void get_uniforms_info(unsigned int program);

        /**
         * \brief Get a uniform's value(s).
         * \param program The program ID.
         * \param uniName The name of the uniform.
         */
        static void get_uniform_info(unsigned int program, const std::string &uniName);

        /**
         * \brief Get the values for a uniform in a named block.
         * \param program The program ID.
         * \param blockName The name of the block.
         * \param uniName The name of the uniform.
         */
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


