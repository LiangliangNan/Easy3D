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

/** ----------------------------------------------------------
 *
 * the code is adapted from ShaderProgram with modifications.
 *		- Very Simple Shader Library (version 0.2.2)
 * Full documentation at
 * http://www.lighthouse3d.com/very-simple-libs
 *
 * This class allows to create programs, load shaders from files,
 * associate vertex attribute names with locations, and work with
 * uniforms, including blocks. It also provides access to the
 * info logs, and many more useful functionalities.
 *
 *----------------------------------------------------------*/


#include <easy3d/renderer/shader_program.h>

#include <cassert>
#include <iostream>
#include <fstream>

#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/util/logging.h>


namespace easy3d {

	GLenum ShaderProgram::spGLShaderTypes[ShaderProgram::NUM_SHADER_TYPES] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_GEOMETRY_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_COMPUTE_SHADER };


	std::string ShaderProgram::spStringShaderTypes[ShaderProgram::NUM_SHADER_TYPES] = {
		"Vertex Shader",
		"Fragment Shader",
		"Geometry Shader",
		"Tessellation Control Shader",
		"Tessellation Evaluation Shader",
		"Compute Shader" };

    std::unordered_map<std::string, ShaderProgram::UniformBlock> ShaderProgram::spBlocks;

	int ShaderProgram::spBlockCount = 1;


    bool ShaderProgram::is_supported() {
        return OpenglUtil::is_supported("GL_VERSION_2_0") || (
            OpenglUtil::is_supported("GL_ARB_shader_objects") &&
            OpenglUtil::is_supported("GL_ARB_shading_language_100") &&
            OpenglUtil::is_supported("GL_ARB_vertex_shader") &&
            OpenglUtil::is_supported("GL_ARB_fragment_shader")) || (
            OpenglUtil::has_extension("GL_EXT_shader_objects") &&
            OpenglUtil::has_extension("GL_EXT_shading_language_100") &&
            OpenglUtil::has_extension("GL_EXT_vertex_shader") &&
            OpenglUtil::has_extension("GL_EXT_fragment_shader"));
    }


    ShaderProgram::ShaderProgram(const std::string& name)
        : program_(0)
        , name_(name)
        , verbose_(true)
	{
        if (!is_supported()) {
            LOG(ERROR) << "shader program not supported on this platform";
            return;
        }

        // Liangliang: it is a bad idea to initialize some OpenGL stuff in the constructors
        //			   because the OpenGL context may not exist or the current context is
        //             not the correct one. Do "create when needed".
        // program_ = glCreateProgram();
	}


	ShaderProgram::~ShaderProgram() {
		clear();
	}


	void ShaderProgram::clear() {
		if (is_bound())
            release();

		if (glIsProgram(program_))
			glDeleteProgram(program_);	

		pUniforms.clear();
	}


	std::string ShaderProgram::load_shader_source(const std::string &file_name, const std::string& include_identifier) {
        // get the path without file name
        auto get_path = [](const std::string& full_path) -> std::string {
            // Remove the file name and store the path to this folder
            auto pos = full_path.find_last_of("/\\");
            return full_path.substr(0, pos + 1);
        };

		std::string code;
		std::ifstream in(file_name.c_str());
		if (in.fail()) {
			LOG(ERROR) << "could not open file: " << file_name;
			return code;
		}

        std::string inc_id = include_identifier + ' ';
        static bool is_recursive_call = false;

		std::string line;
		while (in.good() && std::getline(in, line)) {
            // Look for the new shader include identifier
            if (line.find(inc_id) != std::string::npos)
            {
                // Remove the "include" identifier, this will cause the path to remain
                line.erase(0, inc_id.size());

                // The "include" path is relative to the current shader in path
                std::string path = get_path(file_name);
                line.insert(0, path);

                // By using recursion, the new include in can be extracted
                // and inserted at this location in the shader source code
                is_recursive_call = true;
                code += load_shader_source(line, include_identifier);

                // Do not add this line to the shader source code, as the "include"
                // path would generate a compilation issue in the final source code
                continue;
            }

            code += line + '\n';
        }

        // Only add the null terminator at the end of the complete in,
        // essentially skipping recursive function calls this way
        if (!is_recursive_call)
            code += '\0';

        in.close();

		return code;
	}


    std::string ShaderProgram::_read_linked_program(const std::string& file_name) {
        std::string code;

        //must read files as binary to prevent problems from newline translation
        std::ifstream in(file_name.c_str(), std::ios::binary);
        if (in.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return code;
        }

        in.seekg(0, std::ios::end);
        std::fstream::pos_type length = in.tellg();
        code.resize(length);
        in.seekg(0, std::ios::beg);

        in.read(&(code[0]), length);

        return code;
    }


	bool ShaderProgram::load_shader_from_file(ShaderType type, const std::string& file_name, const std::string& inc_id) {
		std::string code = load_shader_source(file_name, inc_id);
		if (code.empty()) {
			LOG(ERROR) << "failed reading shader file \'" << file_name << "\'";
			return false;
		}
		return load_shader_from_code(type, code);
	}


	bool ShaderProgram::load_shader_from_code(ShaderType type, const std::string& str) {
		if (str.empty()) {
            LOG(ERROR) << "empty code for " << spStringShaderTypes[type] << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return false;
		}
        unsigned int shader = glCreateShader(spGLShaderTypes[type]);
		easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error
		if (!glIsShader(shader)) {
			LOG(ERROR) << "failed to creat " << spStringShaderTypes[type] << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return false;
		}

		const char* code = str.c_str();
        glShaderSource(shader, 1, &code, nullptr);
		glCompileShader(shader);	

		//we must check the compilation result
		std::string log;
		if (!shader_info_log(log, shader)) {
			LOG(ERROR) << log << " - in " << spStringShaderTypes[type] << " " << (name_.empty() ? "" : "\'" +  name_ + "\'");
			glDeleteShader(shader);
			return false;
		}

        if (program_ == 0)
			program_ = glCreateProgram();	

		glAttachShader(program_, shader);	

        // now the shader can be deleted since the program
        // will keep a reference
        glDeleteShader(shader);
			
		return true;
	}


	bool ShaderProgram::link_program() {
		glLinkProgram(program_);	

		//we must check the linkage result
		std::string log;
		if (!program_info_log(log)) {
            LOG(ERROR) << log << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			clear(); // we don't need the program anymore; Also don't leak shaders either.
			return false;
		}

		_add_uniforms();	
		_add_blocks();		

#if 0
		// print some hints for user
		if (OpenglUtil::is_supported("GL_ARB_program_interface_query")) {
			LOG(INFO) << "--------------- shader program: " << name() << " ---------------";
			print_active_attributes();
			print_active_uniforms();
			print_active_uniform_blocks();
		}
#endif

		return true;
	}


	/// returns a string with a shader's infolog
    bool ShaderProgram::shader_info_log(std::string& log, unsigned int shader) const {
		log.clear();
        int compiled = false;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
			int log_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
            if (log_length) {
                log.append(log_length + 1, '\0');
                glGetShaderInfoLog(shader, log_length, nullptr, &log[0]);
            }
			return false;
		}
        return true;
	}


	/// returns a string with the program's infolog
	bool ShaderProgram::program_info_log(std::string& log) const {
		log.clear();
        int linked = false;
        glGetProgramiv(program_, GL_LINK_STATUS, &linked);
        if (!linked) {
			int log_length = 0;
			glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);	
            if (log_length) {
                log.append(log_length + 1, '\0');
                glGetProgramInfoLog(program_, log_length, nullptr, &log[0]);
            }
            return false;
		}
        return true;
	}


	void ShaderProgram::set_program_output(int index, const std::string& name) const {
		glBindFragDataLocation(program_, index, name.c_str());	
	}


    int ShaderProgram::program_output(const std::string& name) const {
		return glGetFragDataLocation(program_, name.c_str());	
	}


	void ShaderProgram::set_attrib_name(ShaderProgram::AttribType at, const std::string& name) const {
		glBindAttribLocation(program_, at, name.c_str());	
	}


	void ShaderProgram::set_attrib_names(const std::vector<ShaderProgram::Attribute>& attributes) const {
		for (const auto& attr : attributes) {
			set_attrib_name(attr.first, attr.second);
		}
	}

	int ShaderProgram::get_attribute_location(const std::string& name) const {
        return glGetAttribLocation(program_, name.c_str());
	}


    unsigned int ShaderProgram::get_program() const {
		return program_;
	}


	ShaderProgram* ShaderProgram::set_block(const std::string& name, const void *value) {
		if (spBlocks.count(name) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "block \'" << name << "\' does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, spBlocks[name].buffer);	
		glBufferSubData(GL_UNIFORM_BUFFER, 0, spBlocks[name].size, value);	
		glBindBuffer(GL_UNIFORM_BUFFER, 0);	
		return this;
	}


	ShaderProgram* ShaderProgram::set_block_uniform(const std::string& blockName, const std::string& uniformName, const void *value) {
		//assert(spBlocks.count(blockName) && spBlocks[blockName].uniformOffsets.count(uniformName));
		if (spBlocks.count(blockName) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "block " << blockName << " does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		UniformBlock b = spBlocks[blockName];
		if (b.uniformOffsets.count(uniformName) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "block/uniform " << blockName << "/" << uniformName << " does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		BlockUniform bUni = b.uniformOffsets[uniformName];
		glBindBuffer(GL_UNIFORM_BUFFER, b.buffer);	
		glBufferSubData(GL_UNIFORM_BUFFER, bUni.offset, bUni.size, value);	
		glBindBuffer(GL_UNIFORM_BUFFER, 0);	
		return this;
	}


	ShaderProgram* ShaderProgram::set_block_uniform_array_element(const std::string& blockName, const std::string& uniformName, int arrayIndex, const void * value) {
		//assert(spBlocks.count(blockName) && spBlocks[blockName].uniformOffsets.count(uniformName));
		if (spBlocks.count(blockName) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "block " << blockName << " does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		UniformBlock b = spBlocks[blockName];
		if (b.uniformOffsets.count(uniformName) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "block/uniform " << blockName << "/" << uniformName << " does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		BlockUniform bUni = b.uniformOffsets[uniformName];
		glBindBuffer(GL_UNIFORM_BUFFER, b.buffer);	
		glBufferSubData(GL_UNIFORM_BUFFER, bUni.offset + bUni.arrayStride * arrayIndex, bUni.arrayStride, value);	
		glBindBuffer(GL_UNIFORM_BUFFER, 0);	
		return this;
	}


	ShaderProgram* ShaderProgram::set_uniform(const std::string& name, int value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "uniform \'" << name << "\' does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		Uniform u = pUniforms[name];
		glProgramUniform1i(program_, u.location, value);
		return this;
	}


	ShaderProgram* ShaderProgram::set_uniform(const std::string& name, unsigned int value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
            LOG_IF_FIRST_N(3, verbose_, WARNING) << "uniform \'" << name << "\' does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		Uniform u = pUniforms[name];
		glProgramUniform1ui(program_, u.location, value);	
		return this;
	}


	ShaderProgram* ShaderProgram::set_uniform(const std::string& name, float value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
            LOG_IF_FIRST_N(3, verbose_, WARNING) << "uniform \'" << name << "\' does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		Uniform u = pUniforms[name];
		glProgramUniform1f(program_, u.location, value);
		return this;
	}


	ShaderProgram* ShaderProgram::set_uniform(const std::string& name, const void *value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
			LOG_IF_FIRST_N(3, verbose_, WARNING) << "uniform \'" << name << "\' does not exist or is not active. " << COUNTER  << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return this;
		}

		Uniform u = pUniforms[name];

		switch (u.type) {

			// Floats
		case GL_FLOAT:
			glProgramUniform1fv(program_, u.location, u.size, (const GLfloat *)value);	
			break;
		case GL_FLOAT_VEC2:
			glProgramUniform2fv(program_, u.location, u.size, (const GLfloat *)value);	
			break;
		case GL_FLOAT_VEC3:
			glProgramUniform3fv(program_, u.location, u.size, (const GLfloat *)value);	
			break;
		case GL_FLOAT_VEC4:
			glProgramUniform4fv(program_, u.location, u.size, (const GLfloat *)value);	
			break;

			// Doubles
		case GL_DOUBLE:
			glProgramUniform1dv(program_, u.location, u.size, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_VEC2:
			glProgramUniform2dv(program_, u.location, u.size, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_VEC3:
			glProgramUniform3dv(program_, u.location, u.size, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_VEC4:
			glProgramUniform4dv(program_, u.location, u.size, (const GLdouble *)value);	
			break;

			// Samplers, Ints and Bools
		case GL_IMAGE_1D:
		case GL_IMAGE_2D:
		case GL_IMAGE_3D:
		case GL_IMAGE_2D_RECT:
		case GL_IMAGE_CUBE:
		case GL_IMAGE_BUFFER:
		case GL_IMAGE_1D_ARRAY:
		case GL_IMAGE_2D_ARRAY:
		case GL_IMAGE_CUBE_MAP_ARRAY:
		case GL_IMAGE_2D_MULTISAMPLE:
		case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
		case GL_INT_IMAGE_1D:
		case GL_INT_IMAGE_2D:
		case GL_INT_IMAGE_3D:
		case GL_INT_IMAGE_2D_RECT:
		case GL_INT_IMAGE_CUBE:
		case GL_INT_IMAGE_BUFFER:
		case GL_INT_IMAGE_1D_ARRAY:
		case GL_INT_IMAGE_2D_ARRAY:
		case GL_INT_IMAGE_CUBE_MAP_ARRAY:
		case GL_INT_IMAGE_2D_MULTISAMPLE:
		case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_1D:
		case GL_UNSIGNED_INT_IMAGE_2D:
		case GL_UNSIGNED_INT_IMAGE_3D:
		case GL_UNSIGNED_INT_IMAGE_2D_RECT:
		case GL_UNSIGNED_INT_IMAGE_CUBE:
		case GL_UNSIGNED_INT_IMAGE_BUFFER:
		case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
		case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_SAMPLER_BUFFER:
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_INT_SAMPLER_BUFFER:
		case GL_INT_SAMPLER_2D_RECT:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
		case GL_BOOL:
		case GL_INT:
            glProgramUniform1iv(program_, u.location, u.size, (const int *)value);
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
            glProgramUniform2iv(program_, u.location, u.size, (const int *)value);
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
            glProgramUniform3iv(program_, u.location, u.size, (const int *)value);
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
            glProgramUniform4iv(program_, u.location, u.size, (const int *)value);
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
            glProgramUniform1uiv(program_, u.location, u.size, (const unsigned int *)value);
			break;
		case GL_UNSIGNED_INT_VEC2:
            glProgramUniform2uiv(program_, u.location, u.size, (const unsigned int *)value);
			break;
		case GL_UNSIGNED_INT_VEC3:
            glProgramUniform3uiv(program_, u.location, u.size, (const unsigned int *)value);
			break;
		case GL_UNSIGNED_INT_VEC4:
            glProgramUniform4uiv(program_, u.location, u.size, (const unsigned int *)value);
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			glProgramUniformMatrix2fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT3:
			glProgramUniformMatrix3fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT4:
			glProgramUniformMatrix4fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT2x3:
			glProgramUniformMatrix2x3fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT2x4:
			glProgramUniformMatrix2x4fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT3x2:
			glProgramUniformMatrix3x2fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT3x4:
			glProgramUniformMatrix3x4fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT4x2:
			glProgramUniformMatrix4x2fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT4x3:
			glProgramUniformMatrix4x3fv(program_, u.location, u.size, false, (const GLfloat *)value);	
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			glProgramUniformMatrix2dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT3:
			glProgramUniformMatrix3dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT4:
			glProgramUniformMatrix4dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT2x3:
			glProgramUniformMatrix2x3dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT2x4:
			glProgramUniformMatrix2x4dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT3x2:
			glProgramUniformMatrix3x2dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT3x4:
			glProgramUniformMatrix3x4dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT4x2:
			glProgramUniformMatrix4x2dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT4x3:
			glProgramUniformMatrix4x3dv(program_, u.location, u.size, false, (const GLdouble *)value);	
			break;
		}

		return this;
	}


	bool ShaderProgram::is_program_valid() const {
        if (!program_) {
            LOG(ERROR) << "program does not exist. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return false;
        }

        int b = GL_FALSE;
        glValidateProgram(program_);
        glGetProgramiv(program_, GL_VALIDATE_STATUS, &b);

        if (!b) {
            int log_length = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);
            if (log_length > 0) {
                std::string log(log_length + 1, '\0');
                glGetProgramInfoLog(program_, log_length, nullptr, &log[0]);
                LOG(ERROR) << log << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            }
		}
		return (b != GL_FALSE);
	}


	bool ShaderProgram::is_bound() const {
        int current_program = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);	
        return (current_program == (int)program_);
	}


	bool ShaderProgram::is_program_linked() const {
        int b = GL_FALSE;
		if (program_) {
			glGetProgramiv(program_, GL_LINK_STATUS, &b);	
		}
		return (b != GL_FALSE);
	}


	void ShaderProgram::bind() const {
        // glValidateProgram() is meant to be called directly before a
        // draw call (i.e., glDraw*()) with that shader bound and all
        // the bindings (VAO, textures) set. Its purpose is to ensure
        // that the shader can execute given the current GL state.
//        if (!is_program_valid())
//            LOG(ERROR) << "program exists but not valid";
		glUseProgram(program_);	
	}


    void ShaderProgram::release() const {
		glUseProgram(0);		
	}


	ShaderProgram* ShaderProgram::bind_texture(const std::string& name, unsigned int tex_id, int unit, GLenum tex_target /* = GL_TEXTURE_2D */)
	{
        glActiveTexture(GL_TEXTURE0 + unit);     easy3d_debug_log_gl_error
        glBindTexture(tex_target, tex_id);    easy3d_debug_log_gl_error
        set_uniform(name, unit);	    easy3d_debug_log_gl_error
		return this;
	}


	ShaderProgram* ShaderProgram::release_texture(GLenum tex_target /* = GL_TEXTURE_2D */) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_target, 0);
		return this;
	}


	bool ShaderProgram::is_attribute_used(const std::string& name) {
		if (name.empty()) {
			LOG(ERROR) << "empty attribute name. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return false;
		}

		if (!is_program_linked()) {
			LOG(ERROR) << "program not linked. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return false;
		}

        int loc = glGetAttribLocation(program_, name.c_str());
		return (loc != -1);
	}


	bool ShaderProgram::is_uniform_used(const std::string& name) {
		if (name.empty()) {
			LOG(ERROR) << "empty uniform name. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return false;
		}

		if (!is_program_linked()) {
			LOG(ERROR) << "program not linked. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return false;
		}

        int loc = glGetUniformLocation(program_, name.c_str());
		return (loc != -1);
	}


	void ShaderProgram::print_active_attributes() {
		if (!is_program_linked()) {
			LOG(ERROR) << "program not linked. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return;
		}

        if (!OpenglUtil::is_supported("GL_ARB_program_interface_query")) {
            LOG(ERROR) << "querying active attributes requires OpenGL >= 4.3. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return;
        }

        int numAttribs;
		glGetProgramInterfaceiv(program_, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

		GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

		LOG(INFO) << "Active attributes:";
		for (int i = 0; i < numAttribs; ++i) {
            int results[3];
            glGetProgramResourceiv(program_, GL_PROGRAM_INPUT, i, 3, properties, 3, nullptr, results);

            int nameBufSize = results[0] + 1;
			char * name = new char[nameBufSize];
            glGetProgramResourceName(program_, GL_PROGRAM_INPUT, i, nameBufSize, nullptr, name);
			LOG(INFO) << "   " << results[2] << "\t\'" << name << "\'\t" << _type_string(results[1]);
			delete[] name;
		}
	}


	void ShaderProgram::print_active_uniforms() {
		if (!is_program_linked()) {
			LOG(ERROR) << "program not linked. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return;
		}

        if (!OpenglUtil::is_supported("GL_ARB_program_interface_query")) {
            LOG(ERROR) << "querying active uniforms requires OpenGL >= 4.3. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return;
        }

        int numUniforms = 0;
		glGetProgramInterfaceiv(program_, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);	

		GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

		LOG(INFO) << "Active uniforms:";
		for (int i = 0; i < numUniforms; ++i) {
            int results[4];
            glGetProgramResourceiv(program_, GL_UNIFORM, i, 4, properties, 4, nullptr, results);
			if (results[3] != -1)
				continue;  // Skip uniforms in blocks 
            int nameBufSize = results[0] + 1;
			char * name = new char[nameBufSize];
            glGetProgramResourceName(program_, GL_UNIFORM, i, nameBufSize, nullptr, name);
			LOG(INFO) << "   " << results[2] << "\t\'" << name << "\'\t" << _type_string(results[1]) << "";
			delete[] name;
		}
	}


	void ShaderProgram::print_active_uniform_blocks() {
		if (!is_program_linked()) {
			LOG(ERROR) << "program not linked. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
			return;
		}

        if (!OpenglUtil::is_supported("GL_ARB_program_interface_query")) {
            LOG(ERROR) << "querying active uniform blocks requires OpenGL >= 4.3. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return;
        }

        int numBlocks = 0;

		glGetProgramInterfaceiv(program_, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);	
		GLenum blockProps[] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH };
		GLenum blockIndex[] = { GL_ACTIVE_VARIABLES };
		GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX };

		for (int block = 0; block < numBlocks; ++block) {
            int blockInfo[2];
            glGetProgramResourceiv(program_, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, nullptr, blockInfo);
            int numUnis = blockInfo[0];

			char * blockName = new char[blockInfo[1] + 1];
            glGetProgramResourceName(program_, GL_UNIFORM_BLOCK, block, blockInfo[1] + 1, nullptr, blockName);
			LOG(INFO) << "Uniform block \"" << blockName << "\"\t";
			delete[] blockName;

            int * unifIndexes = new int[numUnis];
            glGetProgramResourceiv(program_, GL_UNIFORM_BLOCK, block, 1, blockIndex, numUnis, nullptr, unifIndexes);

			for (int unif = 0; unif < numUnis; ++unif) {
                int uniIndex = unifIndexes[unif];
                int results[3];
                glGetProgramResourceiv(program_, GL_UNIFORM, uniIndex, 3, props, 3, nullptr, results);

                int nameBufSize = results[0] + 1;
				char * name = new char[nameBufSize];
                glGetProgramResourceName(program_, GL_UNIFORM, uniIndex, nameBufSize, nullptr, name);
				LOG(INFO) << "   " << results[2] << "\t\'" << name << "\'\t" << _type_string(results[1]) << "";
				delete[] name;
			}

			delete[] unifIndexes;
		}
	}


	void ShaderProgram::_add_blocks() {
		int count, dataSize, actualLen, activeUnif, maxUniLength;
		int uniType, uniSize, uniOffset, uniMatStride, uniArrayStride, auxSize;
		char *name, *name2;

		UniformBlock block;

		glGetProgramiv(program_, GL_ACTIVE_UNIFORM_BLOCKS, &count);	

		for (int i = 0; i < count; ++i) {
			// Get buffers name
			glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &actualLen);	
			name = (char *)malloc(sizeof(char) * actualLen);
            glGetActiveUniformBlockName(program_, i, actualLen, nullptr, name);

			if (!spBlocks.count(name)) {
				// Get buffers size
				block = spBlocks[name];
				glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);	
				//printf("DataSize:%d\n", dataSize);
				glGenBuffers(1, &block.buffer);	
				glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);	
                glBufferData(GL_UNIFORM_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
				glUniformBlockBinding(program_, i, spBlockCount);	
				glBindBufferRange(GL_UNIFORM_BUFFER, spBlockCount, block.buffer, 0, dataSize);	

				glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUnif);	

				unsigned int *indices;
				indices = (unsigned int *)malloc(sizeof(unsigned int) * activeUnif);
				glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (int *)indices);	

				glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);	
				name2 = (char *)malloc(sizeof(char) * maxUniLength);

				for (int k = 0; k < activeUnif; ++k) {

					BlockUniform bUni;

					glGetActiveUniformName(program_, indices[k], maxUniLength, &actualLen, name2);	
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_TYPE, &uniType);	
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_SIZE, &uniSize);	
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_OFFSET, &uniOffset);	
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_MATRIX_STRIDE, &uniMatStride);	
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);	

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
                        default:
                            LOG(WARNING) << "unknown uniform type: " << uniType;
                            break;
						}
					}
					else
						auxSize = _type_size(uniType);

                    bUni.name = name2;
					bUni.offset = uniOffset;
					bUni.type = uniType;
					bUni.size = auxSize;
					bUni.arrayStride = uniArrayStride;

					block.uniformOffsets[name2] = bUni;
				}
				free(name2);

                block.name = name;
				block.size = dataSize;
				block.bindingIndex = spBlockCount;
				spBlocks[name] = block;
				spBlockCount++;
			}
			else
				glUniformBlockBinding(program_, i, spBlocks[name].bindingIndex);	
		}
	}


	void  ShaderProgram::_add_uniforms() {
		int count;
		GLsizei actualLen;
        int size;
        int uniArrayStride;
		GLenum type;
		char *name;

		int maxUniLength;
		glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);	

		glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);	

		name = (char *)malloc(sizeof(char) * maxUniLength);

		unsigned int loc;
		for (int i = 0; i < count; ++i) {
			glGetActiveUniform(program_, i, maxUniLength, &actualLen, &size, &type, name);	
			// -1 indicates that is not an active uniform, although it may be present in a uniform block
			loc = glGetUniformLocation(program_, name);	
			if (loc != -1) {
                glGetActiveUniformsiv(program_, 1, (unsigned int*)&i, GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);
				_add_uniform(name, type, size);
			}
		}
		free(name);
	}


	void ShaderProgram::_add_uniform(const std::string& name, GLenum type, unsigned int size) {
		Uniform u;
        u.name = name;
		u.type = type;
		u.location = glGetUniformLocation(program_, name.c_str());	
		u.size = static_cast<int>(size);
		pUniforms[name] = u;
	}


	int ShaderProgram::_type_size(GLenum type) const {
		int s;
		switch (type) {

		case GL_FLOAT:
			s = sizeof(float);
			break;
		case GL_FLOAT_VEC2:
			s = sizeof(float) * 2;
			break;
		case GL_FLOAT_VEC3:
			s = sizeof(float) * 3;
			break;
		case GL_FLOAT_VEC4:
			s = sizeof(float) * 4;
			break;

			// Doubles
		case GL_DOUBLE:
			s = sizeof(double);
			break;
		case GL_DOUBLE_VEC2:
			s = sizeof(double) * 2;
			break;
		case GL_DOUBLE_VEC3:
			s = sizeof(double) * 3;
			break;
		case GL_DOUBLE_VEC4:
			s = sizeof(double) * 4;
			break;

			// Samplers, Ints and Bools
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_SAMPLER_BUFFER:
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_INT_SAMPLER_BUFFER:
		case GL_INT_SAMPLER_2D_RECT:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
		case GL_BOOL:
		case GL_INT:
			s = sizeof(int);
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			s = sizeof(int) * 2;
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			s = sizeof(int) * 3;
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			s = sizeof(int) * 4;
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			s = sizeof(unsigned int);
			break;
		case GL_UNSIGNED_INT_VEC2:
			s = sizeof(unsigned int) * 2;
			break;
		case GL_UNSIGNED_INT_VEC3:
			s = sizeof(unsigned int) * 3;
			break;
		case GL_UNSIGNED_INT_VEC4:
			s = sizeof(unsigned int) * 4;
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			s = sizeof(float) * 4;
			break;
		case GL_FLOAT_MAT3:
			s = sizeof(float) * 9;
			break;
		case GL_FLOAT_MAT4:
			s = sizeof(float) * 16;
			break;
		case GL_FLOAT_MAT2x3:
			s = sizeof(float) * 6;
			break;
		case GL_FLOAT_MAT2x4:
			s = sizeof(float) * 8;
			break;
		case GL_FLOAT_MAT3x2:
			s = sizeof(float) * 6;
			break;
		case GL_FLOAT_MAT3x4:
			s = sizeof(float) * 12;
			break;
		case GL_FLOAT_MAT4x2:
			s = sizeof(float) * 8;
			break;
		case GL_FLOAT_MAT4x3:
			s = sizeof(float) * 12;
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			s = sizeof(double) * 4;
			break;
		case GL_DOUBLE_MAT3:
			s = sizeof(double) * 9;
			break;
		case GL_DOUBLE_MAT4:
			s = sizeof(double) * 16;
			break;
		case GL_DOUBLE_MAT2x3:
			s = sizeof(double) * 6;
			break;
		case GL_DOUBLE_MAT2x4:
			s = sizeof(double) * 8;
			break;
		case GL_DOUBLE_MAT3x2:
			s = sizeof(double) * 6;
			break;
		case GL_DOUBLE_MAT3x4:
			s = sizeof(double) * 12;
			break;
		case GL_DOUBLE_MAT4x2:
			s = sizeof(double) * 8;
			break;
		case GL_DOUBLE_MAT4x3:
			s = sizeof(double) * 12;
			break;
		default: return 0;
		}
		return s;
	}



	std::string ShaderProgram::_type_string(GLenum type) const {
		switch (type) {
		case GL_FLOAT:
			return "GL_FLOAT";
			break;
		case GL_FLOAT_VEC2:
			return "GL_FLOAT_VEC2";
			break;
		case GL_FLOAT_VEC3:
			return "GL_FLOAT_VEC3";
			break;
		case GL_FLOAT_VEC4:
			return "GL_FLOAT_VEC4";
			break;

			// Doubles
		case GL_DOUBLE:
			return "GL_DOUBLE";
			break;
		case GL_DOUBLE_VEC2:
			return "GL_DOUBLE_VEC2";
			break;
		case GL_DOUBLE_VEC3:
			return "GL_DOUBLE_VEC3";
			break;
		case GL_DOUBLE_VEC4:
			return "GL_DOUBLE_VEC4";
			break;

			// Samplers, Ints and Bools
		case GL_SAMPLER_1D:
			return "GL_SAMPLER_1D";
			break;
		case GL_SAMPLER_2D:
			return "GL_SAMPLER_2D";
			break;
		case GL_SAMPLER_3D:
			return "GL_SAMPLER_3D";
			break;
		case GL_SAMPLER_CUBE:
			return "GL_SAMPLER_CUBE";
			break;
		case GL_SAMPLER_1D_SHADOW:
			return "GL_SAMPLER_1D_SHADOW";
			break;
		case GL_SAMPLER_2D_SHADOW:
			return "GL_SAMPLER_2D_SHADOW";
			break;
		case GL_SAMPLER_1D_ARRAY:
			return "GL_SAMPLER_1D_ARRAY";
			break;
		case GL_SAMPLER_2D_ARRAY:
			return "GL_SAMPLER_2D_ARRAY";
			break;
		case GL_SAMPLER_1D_ARRAY_SHADOW:
			return "GL_SAMPLER_1D_ARRAY_SHADOW";
			break;
		case GL_SAMPLER_2D_ARRAY_SHADOW:
			return "GL_SAMPLER_2D_ARRAY_SHADOW";
			break;
		case GL_SAMPLER_2D_MULTISAMPLE:
			return "GL_SAMPLER_2D_MULTISAMPLE";
			break;
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
			return "GL_SAMPLER_2D_MULTISAMPLE_ARRAY";
			break;
		case GL_SAMPLER_CUBE_SHADOW:
			return "GL_SAMPLER_CUBE_SHADOW";
			break;
		case GL_SAMPLER_BUFFER:
			return "GL_SAMPLER_BUFFER";
			break;
		case GL_SAMPLER_2D_RECT:
			return "GL_SAMPLER_2D_RECT";
			break;
		case GL_SAMPLER_2D_RECT_SHADOW:
			return "GL_SAMPLER_2D_RECT_SHADOW";
			break;
		case GL_INT_SAMPLER_1D:
			return "GL_INT_SAMPLER_1D";
			break;
		case GL_INT_SAMPLER_2D:
			return "GL_INT_SAMPLER_2D";
			break;
		case GL_INT_SAMPLER_3D:
			return "GL_INT_SAMPLER_3D";
			break;
		case GL_INT_SAMPLER_CUBE:
			return "GL_INT_SAMPLER_CUBE";
			break;
		case GL_INT_SAMPLER_1D_ARRAY:
			return "GL_INT_SAMPLER_1D_ARRAY";
			break;
		case GL_INT_SAMPLER_2D_ARRAY:
			return "GL_INT_SAMPLER_2D_ARRAY";
			break;
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
			return "GL_INT_SAMPLER_2D_MULTISAMPLE";
			break;
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			return "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
			break;
		case GL_INT_SAMPLER_BUFFER:
			return "GL_INT_SAMPLER_BUFFER";
			break;
		case GL_INT_SAMPLER_2D_RECT:
			return "GL_INT_SAMPLER_2D_RECT";
			break;
		case GL_UNSIGNED_INT_SAMPLER_1D:
			return "GL_UNSIGNED_INT_SAMPLER_1D";
			break;
		case GL_UNSIGNED_INT_SAMPLER_2D:
			return "GL_UNSIGNED_INT_SAMPLER_2D";
			break;
		case GL_UNSIGNED_INT_SAMPLER_3D:
			return "GL_UNSIGNED_INT_SAMPLER_3D";
			break;
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
			return "GL_UNSIGNED_INT_SAMPLER_CUBE";
			break;
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
			return "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";
			break;
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			return "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
			break;
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
			break;
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			return "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
			break;
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
			return "GL_UNSIGNED_INT_SAMPLER_BUFFER";
			break;
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
			return "GL_UNSIGNED_INT_SAMPLER_2D_RECT";
			break;
		case GL_BOOL:
			return "GL_BOOL";
			break;
		case GL_INT:
			return "GL_INT";
			break;
		case GL_BOOL_VEC2:
			return "GL_BOOL_VEC2";
			break;
		case GL_INT_VEC2:
			return "GL_INT_VEC2";
			break;
		case GL_BOOL_VEC3:
			return "GL_BOOL_VEC3";
			break;
		case GL_INT_VEC3:
			return "GL_INT_VEC3";
			break;
		case GL_BOOL_VEC4:
			return "GL_BOOL_VEC4";
			break;
		case GL_INT_VEC4:
			return "GL_INT_VEC4";
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			return "GL_UNSIGNED_INT";
			break;
		case GL_UNSIGNED_INT_VEC2:
			return "GL_UNSIGNED_INT_VEC2";
			break;
		case GL_UNSIGNED_INT_VEC3:
			return "GL_UNSIGNED_INT_VEC3";
			break;
		case GL_UNSIGNED_INT_VEC4:
			return "GL_UNSIGNED_INT_VEC4";
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			return "GL_FLOAT_MAT2";
			break;
		case GL_FLOAT_MAT3:
			return "GL_FLOAT_MAT3";
			break;
		case GL_FLOAT_MAT4:
			return "GL_FLOAT_MAT4";
			break;
		case GL_FLOAT_MAT2x3:
			return "GL_FLOAT_MAT2x3";
			break;
		case GL_FLOAT_MAT2x4:
			return "GL_FLOAT_MAT2x4";
			break;
		case GL_FLOAT_MAT3x2:
			return "GL_FLOAT_MAT3x2";
			break;
		case GL_FLOAT_MAT3x4:
			return "GL_FLOAT_MAT3x4";
			break;
		case GL_FLOAT_MAT4x2:
			return "GL_FLOAT_MAT4x2";
			break;
		case GL_FLOAT_MAT4x3:
			return "GL_FLOAT_MAT4x3";
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			return "GL_DOUBLE_MAT2";
			break;
		case GL_DOUBLE_MAT3:
			return "GL_DOUBLE_MAT3";
			break;
		case GL_DOUBLE_MAT4:
			return "GL_DOUBLE_MAT4";
			break;
		case GL_DOUBLE_MAT2x3:
			return "GL_DOUBLE_MAT2x3";
			break;
		case GL_DOUBLE_MAT2x4:
			return "GL_DOUBLE_MAT2x4";
			break;
		case GL_DOUBLE_MAT3x2:
			return "GL_DOUBLE_MAT3x2";
			break;
		case GL_DOUBLE_MAT3x4:
			return "GL_DOUBLE_MAT3x4";
			break;
		case GL_DOUBLE_MAT4x2:
			return "GL_DOUBLE_MAT4x2";
			break;
		case GL_DOUBLE_MAT4x3:
			return "GL_DOUBLE_MAT4x3";
			break;
		default: return "unknown type";
		}
		return "unknown type";
	}


	bool ShaderProgram::load_binary(const std::string& file_name) {
        if (!OpenglUtil::is_supported("GL_ARB_get_program_binary")) {
            LOG(ERROR) << "load binary program requires OpenGL >= 4.1. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return false;
        }

		std::string code = _read_linked_program(file_name);
		if (code.empty()) {
			LOG(ERROR) << "empty program in file: " << file_name;
			return false;
		}

		if (program_)
			clear();
		program_ = glCreateProgram();
		if (!program_) {
			LOG(ERROR) << "failed create program from file: " << file_name;
			return false;
		}

		const char* bindata = &code[0];
		glProgramBinary(program_, *(GLenum*)bindata, bindata + 4, GLsizei(code.size() - 4));
		
		// check the program linked or not
		std::string log;
		if (program_info_log(log)) {
			_add_uniforms();	
			_add_blocks();		

#ifndef NDEBUG
            // print some hints for user
            if (OpenglUtil::is_supported("GL_ARB_program_interface_query")) {
                LOG(INFO) << "--------------- shader program: " << name() << " ---------------";
                print_active_attributes();
                print_active_uniforms();
                print_active_uniform_blocks();
            }
#endif
			return true;
		}
		else {
			LOG(ERROR) << "load program failed: " << log;
			clear(); // we don't need the program anymore; Also don't leak shaders either.
			return false;
		}
	}


    bool ShaderProgram::save_binary(const std::string& file_name) {
        if (!OpenglUtil::is_supported("GL_ARB_get_program_binary")) {
            LOG(ERROR) << "save binary program requires OpenGL >= 4.1. " << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return false;
        }

		// check the program linked or not
		std::string log;
		if (!program_info_log(log)) {
            LOG(ERROR) << "program not linked yet." <<
                         (!log.empty() ? " " + log  : "") << (name_.empty() ? "" : " - \'" +  name_ + "\'");
            return false;
		}

        int datasize;
        int datasize2;
		glGetProgramiv(program_, GL_PROGRAM_BINARY_LENGTH, &datasize);

		std::string binraw;
		binraw.resize(datasize + 4);
		char* bindata = &binraw[0];
		glGetProgramBinary(program_, datasize, &datasize2, (GLenum*)bindata, bindata + 4);

		std::ofstream stream;
		stream.open(file_name.c_str(), std::ios::binary | std::ios::out);
		if (stream.is_open()) {
			stream.write(bindata, datasize + 4);
            return true;
		}
        return false;
	}

}
