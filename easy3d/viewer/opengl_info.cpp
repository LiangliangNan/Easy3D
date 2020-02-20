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

#include <easy3d/viewer/opengl_info.h>

#include <iostream>
#include <cstdio>	// for vsprintf()
#include <cstdarg>	// for va_start() and va_end()
#include <string>	// for to_string()

#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    static const std::string err_msg = "error (null_string)";


    bool			OpenglInfo::_glew_initialized = false;
    std::ostream*	OpenglInfo::_output_stream = (std::iostream*)&(std::cout);


    bool OpenglInfo::init() {
        if (_glew_initialized)
            return true;

        GLenum status = glewInit();
        _glew_initialized = true;

        if (GLEW_OK != status) {
            // Problem: glewInit failed, something is seriously wrong.
            LOG(ERROR) << glewGetErrorString(status);
            return false;
        }

        return true;
    }


    bool OpenglInfo::is_initialized() {
        return _glew_initialized;
    }


    bool OpenglInfo::is_supported(const std::string& name) {
        if (!_glew_initialized)
            init();
        return (glewIsSupported(name.c_str()) != 0);
    }


    bool OpenglInfo::has_entension(const std::string &ext) {
        if (!_glew_initialized)
            init();
        int num = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num);
        for(int i = 0; i < num; ++i) {
            const unsigned char* name = glGetStringi(GL_EXTENSIONS, static_cast<unsigned int>(i));
            if (std::string(reinterpret_cast<const char*>(name)) == ext)
                return true;
        }
        return false;
    }

    int OpenglInfo::gl_profile() {
        if (!_glew_initialized)
            init();

        // When MacOSX 10.7 introduced support for OpenGL beyond 2.1, they also introduced the core / compatibility dichotomy.
        // However, they did not introduce support for the compatibility profile itself. Instead, MacOSX gives you a choice:
        // core profile for versions 3.2 or higher, or just version 2.1. There is no way to get access to features after 2.1
        // and still access the Fixed Function Pipeline.
        //
        //Detect which profile the context supports:
        int profile = -1;
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile); // GL_CONTEXT_CORE_PROFILE_BIT  or GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
        easy3d_debug_log_gl_error;
        return profile;
    }


    std::string OpenglInfo::gl_vendor() {
        const GLubyte* str = glGetString(GL_VENDOR);
        if (str)
            return std::string(reinterpret_cast<const char*>(str));
        else
            return err_msg;
    }


    std::string OpenglInfo::gl_renderer() {
        const GLubyte* str = glGetString(GL_RENDERER);
        if (str)
            return std::string(reinterpret_cast<const char*>(str));
        else
            return err_msg;
    }


    std::string OpenglInfo::gl_version() {
        const GLubyte* str = glGetString(GL_VERSION);
        if (str)
            return std::string(reinterpret_cast<const char*>(str));
        else
            return err_msg;
    }

    std::string OpenglInfo::gl_extensions() {
        const GLubyte* str = glGetString(GL_EXTENSIONS);
        if (str)
            return std::string(reinterpret_cast<const char*>(str));
        else
            return err_msg;
    }

    std::string OpenglInfo::glew_version() {
        if (!_glew_initialized)
            init();

        const GLubyte* str = glewGetString(GLEW_VERSION);
        if (str)
            return std::string(reinterpret_cast<const char*>(str));
        else
            return err_msg;
    }

    std::string OpenglInfo::glsl_version() {
        const GLubyte* str = glGetString(GL_SHADING_LANGUAGE_VERSION);
        if (str)
            return std::string(reinterpret_cast<const char*>(str));
        else
            return "not supported";
    }


    int OpenglInfo::gl_major_version() {
        int v;
        glGetIntegerv(GL_MAJOR_VERSION, &v);
        return v;
    }


    int OpenglInfo::gl_minor_version() {
        int v;
        glGetIntegerv(GL_MINOR_VERSION, &v);
        return v;
    }


    float OpenglInfo::glew_version_number() {
        if (!_glew_initialized)
            init();

        const GLubyte* str = glewGetString(GLEW_VERSION);
        if (str) {
            float version = std::stof(reinterpret_cast<const char*>(str), 0);
            return version;
        }
        else
            return 0.0f;
    }


    float OpenglInfo::gl_version_number() {
        const GLubyte* str = glGetString(GL_VERSION);
        if (str) {
            float version = std::stof(reinterpret_cast<const char*>(str), 0);
            return version;
        }
        else
            return 0.0f;

        //GLint major, minor;
        //glGetIntegerv(GL_MAJOR_VERSION, &major);
        //glGetIntegerv(GL_MINOR_VERSION, &minor);
        //const std::string str = std::to_string(major) + "." + std::to_string(minor);
        //float version = std::stof(str);
        //return version;
    }


    float OpenglInfo::glsl_version_number() {
        const GLubyte* str = glGetString(GL_SHADING_LANGUAGE_VERSION);
        float version = std::stof(reinterpret_cast<const char*>(str), 0);
        return version;
    }


    // sets the output stream for the messages.
    // if null, cout is used
    void OpenglInfo::set_output(std::ostream *out) {
        if (!out)
            // if null use cout
            _output_stream = (std::iostream*)&std::cout;
        else
            _output_stream = out;
    }


    // ---------------------------------------------------------------------------------

    std::unordered_map<int, std::string>	OpenglInfo::spBufferAccess;
    std::unordered_map<int, std::string>	OpenglInfo::spBufferUsage;
    std::unordered_map<int, std::string>	OpenglInfo::spBufferBinding;
    std::unordered_map<int, int>			OpenglInfo::spBoundBuffer;
    std::unordered_map<int, int>			OpenglInfo::spBufferBound;
    std::unordered_map<int, std::string>	OpenglInfo::spDataF;
    std::unordered_map<int, std::string>	OpenglInfo::spGLSLType;
    std::unordered_map<int, int> 			OpenglInfo::spGLSLTypeSize;
    std::unordered_map<int, std::string>	OpenglInfo::spShaderType;
    std::unordered_map<int, std::string>	OpenglInfo::spTransFeedBufferMode;
    std::unordered_map<int, std::string>	OpenglInfo::spGLSLPrimitives;
    std::unordered_map<int, std::string>	OpenglInfo::spTessGenSpacing;
    std::unordered_map<int, std::string>	OpenglInfo::spVertexOrder;

    bool OpenglInfo::_spInit = OpenglInfo::_init();

    /// display current binded buffer info
    void OpenglInfo::getCurrentBufferInfo() {

        int info;

        *_output_stream << std::endl;
        _add_message("Current Buffer Bindings");

        // iterate for all buffer types
        std::unordered_map<int, int>::iterator iter = spBufferBound.begin();
        for (; iter != spBufferBound.end(); ++iter) {
            // get current binding for a type of buffer
            glGetIntegerv(iter->first, &info);
            // if a buffer is bound get its info
            if (info) {

                _add_message("\tBuffer Info for name: %d", info);
                _add_message("\t\tBuffer Type: %s", spBufferBinding[iter->first].c_str());

                glGetBufferParameteriv(iter->second, GL_BUFFER_ACCESS, &info);
                _add_message("\t\tAccess: %s", spBufferAccess[info].c_str());

                glGetBufferParameteriv(iter->second, GL_BUFFER_MAPPED, &info);
                _add_message("\t\tMapped: %d", info);

                glGetBufferParameteriv(iter->second, GL_BUFFER_SIZE, &info);
                _add_message("\t\tSize: %d", info);

                glGetBufferParameteriv(iter->second, GL_BUFFER_USAGE, &info);
                _add_message("\t\tUsage: %s", spBufferUsage[info].c_str());
            }
        }
    }


    /// display the buffer information
    void OpenglInfo::getBufferInfo(GLenum target, int bufferName) {
        int info, prevBuffer;

        *_output_stream << std::endl;
        _add_message("Info for buffer name: %d target: %s", bufferName, spBufferBinding[spBoundBuffer[target]].c_str());

        // get previously bound buffer
        glGetIntegerv(spBoundBuffer[target], &prevBuffer);
        // bind requested buffer to get info
        glBindBuffer(target, bufferName);

        glGetBufferParameteriv(target, GL_BUFFER_ACCESS, &info);
        _add_message("\tAccess: %s", spBufferAccess[info].c_str());

        glGetBufferParameteriv(target, GL_BUFFER_MAPPED, &info);
        _add_message("\tMapped: %d", info);

        glGetBufferParameteriv(target, GL_BUFFER_SIZE, &info);
        _add_message("\tSize: %d", info);

        glGetBufferParameteriv(target, GL_BUFFER_USAGE, &info);
        _add_message("\tUsage: %s", spBufferUsage[info].c_str());

        // re-bind previous buffer
        glBindBuffer(target, prevBuffer);
    }


    /// display detailed VAO info
    void OpenglInfo::getVAOInfo(unsigned int buffer) {
        int count, info, prevBuffer;

        *_output_stream << std::endl;

        // is it a VAO?
        if (!glIsVertexArray(buffer)) {
            _add_message("name: %d is not a VAO", buffer);
            return;
        }

        _add_message("VAO Info for name: %d", buffer);

        // bind requested VAO
        // should be able to get previously bounded VAO...
        glBindVertexArray(buffer);

        // get element array buffer name
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &info);
        if (info)
            _add_message("\tElement Array: %d", info);

        // get info for each attrib mapped buffer
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &count);
        for (int i = 0; i < count; ++i) {

            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &info);
            if (info) {
                _add_message("\tAttrib index: %d", i);

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &info);
                _add_message("\t\tBuffer bound: %d", info);

                glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, info);
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &info);
                glBindBuffer(GL_ARRAY_BUFFER, prevBuffer);
                _add_message("\t\tSize: %d", info);

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &info);
                _add_message("\t\tComponents: %d", info);

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &info);
                _add_message("\t\tData Type: %s", spDataF[info].c_str());

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &info);
                _add_message("\t\tStride: %d", info);

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &info);
                _add_message("\t\tNormalized: %d", info);

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &info);
                _add_message("\t\tDivisor: %d", info);

                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_INTEGER, &info);
                _add_message("\t\tInteger: %d", info);
            }
        }
        glBindVertexArray(0);
    }


    // display info for all active uniforms in a program
    void OpenglInfo::getUniformsInfo(unsigned int program) {
        *_output_stream << std::endl;

        // is it a program ?
        if (!glIsProgram(program)) {
            _add_message("name: %d is not a program", program);
            return;
        }

        int activeUnif, actualLen, index, uniType,
            uniSize, uniMatStride, uniArrayStride, uniOffset;
        char name[256];

        // Get uniforms info (not in named blocks)
        _add_message("Uniforms Info for program: %d", program);
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUnif);

        for (unsigned int i = 0; i < (unsigned int)activeUnif; ++i) {
            glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &index);
            if (index == -1) {
                glGetActiveUniformName(program, i, 256, &actualLen, name);
                glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_TYPE, &uniType);
                _add_message("\t%s", name);
                _add_message("\t\t%s", spGLSLType[uniType].c_str());
                _add_message("\t\tlocation: %d", i);

                glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_SIZE, &uniSize);
                glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);

                int auxSize;
                if (uniArrayStride > 0)
                    auxSize = uniArrayStride * uniSize;
                else
                    auxSize = spGLSLTypeSize[uniType];

                _add_message("\t\tsize: %d", auxSize);
                if (uniArrayStride > 0)
                    _add_message("\t\tstride: %d", uniArrayStride);
            }
        }
        // Get named blocks info
        int count, dataSize, info;
        glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &count);

        for (int i = 0; i < count; ++i) {
            // Get blocks name
            glGetActiveUniformBlockName(program, i, 256, nullptr, name);
            glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
            _add_message("%s\n\tSize %d", name, dataSize);

            glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_BINDING, &index);
            _add_message("\tBlock binding point: %d", index);
            glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, index, &info);
            _add_message("\tBuffer bound to binding point: %d {", info);


            glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUnif);

            unsigned int *indices;
            indices = (unsigned int *)malloc(sizeof(unsigned int) * activeUnif);
            glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (int *)indices);

            for (int k = 0; k < activeUnif; ++k) {

                glGetActiveUniformName(program, indices[k], 256, &actualLen, name);
                glGetActiveUniformsiv(program, 1, &indices[k], GL_UNIFORM_TYPE, &uniType);
                _add_message("\t\t%s\n\t\t\t%s", name, spGLSLType[uniType].c_str());

                glGetActiveUniformsiv(program, 1, &indices[k], GL_UNIFORM_OFFSET, &uniOffset);
                _add_message("\t\t\toffset: %d", uniOffset);

                glGetActiveUniformsiv(program, 1, &indices[k], GL_UNIFORM_SIZE, &uniSize);

                glGetActiveUniformsiv(program, 1, &indices[k], GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);

                glGetActiveUniformsiv(program, 1, &indices[k], GL_UNIFORM_MATRIX_STRIDE, &uniMatStride);

                int auxSize;
                if (uniArrayStride > 0)
                    auxSize = uniArrayStride * uniSize;

                else if (uniMatStride > 0) {

                    switch (uniType) {
                    case GL_FLOAT_MAT2:
                    case GL_FLOAT_MAT2x3:
                    case GL_FLOAT_MAT2x4:
                    case GL_DOUBLE_MAT2:
                    case GL_DOUBLE_MAT2x3:
                    case GL_DOUBLE_MAT2x4:
                        auxSize = 2 * uniMatStride;
                        break;
                    case GL_FLOAT_MAT3:
                    case GL_FLOAT_MAT3x2:
                    case GL_FLOAT_MAT3x4:
                    case GL_DOUBLE_MAT3:
                    case GL_DOUBLE_MAT3x2:
                    case GL_DOUBLE_MAT3x4:
                        auxSize = 3 * uniMatStride;
                        break;
                    case GL_FLOAT_MAT4:
                    case GL_FLOAT_MAT4x2:
                    case GL_FLOAT_MAT4x3:
                    case GL_DOUBLE_MAT4:
                    case GL_DOUBLE_MAT4x2:
                    case GL_DOUBLE_MAT4x3:
                        auxSize = 4 * uniMatStride;
                        break;
                    }
                }
                else
                    auxSize = spGLSLTypeSize[uniType];

                auxSize = _get_uniform_byte_size(uniSize, uniType, uniArrayStride, uniMatStride);
                _add_message("\t\t\tsize: %d", auxSize);
                if (uniArrayStride > 0)
                    _add_message("\t\t\tarray stride: %d", uniArrayStride);
                if (uniMatStride > 0)
                    _add_message("\t\t\tmat stride: %d", uniMatStride);
            }
            _add_message("\t}");
        }
    }


    /// display a uniform's value(s)
    void OpenglInfo::getUniformInfo(unsigned int program, const std::string& uniName) {

        *_output_stream << std::endl;

        // is it a program ?
        if (!glIsProgram(program)) {
            _add_message("name: %d is not a program", program);
            return;
        }

        GLenum type;
        GLsizei l;
        GLint s;
        char c[50];
        int loc = glGetUniformLocation((int)program, uniName.c_str());
        glGetActiveUniform(program, loc, 0, &l, &s, &type, c);

        if (loc != -1) {
            _add_message("Values for uniform %s in program %d", uniName.c_str(), program);
            int rows = _get_rows(type), columns = _get_columns(type);
            if (_get_type(type) == FLOAT) {
                float f[16];
                glGetUniformfv(program, loc, f);
                _display_uniformf(f, rows, columns);
            }
            else if (_get_type(type) == INT) {
                int f[16];
                glGetUniformiv(program, loc, f);
                _display_uniformi(f, rows, columns);
            }
            else if (_get_type(type) == UNSIGNED_INT) {
                unsigned int f[16];
                glGetUniformuiv(program, loc, f);
                _display_uniformui(f, rows, columns);
            }
            else if (_get_type(type) == DOUBLE) {
                double f[16];
                glGetUniformdv(program, loc, f);
                _display_uniformd(f, rows, columns);
            }
        }
        else
            _add_message("%s is not an active uniform in program %u", uniName.c_str(), program);
    }


    /// display the values for a uniform in a named block
    void OpenglInfo::getUniformInBlockInfo(unsigned int program, const std::string&blockName, const std::string& uniName) {
        *_output_stream << std::endl;

        // is it a program ?
        if (!glIsProgram(program)) {
            _add_message("name: %d is not a program", program);
            return;
        }

        unsigned int index = glGetUniformBlockIndex(program, blockName.c_str());
        if (index == GL_INVALID_INDEX) {
            _add_message("%s is not a valid uniform name in block %s", uniName.c_str(), blockName.c_str());
            return;
        }
        int bindIndex, bufferIndex;
        glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_BINDING, &bindIndex);
        _add_message("Block binding point: %d", bindIndex);
        glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, bindIndex, &bufferIndex);
        _add_message("Buffer bound to binding point: %d ", bufferIndex);
        unsigned int uniIndex;
        const char *c = uniName.c_str();
        glGetUniformIndices(program, 1, &c, &uniIndex);
        _add_message("Index of Uniform: %u", uniIndex);

        int uniType, uniOffset, uniSize, uniArrayStride, uniMatStride;
        glGetActiveUniformsiv(program, 1, &uniIndex, GL_UNIFORM_TYPE, &uniType);

        glGetActiveUniformsiv(program, 1, &uniIndex, GL_UNIFORM_OFFSET, &uniOffset);

        glGetActiveUniformsiv(program, 1, &uniIndex, GL_UNIFORM_SIZE, &uniSize);

        glGetActiveUniformsiv(program, 1, &uniIndex, GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);

        glGetActiveUniformsiv(program, 1, &uniIndex, GL_UNIFORM_MATRIX_STRIDE, &uniMatStride);

        int auxSize = _get_uniform_byte_size(uniSize, uniType, uniArrayStride, uniMatStride);
        // get previously bound buffer
        int prevBuffer;
        glGetIntegerv(spBoundBuffer[GL_UNIFORM_BUFFER], &prevBuffer);

        glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);

        int rows = _get_rows(uniType);
        int columns = auxSize / (rows * sizeof(float));

        if (_get_type(uniType) == FLOAT) {
            float f[16];
            glGetBufferSubData(GL_UNIFORM_BUFFER, uniOffset, auxSize, f);
            _display_uniformf(f, rows, columns);
        }
        else if (_get_type(uniType) == INT) {
            int f[16];
            glGetBufferSubData(GL_UNIFORM_BUFFER, uniOffset, auxSize, f);
            _display_uniformi(f, rows, columns);
        }
        else if (_get_type(uniType) == UNSIGNED_INT) {
            unsigned int f[16];
            glGetBufferSubData(GL_UNIFORM_BUFFER, uniOffset, auxSize, f);
            _display_uniformui(f, rows, columns);
        }
        else if (_get_type(uniType) == DOUBLE) {
            double f[16];
            glGetBufferSubData(GL_UNIFORM_BUFFER, uniOffset, auxSize, f);
            _display_uniformd(f, rows, columns);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, prevBuffer);
    }


    /// display detailed info for attributes in a program
    void OpenglInfo::getAttributesInfo(unsigned int program) {

        int activeAttr, size, loc;
        GLsizei length;
        GLenum type;
        char name[256];

        *_output_stream << std::endl;

        // check if it is a program
        if (!glIsProgram(program)) {
            _add_message("name: %d is not a program", program);
            return;
        }

        _add_message("Attribute Info for program %d", program);
        // how many attribs?
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttr);
        // get location and type for each attrib
        for (unsigned int i = 0; i < (unsigned int)activeAttr; ++i) {
            glGetActiveAttrib(program, i, 256, &length, &size, &type, name);
            loc = glGetAttribLocation(program, name);
            _add_message("\t%s\tloc: %d\t\ttype: %s", name, loc, spGLSLType[type].c_str());
        }

    }


    /// display detailed info for a program
    void OpenglInfo::getProgramInfo(unsigned int program) {
        *_output_stream << std::endl;

        // check if name is really a program
        if (!glIsProgram(program)) {
            _add_message("Name %u is not a program", program);
            return;
        }

        _add_message("Program Information for name %u", program);
        unsigned int shaders[5];
        int count, info, linked;
        bool geom = false, tess = false;

        // Get the shader's name
        _add_message("\tShaders {");
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &count);
        glGetAttachedShaders(program, count, nullptr, shaders);
        for (int i = 0; i < count; ++i) {

            glGetShaderiv(shaders[i], GL_SHADER_TYPE, &info);
            _add_message("\t\t%s: %d", spShaderType[info].c_str(), shaders[i]);
            if (info == GL_GEOMETRY_SHADER)
                geom = true;
            if (info == GL_TESS_EVALUATION_SHADER || info == GL_TESS_CONTROL_SHADER)
                tess = true;
        }
        _add_message("\t}");

        // Get program info
        glGetProgramiv(program, GL_PROGRAM_SEPARABLE, &info);
        _add_message("\tProgram Separable: %d", info);

        glGetProgramiv(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, &info);
        _add_message("\tProgram Binary Retrievable Hint: %d", info);

        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        _add_message("\tLink Status: %d", linked);

        glGetProgramiv(program, GL_VALIDATE_STATUS, &info);
        _add_message("\tValidate_Status: %d", info);

        glGetProgramiv(program, GL_DELETE_STATUS, &info);
        _add_message("\tDelete_Status: %d", info);

        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &info);
        _add_message("\tActive_Attributes: %d", info);

        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &info);
        _add_message("\tActive_Uniforms: %d", info);

        glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &info);
        _add_message("\tActive_Uniform_Blocks: %d", info);

        // GL_VERSION_4_2
        glGetProgramiv(program, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &info);
        _add_message("\tActive_Atomic Counters: %d", info);

        // check if trans feed is active
        glGetProgramiv(program, GL_TRANSFORM_FEEDBACK_BUFFER_MODE, &info);
        _add_message("\tTransform Feedback Buffer Mode: %s", spTransFeedBufferMode[info].c_str());

        glGetProgramiv(program, GL_TRANSFORM_FEEDBACK_VARYINGS, &info);
        _add_message("\tTransform Feedback Varyings: %d", info);

        // Geometry shader info, if present
        if (geom && linked) {
            glGetProgramiv(program, GL_GEOMETRY_VERTICES_OUT, &info);
            _add_message("\tGeometry Vertices Out: %d", info);

            glGetProgramiv(program, GL_GEOMETRY_INPUT_TYPE, &info);
            _add_message("\tGeometry Input Type: %s", spGLSLPrimitives[info].c_str());

            glGetProgramiv(program, GL_GEOMETRY_OUTPUT_TYPE, &info);
            _add_message("\tGeometry Output Type: %s", spGLSLPrimitives[info].c_str());

            glGetProgramiv(program, GL_GEOMETRY_SHADER_INVOCATIONS, &info);
            _add_message("\tGeometry Shader Invocations: %d", info);
        }
        // tessellation shaders info, if present
        if (tess && linked) {
            glGetProgramiv(program, GL_TESS_CONTROL_OUTPUT_VERTICES, &info);
            _add_message("\tTess Control Output Vertices: %d", info);

            glGetProgramiv(program, GL_TESS_GEN_MODE, &info);
            _add_message("\tTess Gen Mode: %s", spGLSLPrimitives[info].c_str());

            glGetProgramiv(program, GL_TESS_GEN_SPACING, &info);
            _add_message("\tTess Spacing: %s", spTessGenSpacing[info].c_str());

            glGetProgramiv(program, GL_TESS_GEN_VERTEX_ORDER, &info);
            _add_message("\tTess Vertex Order: %s", spVertexOrder[info].c_str());

            glGetProgramiv(program, GL_TESS_GEN_POINT_MODE, &info);
            _add_message("\tTess Gen Point Mode: %d", info);
        }
        _add_message("");
    }



    // Printf style! plus a newline at the end
    void OpenglInfo::_add_message(std::string format, ...) {
        char spAux[256];
        va_list args;
        va_start(args, format);
    #ifdef _WIN32
        vsprintf_s(spAux, format.c_str(), args);
    #else
        vsprintf(spAux, format.c_str(), args);
    #endif
        va_end(args);

        *_output_stream << spAux << "\n";
    }



    // aux function to get the size in bytes of a uniform
    // it takes the strides into account
    int OpenglInfo::_get_uniform_byte_size(int uniSize, int uniType, int uniArrayStride, int uniMatStride) {

        int auxSize;
        if (uniArrayStride > 0)
            auxSize = uniArrayStride * uniSize;

        else if (uniMatStride > 0) {

            switch (uniType) {
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_DOUBLE_MAT2:
            case GL_DOUBLE_MAT2x3:
            case GL_DOUBLE_MAT2x4:
                auxSize = 2 * uniMatStride;
                break;
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_DOUBLE_MAT3:
            case GL_DOUBLE_MAT3x2:
            case GL_DOUBLE_MAT3x4:
                auxSize = 3 * uniMatStride;
                break;
            case GL_FLOAT_MAT4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
            case GL_DOUBLE_MAT4:
            case GL_DOUBLE_MAT4x2:
            case GL_DOUBLE_MAT4x3:
                auxSize = 4 * uniMatStride;
                break;
            }
        }
        else
            auxSize = spGLSLTypeSize[uniType];

        return auxSize;
    }



    // aux function to display float based uniforms
    void OpenglInfo::_display_uniformf(float *f, int rows, int columns) {

        for (int i = 0; i < rows; ++i) {
            if (columns == 1)
                _add_message("%f", f[i*columns]);
            else if (columns == 2)
                _add_message("%f %f", f[i*columns], f[i*columns + 1]);
            else if (columns == 3)
                _add_message("%f %f %f", f[i*columns], f[i*columns + 1], f[i*columns + 2]);
            else if (columns == 4)
                _add_message("%f %f %f %f", f[i*columns], f[i*columns + 1], f[i*columns + 2], f[i*columns + 3]);
        }
    }


    // aux function to display int based uniforms
    void OpenglInfo::_display_uniformi(int *f, int rows, int columns) {

        for (int i = 0; i < rows; ++i) {
            if (columns == 1)
                _add_message("%d", f[i*columns]);
            else if (columns == 2)
                _add_message("%d %d", f[i*columns], f[i*columns + 1]);
            else if (columns == 3)
                _add_message("%d %d %d", f[i*columns], f[i*columns + 1], f[i*columns + 2]);
            else if (columns == 4)
                _add_message("%d %d %d %d", f[i*columns], f[i*columns + 1], f[i*columns + 2], f[i*columns + 3]);
        }
    }


    // aux function to display unsigned int based uniforms
    void OpenglInfo::_display_uniformui(unsigned int *f, int rows, int columns) {

        for (int i = 0; i < rows; ++i) {
            if (columns == 1)
                _add_message("%u", f[i*columns]);
            else if (columns == 2)
                _add_message("%u %u", f[i*columns], f[i*columns + 1]);
            else if (columns == 3)
                _add_message("%u %u %u", f[i*columns], f[i*columns + 1], f[i*columns + 2]);
            else if (columns == 4)
                _add_message("%u %u %u %u", f[i*columns], f[i*columns + 1], f[i*columns + 2], f[i*columns + 3]);
        }
    }


    // aux function to display double based uniforms
    void OpenglInfo::_display_uniformd(double *f, int rows, int columns) {

        for (int i = 0; i < rows; ++i) {
            if (columns == 1)
                _add_message("%f", f[i*columns]);
            else if (columns == 2)
                _add_message("%f %f", f[i*columns], f[i*columns + 1]);
            else if (columns == 3)
                _add_message("%f %f %f", f[i*columns], f[i*columns + 1], f[i*columns + 2]);
            else if (columns == 4)
                _add_message("%f %f %f %f", f[i*columns], f[i*columns + 1], f[i*columns + 2], f[i*columns + 3]);
        }

    }


    // gets the atomic data type
    OpenglInfo::Types OpenglInfo::_get_type(GLenum type) {
        switch (type) {
        case GL_DOUBLE:
        case GL_DOUBLE_MAT2:
        case GL_DOUBLE_MAT2x3:
        case GL_DOUBLE_MAT2x4:
        case GL_DOUBLE_MAT3:
        case GL_DOUBLE_MAT3x2:
        case GL_DOUBLE_MAT3x4:
        case GL_DOUBLE_MAT4:
        case GL_DOUBLE_MAT4x2:
        case GL_DOUBLE_MAT4x3:
        case GL_DOUBLE_VEC2:
        case GL_DOUBLE_VEC3:
        case GL_DOUBLE_VEC4:
            return OpenglInfo::DOUBLE;
        case GL_FLOAT:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
            return OpenglInfo::FLOAT;
        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
        case GL_INT:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
            return OpenglInfo::INT;
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_VEC2:
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
            return OpenglInfo::UNSIGNED_INT;

        default:
            return OpenglInfo::DONT_KNOW;

        }
    }



    // gets the number of rows for a GLSL type
    int OpenglInfo::_get_rows(GLenum type) {

        switch (type) {
        case GL_DOUBLE_MAT2:
        case GL_DOUBLE_MAT2x3:
        case GL_DOUBLE_MAT2x4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
            return 2;

        case GL_DOUBLE_MAT3:
        case GL_DOUBLE_MAT3x2:
        case GL_DOUBLE_MAT3x4:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
            return 3;

        case GL_DOUBLE_MAT4:
        case GL_DOUBLE_MAT4x2:
        case GL_DOUBLE_MAT4x3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
            return 4;

        default: return 1;
        }
    }


    // gets the number of columns for a GLSL type
    int OpenglInfo::_get_columns(GLenum type) {

        switch (type) {
        case GL_DOUBLE_MAT2:
        case GL_FLOAT_MAT2:
        case GL_DOUBLE_MAT3x2:
        case GL_FLOAT_MAT3x2:
        case GL_DOUBLE_MAT4x2:
        case GL_FLOAT_MAT4x2:
        case GL_UNSIGNED_INT_VEC2:
        case GL_INT_VEC2:
        case GL_BOOL_VEC2:
        case GL_FLOAT_VEC2:
        case GL_DOUBLE_VEC2:
            return 2;

        case GL_DOUBLE_MAT2x3:
        case GL_FLOAT_MAT2x3:
        case GL_DOUBLE_MAT3:
        case GL_FLOAT_MAT3:
        case GL_DOUBLE_MAT4x3:
        case GL_FLOAT_MAT4x3:
        case GL_UNSIGNED_INT_VEC3:
        case GL_INT_VEC3:
        case GL_BOOL_VEC3:
        case GL_FLOAT_VEC3:
        case GL_DOUBLE_VEC3:
            return 3;

        case GL_DOUBLE_MAT2x4:
        case GL_FLOAT_MAT2x4:
        case GL_DOUBLE_MAT3x4:
        case GL_FLOAT_MAT3x4:
        case GL_DOUBLE_MAT4:
        case GL_FLOAT_MAT4:
        case GL_UNSIGNED_INT_VEC4:
        case GL_INT_VEC4:
        case GL_BOOL_VEC4:
        case GL_FLOAT_VEC4:
        case GL_DOUBLE_VEC4:
            return 4;

        default: return 1;
        }
    }


    bool OpenglInfo::_init() {
        spTessGenSpacing[GL_EQUAL] = "GL_EQUAL";
        spTessGenSpacing[GL_FRACTIONAL_EVEN] = "GL_FRACTIONAL_EVEN";
        spTessGenSpacing[GL_FRACTIONAL_ODD] = "GL_FRACTIONAL_ODD";

        spVertexOrder[GL_CCW] = "GL_CCW";
        spVertexOrder[GL_CW] = "GL_CW";

        spGLSLPrimitives[GL_QUADS] = "GL_QUADS";
        spGLSLPrimitives[GL_ISOLINES] = "GL_ISOLINES";
        spGLSLPrimitives[GL_POINTS] = "GL_POINTS";
        spGLSLPrimitives[GL_LINES] = "GL_LINES";
        spGLSLPrimitives[GL_LINES_ADJACENCY] = "GL_LINES_ADJACENCY";
        spGLSLPrimitives[GL_TRIANGLES] = "GL_TRIANGLES";
        spGLSLPrimitives[GL_LINE_STRIP] = "GL_LINE_STRIP";
        spGLSLPrimitives[GL_TRIANGLE_STRIP] = "GL_TRIANGLE_STRIP";
        spGLSLPrimitives[GL_TRIANGLES_ADJACENCY] = "GL_TRIANGLES_ADJACENCY";

        spTransFeedBufferMode[GL_SEPARATE_ATTRIBS] = "GL_SEPARATE_ATTRIBS";
        spTransFeedBufferMode[GL_INTERLEAVED_ATTRIBS] = "GL_INTERLEAVED_ATTRIBS";

        spShaderType[GL_VERTEX_SHADER] = "GL_VERTEX_SHADER";
        spShaderType[GL_GEOMETRY_SHADER] = "GL_GEOMETRY_SHADER";
        spShaderType[GL_TESS_CONTROL_SHADER] = "GL_TESS_CONTROL_SHADER";
        spShaderType[GL_TESS_EVALUATION_SHADER] = "GL_TESS_EVALUATION_SHADER";
        spShaderType[GL_FRAGMENT_SHADER] = "GL_FRAGMENT_SHADER";
        spShaderType[GL_COMPUTE_SHADER] = "GL_COMPUTE_SHADER";

        spBufferBinding[GL_ARRAY_BUFFER_BINDING] = "GL_ARRAY_BUFFER";
        spBufferBinding[GL_ELEMENT_ARRAY_BUFFER_BINDING] = "GL_ELEMENT_ARRAY_BUFFER";
        spBufferBinding[GL_PIXEL_PACK_BUFFER_BINDING] = "GL_PIXEL_PACK_BUFFER";
        spBufferBinding[GL_PIXEL_UNPACK_BUFFER_BINDING] = "GL_PIXEL_UNPACK_BUFFER";
        spBufferBinding[GL_TRANSFORM_FEEDBACK_BUFFER_BINDING] = "GL_TRANSFORM_FEEDBACK_BUFFER";
        spBufferBinding[GL_UNIFORM_BUFFER_BINDING] = "GL_UNIFORM_BUFFER";
        spBufferBinding[GL_TEXTURE_BUFFER_BINDING] = "GL_TEXTURE_BUFFER";
        spBufferBinding[GL_COPY_READ_BUFFER_BINDING] = "GL_COPY_READ_BUFFER";
        spBufferBinding[GL_COPY_WRITE_BUFFER_BINDING] = "GL_COPY_WRITE_BUFFER";
        spBufferBinding[GL_DRAW_INDIRECT_BUFFER_BINDING] = "GL_DRAW_INDIRECT_BUFFER";
        spBufferBinding[GL_ATOMIC_COUNTER_BUFFER_BINDING] = "GL_ATOMIC_COUNTER_BUFFER";

        spBufferBound[GL_ARRAY_BUFFER_BINDING] = GL_ARRAY_BUFFER;
        spBufferBound[GL_ELEMENT_ARRAY_BUFFER_BINDING] = GL_ELEMENT_ARRAY_BUFFER;
        spBufferBound[GL_PIXEL_PACK_BUFFER_BINDING] = GL_PIXEL_PACK_BUFFER;
        spBufferBound[GL_PIXEL_UNPACK_BUFFER_BINDING] = GL_PIXEL_UNPACK_BUFFER;
        spBufferBound[GL_TRANSFORM_FEEDBACK_BUFFER_BINDING] = GL_TRANSFORM_FEEDBACK_BUFFER;
        spBufferBound[GL_UNIFORM_BUFFER_BINDING] = GL_UNIFORM_BUFFER;
        spBufferBound[GL_TEXTURE_BUFFER_BINDING] = GL_TEXTURE_BUFFER;
        spBufferBound[GL_COPY_READ_BUFFER_BINDING] = GL_COPY_READ_BUFFER;
        spBufferBound[GL_COPY_WRITE_BUFFER_BINDING] = GL_COPY_WRITE_BUFFER;
        spBufferBound[GL_DRAW_INDIRECT_BUFFER_BINDING] = GL_DRAW_INDIRECT_BUFFER;
        spBufferBound[GL_ATOMIC_COUNTER_BUFFER_BINDING] = GL_ATOMIC_COUNTER_BUFFER;

        spBoundBuffer[GL_ARRAY_BUFFER] = GL_ARRAY_BUFFER_BINDING;
        spBoundBuffer[GL_ELEMENT_ARRAY_BUFFER] = GL_ELEMENT_ARRAY_BUFFER_BINDING;
        spBoundBuffer[GL_PIXEL_PACK_BUFFER] = GL_PIXEL_PACK_BUFFER_BINDING;
        spBoundBuffer[GL_PIXEL_UNPACK_BUFFER] = GL_PIXEL_UNPACK_BUFFER_BINDING;
        spBoundBuffer[GL_TRANSFORM_FEEDBACK_BUFFER] = GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
        spBoundBuffer[GL_UNIFORM_BUFFER] = GL_UNIFORM_BUFFER_BINDING;
        spBoundBuffer[GL_TEXTURE_BUFFER] = GL_TEXTURE_BUFFER_BINDING;
        spBoundBuffer[GL_COPY_READ_BUFFER] = GL_COPY_READ_BUFFER_BINDING;
        spBoundBuffer[GL_COPY_WRITE_BUFFER] = GL_COPY_WRITE_BUFFER_BINDING;
        spBoundBuffer[GL_DRAW_INDIRECT_BUFFER] = GL_DRAW_INDIRECT_BUFFER;
        spBoundBuffer[GL_ATOMIC_COUNTER_BUFFER] = GL_ATOMIC_COUNTER_BUFFER;

        spBufferUsage[GL_STREAM_DRAW] = "GL_STREAM_DRAW";
        spBufferUsage[GL_STREAM_READ] = "GL_STREAM_READ";
        spBufferUsage[GL_STREAM_COPY] = "GL_STREAM_COPY";
        spBufferUsage[GL_STATIC_DRAW] = "GL_STATIC_DRAW";
        spBufferUsage[GL_STATIC_READ] = "GL_STATIC_READ";
        spBufferUsage[GL_STATIC_COPY] = "GL_STATIC_COPY";
        spBufferUsage[GL_DYNAMIC_DRAW] = "GL_DYNAMIC_DRAW";
        spBufferUsage[GL_DYNAMIC_READ] = "GL_DYNAMIC_READ";
        spBufferUsage[GL_DYNAMIC_COPY] = "GL_DYNAMIC_COPY";

        spBufferAccess[GL_READ_ONLY] = "GL_READ_ONLY";
        spBufferAccess[GL_WRITE_ONLY] = "GL_WRITE_ONLY";
        spBufferAccess[GL_READ_WRITE] = "GL_READ_WRITE";

        spGLSLTypeSize[GL_FLOAT] = sizeof(float);
        spGLSLTypeSize[GL_FLOAT_VEC2] = sizeof(float) * 2;
        spGLSLTypeSize[GL_FLOAT_VEC3] = sizeof(float) * 3;
        spGLSLTypeSize[GL_FLOAT_VEC4] = sizeof(float) * 4;
        spGLSLTypeSize[GL_DOUBLE] = sizeof(double);
        spGLSLTypeSize[GL_DOUBLE_VEC2] = sizeof(double) * 2;
        spGLSLTypeSize[GL_DOUBLE_VEC3] = sizeof(double) * 3;
        spGLSLTypeSize[GL_DOUBLE_VEC4] = sizeof(double) * 4;
        spGLSLTypeSize[GL_SAMPLER_1D] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_3D] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_CUBE] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_1D_SHADOW] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_SHADOW] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_1D_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_1D_ARRAY_SHADOW] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_ARRAY_SHADOW] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_MULTISAMPLE] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_MULTISAMPLE_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_CUBE_SHADOW] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_BUFFER] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_RECT] = sizeof(int);
        spGLSLTypeSize[GL_SAMPLER_2D_RECT_SHADOW] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_1D] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_2D] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_3D] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_CUBE] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_1D_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_2D_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_2D_MULTISAMPLE] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_BUFFER] = sizeof(int);
        spGLSLTypeSize[GL_INT_SAMPLER_2D_RECT] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_1D] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_2D] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_3D] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_CUBE] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_1D_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_2D_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_BUFFER] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_SAMPLER_2D_RECT] = sizeof(int);
        spGLSLTypeSize[GL_BOOL] = sizeof(int);
        spGLSLTypeSize[GL_INT] = sizeof(int);
        spGLSLTypeSize[GL_BOOL_VEC2] = sizeof(int) * 2;
        spGLSLTypeSize[GL_INT_VEC2] = sizeof(int) * 2;
        spGLSLTypeSize[GL_BOOL_VEC3] = sizeof(int) * 3;
        spGLSLTypeSize[GL_INT_VEC3] = sizeof(int) * 3;
        spGLSLTypeSize[GL_BOOL_VEC4] = sizeof(int) * 4;
        spGLSLTypeSize[GL_INT_VEC4] = sizeof(int) * 4;
        spGLSLTypeSize[GL_UNSIGNED_INT] = sizeof(int);
        spGLSLTypeSize[GL_UNSIGNED_INT_VEC2] = sizeof(int) * 2;
        spGLSLTypeSize[GL_UNSIGNED_INT_VEC3] = sizeof(int) * 2;
        spGLSLTypeSize[GL_UNSIGNED_INT_VEC4] = sizeof(int) * 2;
        spGLSLTypeSize[GL_FLOAT_MAT2] = sizeof(float) * 4;
        spGLSLTypeSize[GL_FLOAT_MAT3] = sizeof(float) * 9;
        spGLSLTypeSize[GL_FLOAT_MAT4] = sizeof(float) * 16;
        spGLSLTypeSize[GL_FLOAT_MAT2x3] = sizeof(float) * 6;
        spGLSLTypeSize[GL_FLOAT_MAT2x4] = sizeof(float) * 8;
        spGLSLTypeSize[GL_FLOAT_MAT3x2] = sizeof(float) * 6;
        spGLSLTypeSize[GL_FLOAT_MAT3x4] = sizeof(float) * 12;
        spGLSLTypeSize[GL_FLOAT_MAT4x2] = sizeof(float) * 8;
        spGLSLTypeSize[GL_FLOAT_MAT4x3] = sizeof(float) * 12;
        spGLSLTypeSize[GL_DOUBLE_MAT2] = sizeof(double) * 4;
        spGLSLTypeSize[GL_DOUBLE_MAT3] = sizeof(double) * 9;
        spGLSLTypeSize[GL_DOUBLE_MAT4] = sizeof(double) * 16;
        spGLSLTypeSize[GL_DOUBLE_MAT2x3] = sizeof(double) * 6;
        spGLSLTypeSize[GL_DOUBLE_MAT2x4] = sizeof(double) * 8;
        spGLSLTypeSize[GL_DOUBLE_MAT3x2] = sizeof(double) * 6;
        spGLSLTypeSize[GL_DOUBLE_MAT3x4] = sizeof(double) * 12;
        spGLSLTypeSize[GL_DOUBLE_MAT4x2] = sizeof(double) * 8;
        spGLSLTypeSize[GL_DOUBLE_MAT4x3] = sizeof(double) * 12;


        spGLSLType[GL_FLOAT] = "GL_FLOAT";
        spGLSLType[GL_FLOAT_VEC2] = "GL_FLOAT_VEC2";
        spGLSLType[GL_FLOAT_VEC3] = "GL_FLOAT_VEC3";
        spGLSLType[GL_FLOAT_VEC4] = "GL_FLOAT_VEC4";
        spGLSLType[GL_DOUBLE] = "GL_DOUBLE";
        spGLSLType[GL_DOUBLE_VEC2] = "GL_DOUBLE_VEC2";
        spGLSLType[GL_DOUBLE_VEC3] = "GL_DOUBLE_VEC3";
        spGLSLType[GL_DOUBLE_VEC4] = "GL_DOUBLE_VEC4";
        spGLSLType[GL_SAMPLER_1D] = "GL_SAMPLER_1D";
        spGLSLType[GL_SAMPLER_2D] = "GL_SAMPLER_2D";
        spGLSLType[GL_SAMPLER_3D] = "GL_SAMPLER_3D";
        spGLSLType[GL_SAMPLER_CUBE] = "GL_SAMPLER_CUBE";
        spGLSLType[GL_SAMPLER_1D_SHADOW] = "GL_SAMPLER_1D_SHADOW";
        spGLSLType[GL_SAMPLER_2D_SHADOW] = "GL_SAMPLER_2D_SHADOW";
        spGLSLType[GL_SAMPLER_1D_ARRAY] = "GL_SAMPLER_1D_ARRAY";
        spGLSLType[GL_SAMPLER_2D_ARRAY] = "GL_SAMPLER_2D_ARRAY";
        spGLSLType[GL_SAMPLER_1D_ARRAY_SHADOW] = "GL_SAMPLER_1D_ARRAY_SHADOW";
        spGLSLType[GL_SAMPLER_2D_ARRAY_SHADOW] = "GL_SAMPLER_2D_ARRAY_SHADOW";
        spGLSLType[GL_SAMPLER_2D_MULTISAMPLE] = "GL_SAMPLER_2D_MULTISAMPLE";
        spGLSLType[GL_SAMPLER_2D_MULTISAMPLE_ARRAY] = "GL_SAMPLER_2D_MULTISAMPLE_ARRAY";
        spGLSLType[GL_SAMPLER_CUBE_SHADOW] = "GL_SAMPLER_CUBE_SHADOW";
        spGLSLType[GL_SAMPLER_BUFFER] = "GL_SAMPLER_BUFFER";
        spGLSLType[GL_SAMPLER_2D_RECT] = "GL_SAMPLER_2D_RECT";
        spGLSLType[GL_SAMPLER_2D_RECT_SHADOW] = "GL_SAMPLER_2D_RECT_SHADOW";
        spGLSLType[GL_INT_SAMPLER_1D] = "GL_INT_SAMPLER_1D";
        spGLSLType[GL_INT_SAMPLER_2D] = "GL_INT_SAMPLER_2D";
        spGLSLType[GL_INT_SAMPLER_3D] = "GL_INT_SAMPLER_3D";
        spGLSLType[GL_INT_SAMPLER_CUBE] = "GL_INT_SAMPLER_CUBE";
        spGLSLType[GL_INT_SAMPLER_1D_ARRAY] = "GL_INT_SAMPLER_1D_ARRAY";
        spGLSLType[GL_INT_SAMPLER_2D_ARRAY] = "GL_INT_SAMPLER_2D_ARRAY";
        spGLSLType[GL_INT_SAMPLER_2D_MULTISAMPLE] = "GL_INT_SAMPLER_2D_MULTISAMPLE";
        spGLSLType[GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY] = "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
        spGLSLType[GL_INT_SAMPLER_BUFFER] = "GL_INT_SAMPLER_BUFFER";
        spGLSLType[GL_INT_SAMPLER_2D_RECT] = "GL_INT_SAMPLER_2D_RECT";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_1D] = "GL_UNSIGNED_INT_SAMPLER_1D";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_2D] = "GL_UNSIGNED_INT_SAMPLER_2D";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_3D] = "GL_UNSIGNED_INT_SAMPLER_3D";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_CUBE] = "GL_UNSIGNED_INT_SAMPLER_CUBE";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_1D_ARRAY] = "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_2D_ARRAY] = "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE] = "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY] = "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_BUFFER] = "GL_UNSIGNED_INT_SAMPLER_BUFFER";
        spGLSLType[GL_UNSIGNED_INT_SAMPLER_2D_RECT] = "GL_UNSIGNED_INT_SAMPLER_2D_RECT";
        spGLSLType[GL_BOOL] = "GL_BOOL";
        spGLSLType[GL_INT] = "GL_INT";
        spGLSLType[GL_BOOL_VEC2] = "GL_BOOL_VEC2";
        spGLSLType[GL_INT_VEC2] = "GL_INT_VEC2";
        spGLSLType[GL_BOOL_VEC3] = "GL_BOOL_VEC3";
        spGLSLType[GL_INT_VEC3] = "GL_INT_VEC3";
        spGLSLType[GL_BOOL_VEC4] = "GL_BOOL_VEC4";
        spGLSLType[GL_INT_VEC4] = "GL_INT_VEC4";
        spGLSLType[GL_UNSIGNED_INT] = "GL_UNSIGNED_INT";
        spGLSLType[GL_UNSIGNED_INT_VEC2] = "GL_UNSIGNED_INT_VEC2";
        spGLSLType[GL_UNSIGNED_INT_VEC3] = "GL_UNSIGNED_INT_VEC3";
        spGLSLType[GL_UNSIGNED_INT_VEC4] = "GL_UNSIGNED_INT_VEC4";
        spGLSLType[GL_FLOAT_MAT2] = "GL_FLOAT_MAT2";
        spGLSLType[GL_FLOAT_MAT3] = "GL_FLOAT_MAT3";
        spGLSLType[GL_FLOAT_MAT4] = "GL_FLOAT_MAT4";
        spGLSLType[GL_FLOAT_MAT2x3] = "GL_FLOAT_MAT2x3";
        spGLSLType[GL_FLOAT_MAT2x4] = "GL_FLOAT_MAT2x4";
        spGLSLType[GL_FLOAT_MAT3x2] = "GL_FLOAT_MAT3x2";
        spGLSLType[GL_FLOAT_MAT3x4] = "GL_FLOAT_MAT3x4";
        spGLSLType[GL_FLOAT_MAT4x2] = "GL_FLOAT_MAT4x2";
        spGLSLType[GL_FLOAT_MAT4x3] = "GL_FLOAT_MAT4x3";
        spGLSLType[GL_DOUBLE_MAT2] = "GL_DOUBLE_MAT2";
        spGLSLType[GL_DOUBLE_MAT3] = "GL_DOUBLE_MAT3";
        spGLSLType[GL_DOUBLE_MAT4] = "GL_DOUBLE_MAT4";
        spGLSLType[GL_DOUBLE_MAT2x3] = "GL_DOUBLE_MAT2x3";
        spGLSLType[GL_DOUBLE_MAT2x4] = "GL_DOUBLE_MAT2x4";
        spGLSLType[GL_DOUBLE_MAT3x2] = "GL_DOUBLE_MAT3x2";
        spGLSLType[GL_DOUBLE_MAT3x4] = "GL_DOUBLE_MAT3x4";
        spGLSLType[GL_DOUBLE_MAT4x2] = "GL_DOUBLE_MAT4x2";
        spGLSLType[GL_DOUBLE_MAT4x3] = "GL_DOUBLE_MAT4x3";


        spDataF[GL_UNSIGNED_BYTE] = "GL_UNSIGNED_BYTE";
        spDataF[GL_BYTE] = "GL_BYTE";
        spDataF[GL_UNSIGNED_SHORT] = "GL_UNSIGNED_SHORT";
        spDataF[GL_SHORT] = "GL_SHORT";
        spDataF[GL_UNSIGNED_INT] = "GL_UNSIGNED_INT";
        spDataF[GL_INT] = "GL_INT";
        spDataF[GL_HALF_FLOAT] = "GL_HALF_FLOAT";
        spDataF[GL_FLOAT] = "GL_FLOAT";
        spDataF[GL_UNSIGNED_BYTE_3_3_2] = "GL_UNSIGNED_BYTE_3_3_2";
        spDataF[GL_UNSIGNED_BYTE_2_3_3_REV] = "GL_UNSIGNED_BYTE_2_3_3_REV";
        spDataF[GL_UNSIGNED_SHORT_5_6_5] = "GL_UNSIGNED_SHORT_5_6_5";
        spDataF[GL_UNSIGNED_SHORT_5_6_5_REV] = "GL_UNSIGNED_SHORT_5_6_5_REV";
        spDataF[GL_UNSIGNED_SHORT_4_4_4_4] = "GL_UNSIGNED_SHORT_4_4_4_4";
        spDataF[GL_UNSIGNED_SHORT_4_4_4_4_REV] = "GL_UNSIGNED_SHORT_4_4_4_4_REV";
        spDataF[GL_UNSIGNED_SHORT_5_5_5_1] = "GL_UNSIGNED_SHORT_5_5_5_1";
        spDataF[GL_UNSIGNED_SHORT_1_5_5_5_REV] = "GL_UNSIGNED_SHORT_1_5_5_5_REV";
        spDataF[GL_UNSIGNED_INT_8_8_8_8] = "GL_UNSIGNED_INT_8_8_8_8";
        spDataF[GL_UNSIGNED_INT_8_8_8_8_REV] = "GL_UNSIGNED_INT_8_8_8_8_REV";
        spDataF[GL_UNSIGNED_INT_10_10_10_2] = "GL_UNSIGNED_INT_10_10_10_2";
        spDataF[GL_UNSIGNED_INT_2_10_10_10_REV] = "GL_UNSIGNED_INT_2_10_10_10_REV";
        return true;
    }

}
