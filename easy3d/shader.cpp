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


#include <easy3d/shader.h>

#include <cassert>
#include <iostream>
#include <fstream>

#include <3rd_party/glew/include/GL/glew.h>
#include <easy3d/opengl_error.h>


namespace easy3d {

	Shader::Shader(const std::string& name /* = "unknown" */)
		: name_(name)
		, prog_id_(0)
	{
	}

	Shader::~Shader()
	{
		cleanup();
	}


	void Shader::cleanup()
	{
		if (glIsProgram(prog_id_))
			glDeleteProgram(prog_id_);
		prog_id_ = 0;
	}


	void Shader::bind()
	{
		glUseProgram(prog_id_);
	}

	void Shader::unbind()
	{
		glUseProgram(0);
	}

	bool Shader::load_from_base_name(const std::string& path, const std::string& base_name, bool has_geom /* = false */)
	{
		std::string vert_file = path + "/" + base_name + ".vert";
		std::string frag_file = path + "/" + base_name + ".frag";
		std::string geom_file;
		if (has_geom)
			geom_file = path + "/" + base_name + ".geom";

		return load_from_files(vert_file, frag_file, geom_file);
	}

	bool Shader::load_from_files(const std::string& vert_file, const std::string& frag_file, const std::string& geom_file /* = "" */)
	{
		std::string vert_code;
		if (!read_shader_file(vert_file, vert_code))
			return false;

		std::string frag_code;
		if (!read_shader_file(frag_file, frag_code))
			return false;

		std::string geom_code;
		if (!geom_file.empty()) {
			read_shader_file(geom_file, geom_code);
		}

		return load_from_codes(vert_code, frag_code, geom_code);
	}

	bool Shader::load_from_codes(const std::string& vert_code, const std::string& frag_code, const std::string& geom_code /* = "" */)
	{
		cleanup();

		//GL ids
		unsigned int vs = load_shader(GL_VERTEX_SHADER, vert_code);	easy3d_debug_gl_error;
		if (!glIsShader(vs)) {
			return false;
		}

		unsigned int ps = load_shader(GL_FRAGMENT_SHADER, frag_code);	easy3d_debug_gl_error;
		if (!glIsShader(ps)) {
			return false;
		}

		unsigned int gs = 0;
		if (!geom_code.empty()) {
			gs = load_shader(GL_GEOMETRY_SHADER, geom_code);	easy3d_debug_gl_error;
			if (!glIsShader(gs)) {
				return false;
			}
		}

		//we create an empty GL program
		prog_id_ = glCreateProgram();	easy3d_debug_gl_error;

		//we attach loaded shaders to it
		glAttachShader(prog_id_, vs);	easy3d_debug_gl_error;
		glAttachShader(prog_id_, ps);	easy3d_debug_gl_error;
		if (glIsShader(gs))
			glAttachShader(prog_id_, gs);
		easy3d_debug_gl_error;


		set_attrib_name(Shader::POSITION, "vtx_position");
		set_attrib_name(Shader::COLOR, "vtx_color");



		//we link them all together
		glLinkProgram(prog_id_);	easy3d_debug_gl_error;

		//we check for success
		int link_status = GL_TRUE;
		glGetProgramiv(prog_id_, GL_LINK_STATUS, &link_status);	easy3d_debug_gl_error;
		if (link_status != GL_TRUE)
		{
			int log_length = 0;
			glGetProgramiv(prog_id_, GL_INFO_LOG_LENGTH, &log_length);
			std::string str_log;
			str_log.append(log_length + 1, '\0');
			glGetProgramInfoLog(prog_id_, log_length, 0, &str_log[0]);
			std::cerr << "Failed to compile shader program: " << str_log << std::endl;

			cleanup();
		}
		easy3d_debug_gl_error;

		// even if program creation was successful, we don't need the shaders anymore
		glDeleteShader(vs);	easy3d_debug_gl_error;
		glDeleteShader(ps);	easy3d_debug_gl_error;
		if (glIsShader(gs))
			glDeleteShader(gs);	easy3d_debug_gl_error;

		add_uniforms();	easy3d_debug_gl_error;

		return true;
	}

	bool Shader::read_shader_file(const std::string& file_name, std::string& shader_code) {
		std::string line;
		std::ifstream in(file_name.c_str());
		if (in.fail()) {
			std::cerr << "could not open file:" << "\'" << file_name << "\'" << std::endl;
			return false;
		}

		shader_code.clear();
		while (in.good()) {
			getline(in, line);
			shader_code += line + "\n";
		}
		in.close();

		if (shader_code.empty()) {
			std::cerr << "failed reading shader file:" << "\'" << file_name << "\'" << std::endl;
			return false;
		}
		else
			return true;
	}

	unsigned int Shader::load_shader(unsigned int type, const std::string& shader_code)
	{
		if (shader_code.empty())
			return 0;

		//create shader
		unsigned int shader = glCreateShader(type);
		if (!glIsShader(shader)) {
			std::cerr << "Failed to create shader (type = " << type << ")" << std::endl;
			return 0;
		}

		const char* code = shader_code.c_str();
		glShaderSource(shader, 1, &code, NULL);

		glCompileShader(shader);

		//we must check compilation result
		int compile_status = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (compile_status != GL_TRUE)
		{
			int log_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
			std::string str_log;
			str_log.append(log_length + 1, '\0');

			glGetShaderInfoLog(shader, log_length, 0, &str_log[0]);
			std::cerr << "Failed to compile shader: " << str_log << std::endl;

			glDeleteShader(shader);
			shader = 0;
			return 0;
		}

		return shader;
	}


	void Shader::set_attrib_name(AttribType at, const std::string& name) {
		glBindAttribLocation(prog_id_, at, name.c_str());
	}




	void Shader::add_uniforms() {
		int count;
		GLsizei actualLen;
		GLint size;
		GLint uniArrayStride;
		GLenum type;

		int maxUniLength;
		glGetProgramiv(prog_id_, GL_ACTIVE_UNIFORMS, &count);

		glGetProgramiv(prog_id_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);

		char* name = (char *)malloc(sizeof(char) * maxUniLength);

		unsigned int loc;
		for (int i = 0; i < count; ++i) {
			glGetActiveUniform(prog_id_, i, maxUniLength, &actualLen, &size, &type, name);	
			// -1 indicates that is not an active uniform, although it may be present in a uniform block
			loc = glGetUniformLocation(prog_id_, name);	
			if (loc != -1) {
				glGetActiveUniformsiv(prog_id_, 1, (GLuint*)&i, GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);	
				add_uniform(name, type, size);
			}
		}
		free(name);
	}


	void Shader::add_uniform(const std::string& name, GLenum type, unsigned int size) {
		Uniform u;
		u.type = type;
		u.location = glGetUniformLocation(prog_id_, name.c_str());	
		u.size = size;
		uniforms_[name] = u;
	}


	void Shader::set_uniform(const std::string& name, int value) {
		int loc = glGetUniformLocation(prog_id_, name.c_str());
		if (loc == -1) {
			std::cout << "uniform \'" << name << "\' does not exist" << std::endl;
			return;
		}
		glProgramUniform1i(prog_id_, loc, value);
	}


	void Shader::set_uniform(const std::string& name, unsigned int value) {
		int loc = glGetUniformLocation(prog_id_, name.c_str());
		if (loc == -1) {
			std::cout << "uniform \'" << name << "\' does not exist" << std::endl;
			return;
		}
		glProgramUniform1ui(prog_id_, loc, value);
	}


	void Shader::set_uniform(const std::string& name, float value) {
		int loc = glGetUniformLocation(prog_id_, name.c_str());
		if (loc == -1) {
			std::cout << "uniform \'" << name << "\' does not exist" << std::endl;
			return;
		}
		glProgramUniform1f(prog_id_, loc, value);
	}


	void Shader::set_uniform(const std::string& name, const void *value) {
		int loc = glGetUniformLocation(prog_id_, name.c_str());
		if (loc == -1) {
			std::cout << "uniform \'" << name << "\' does not exist" << std::endl;
			return;
		}


		Uniform u = uniforms_[name];

		switch (u.type) {

			// Floats
		case GL_FLOAT:
			glProgramUniform1fv(prog_id_, loc, u.size, (const GLfloat *)value);	
			break;
		case GL_FLOAT_VEC2:
			glProgramUniform2fv(prog_id_, loc, u.size, (const GLfloat *)value);	
			break;
		case GL_FLOAT_VEC3:
			glProgramUniform3fv(prog_id_, loc, u.size, (const GLfloat *)value);	
			break;
		case GL_FLOAT_VEC4:
			glProgramUniform4fv(prog_id_, loc, u.size, (const GLfloat *)value);	
			break;

			// Doubles
		case GL_DOUBLE:
			glProgramUniform1dv(prog_id_, loc, u.size, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_VEC2:
			glProgramUniform2dv(prog_id_, loc, u.size, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_VEC3:
			glProgramUniform3dv(prog_id_, loc, u.size, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_VEC4:
			glProgramUniform4dv(prog_id_, loc, u.size, (const GLdouble *)value);	
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
			glProgramUniform1iv(prog_id_, loc, u.size, (const GLint *)value);	
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			glProgramUniform2iv(prog_id_, loc, u.size, (const GLint *)value);	
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			glProgramUniform3iv(prog_id_, loc, u.size, (const GLint *)value);	
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			glProgramUniform4iv(prog_id_, loc, u.size, (const GLint *)value);	
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			glProgramUniform1uiv(prog_id_, loc, u.size, (const GLuint *)value);	
			break;
		case GL_UNSIGNED_INT_VEC2:
			glProgramUniform2uiv(prog_id_, loc, u.size, (const GLuint *)value);	
			break;
		case GL_UNSIGNED_INT_VEC3:
			glProgramUniform3uiv(prog_id_, loc, u.size, (const GLuint *)value);	
			break;
		case GL_UNSIGNED_INT_VEC4:
			glProgramUniform4uiv(prog_id_, loc, u.size, (const GLuint *)value);	
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			glProgramUniformMatrix2fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT3:
			glProgramUniformMatrix3fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT4:
			glProgramUniformMatrix4fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT2x3:
			glProgramUniformMatrix2x3fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT2x4:
			glProgramUniformMatrix2x4fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT3x2:
			glProgramUniformMatrix3x2fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT3x4:
			glProgramUniformMatrix3x4fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT4x2:
			glProgramUniformMatrix4x2fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;
		case GL_FLOAT_MAT4x3:
			glProgramUniformMatrix4x3fv(prog_id_, loc, u.size, false, (const GLfloat *)value);	
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			glProgramUniformMatrix2dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT3:
			glProgramUniformMatrix3dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT4:
			glProgramUniformMatrix4dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT2x3:
			glProgramUniformMatrix2x3dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT2x4:
			glProgramUniformMatrix2x4dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT3x2:
			glProgramUniformMatrix3x2dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT3x4:
			glProgramUniformMatrix3x4dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT4x2:
			glProgramUniformMatrix4x2dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		case GL_DOUBLE_MAT4x3:
			glProgramUniformMatrix4x3dv(prog_id_, loc, u.size, false, (const GLdouble *)value);	
			break;
		}
	}



}