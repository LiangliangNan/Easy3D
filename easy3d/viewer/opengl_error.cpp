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

#include <easy3d/viewer/opengl_error.h>

#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/opengl_info.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace opengl {

        // Convert an OpenGL error code into a descriptive string.
        inline const char* gl_error_string(GLenum code) {
            switch (code)
            {
            case GL_NO_ERROR:
                return "No error";
            case GL_INVALID_ENUM:
                return "Invalid enum (An unacceptable value is specified for an enumerated argument)";
            case GL_INVALID_VALUE:
                return "Invalid value (A numeric argument is out of range)";
            case GL_INVALID_OPERATION:
                return "Invalid operation (The specified operation is not allowed in the current state)";
            case GL_OUT_OF_MEMORY:
                return "Out of memory (There is not enough memory left to execute the command)";
    #ifdef GL_STACK_OVERFLOW
            case GL_STACK_OVERFLOW:
                return "Stack overflow (An attempt has been made to perform an operation that would cause an internal stack to overflow)";
    #endif
    #ifdef GL_STACK_UNDERFLOW
            case GL_STACK_UNDERFLOW:
                return "Stack underflow (An attempt has been made to perform an operation that would cause an internal stack to underflow)";
    #endif
            case GL_TABLE_TOO_LARGE:
                return "Table too large";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "Invalid framebuffer operation (The framebuffer object is not complete)";
            }
            return "Unknown error";
        }


        bool check_gl_error(const std::string& file, const std::string& function, int line) {
            GLenum error_code = glGetError();
            if (error_code != GL_NO_ERROR) {
                const std::string& msg = gl_error_string(error_code);
                LOG(ERROR) << "GL error: \n"
                           << "\tfile: " << file_system::simple_name(file) << std::endl
                           << "\tline: " << line << std::endl
                           << "\tfunction: " << function << std::endl
                           << "\tinfo: " << msg << std::endl;
                return false;
            }
            else
                return true;
        }

        bool gl_error(std::string& log) {
            GLenum error_code = glGetError();
            if (error_code != GL_NO_ERROR) {
                const char* str = gl_error_string(error_code);
                if (str)
                    log = std::string(str);
                return false;
            }
            else
                return true;
        }


        bool check_frame_buffer_error(const std::string& file, const std::string& function, int line) {
            std::string msg;
            if (!frame_buffer_error(msg)) {
				LOG(ERROR) << "GL framebuffer error: \n"
						   << "\tfile: " << file_system::simple_name(file) << std::endl
						   << "\tline: " << line << std::endl
						   << "\tfunction: " << function << std::endl
						   << "\tinfo: " << msg << std::endl;
                return false;
            }
            else
                return true;
        }


        bool frame_buffer_error(std::string& log)
        {
            GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            switch (Status)
            {
            case GL_FRAMEBUFFER_COMPLETE:
                return true;
            case GL_FRAMEBUFFER_UNDEFINED:
                log = "GL_FRAMEBUFFER_UNDEFINED";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                log = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                log = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                log = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                log = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                log = "GL_FRAMEBUFFER_UNSUPPORTED";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                log = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                log = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
                break;
            default:
                log = "unknown error";
            }

            return false;
        }


        //------------------------------------------------------------------------------
        // OpenGL debug callback
        //------------------------------------------------------------------------------

        static void GLAPIENTRY opengl_debug_callback(GLenum source,
          GLenum type,
          GLuint id,
          GLenum severity,
          GLsizei length,
          const GLchar* message,
          const GLvoid* userParam)
        {
            (void)id; (void)length; (void)userParam;

            GLenum filter = GL_DEBUG_SEVERITY_MEDIUM;
            GLenum severitycmp = severity;
            // minor fixup for filtering so notification becomes lowest priority
            if (GL_DEBUG_SEVERITY_NOTIFICATION == filter) {
                filter = GL_DEBUG_SEVERITY_LOW + 1;
            }
            if (GL_DEBUG_SEVERITY_NOTIFICATION == severitycmp) {
                severitycmp = GL_DEBUG_SEVERITY_LOW + 1;
            }

            if (!filter || severitycmp <= filter)
            {

                //static std::map<GLuint, bool> ignoreMap;
                //if(ignoreMap[id] == true)
                //    return;
                std::string strSource = "0";
                std::string strType = strSource;
                switch (source)
                {
                case GL_DEBUG_SOURCE_API:
                    strSource = "API";
                    break;
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    strSource = "WINDOWS";
                    break;
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    strSource = "SHADER COMP";
                    break;
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    strSource = "3RD PARTY";
                    break;
                case GL_DEBUG_SOURCE_APPLICATION:
                    strSource = "APP";
                    break;
                case GL_DEBUG_SOURCE_OTHER:
                    strSource = "OTHER";
                    break;
                }
                switch (type)
                {
                case GL_DEBUG_TYPE_ERROR:
                    strType = "ERROR";
                    break;
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    strType = "Deprecated";
                    break;
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    strType = "Undefined";
                    break;
                case GL_DEBUG_TYPE_PORTABILITY:
                    strType = "Portability";
                    break;
                case GL_DEBUG_TYPE_PERFORMANCE:
                    strType = "Performance";
                    break;
                case GL_DEBUG_TYPE_OTHER:
                    strType = "Other";
                    break;
                }
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH:
                    printf("debug : High - %s - %s : %s\n", strSource.c_str(), strType.c_str(), message);
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    printf("debug : Medium - %s - %s : %s\n", strSource.c_str(), strType.c_str(), message);
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    printf("debug : Low - %s - %s : %s\n", strSource.c_str(), strType.c_str(), message);
                    break;
                default:
                    printf("debug : comment - %s - %s : %s\n", strSource.c_str(), strType.c_str(), message);
                    break;
                }
            }
        }


        void setup_gl_debug_callback() {
            // glDebugMessageCallback requires OpenGL >= 4.3
            if (glDebugMessageCallback) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
                glDebugMessageCallback(opengl_debug_callback, nullptr);  // no user data
            }
        }

    }

}
