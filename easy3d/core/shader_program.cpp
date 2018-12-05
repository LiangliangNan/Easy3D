
#include "shader_program.h"
#include "opengl_info.h"
#include "opengl_error.h"

#include <cassert>
#include <iostream>
#include <fstream>


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

	std::map<std::string, ShaderProgram::UniformBlock> ShaderProgram::spBlocks;

	int ShaderProgram::spBlockCount = 1;

	bool ShaderProgram::is_supported_ = false;
	bool ShaderProgram::initialized_ = false;


	ShaderProgram::ShaderProgram(const std::string& name)
		: name_(name)
		, program_(0)
	{
		if (is_supported()) {
			for (int i = 0; i < ShaderProgram::NUM_SHADER_TYPES; ++i) {
				pShader[i] = 0;
			}

			// Liangliang: it is always a bad idea to initialize some OpenGL stuff in the constructors
			//			   because the OpenGL context may not exist or the current context is not the 
			//			   correct one. It is always correct to "create when needed".
			// program_ = glCreateProgram();
		}
	}


	ShaderProgram::~ShaderProgram() {
		clear();
	}


	void ShaderProgram::clear() {
		if (is_bound())
			unbind();	mpl_debug_gl_error;

		for (int i = 0; i < ShaderProgram::NUM_SHADER_TYPES; ++i) {
			if (pShader[i]) {
				// Only shaders that are not attached can be effectively deleted.
				glDetachShader(program_, pShader[i]);	mpl_debug_gl_error;

				// In the case of a shader that is still attached to some (one or more) 
				// programs, the shader is not actually deleted, but only marked for 
				// deletion. The delete operation will only be concluded when the shader 
				// is no longer attached to any program, i.e. it has been detached from 
				// all programs it was attached to.
				glDeleteShader(pShader[i]);	mpl_debug_gl_error;
			}
		}

		if (glIsProgram(program_))
			glDeleteProgram(program_);	mpl_debug_gl_error;

		pUniforms.clear();
	}

	bool ShaderProgram::is_supported() {
		if (!initialized_) {
			bool multi_texture =
				OpenglInfo::is_supported("GL_VERSION_1_3") ||
				OpenglInfo::is_supported("GL_ARB_multitexture");

			bool glsl = OpenglInfo::is_supported("GL_VERSION_2_0") || (
				OpenglInfo::is_supported("GL_ARB_shader_objects") &&
				OpenglInfo::is_supported("GL_ARB_shading_language_100") &&
				OpenglInfo::is_supported("GL_ARB_vertex_shader") &&
				OpenglInfo::is_supported("GL_ARB_fragment_shader")
				);

			is_supported_ = multi_texture && glsl;
			if (!is_supported_)
				std::cerr << "Shader program not supported" << std::endl;
			initialized_ = true;
		}
		return is_supported_;
	}


	std::string ShaderProgram::_read_file(const std::string& file_name) {
		std::string code;

#if 0 // line by line
		std::ifstream in(file_name.c_str());
		if (in.fail()) {
			std::cerr "could not open file \'" << file_name << "\'" << std::endl;
			return false;
		}

		std::string line;
		while (in.good()) {
			getline(in, line);
			code += line + "\n";
		}
#else 
		//must read files as binary to prevent problems from newline translation
		std::ifstream in(file_name.c_str(), std::ios::binary);
		if (in.fail()) {
			std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
			return code;
		}

		in.seekg(0, std::ios::end);
		std::size_t length = in.tellg();
		code.resize(length);
		in.seekg(0, std::ios::beg);

		in.read(&(code[0]), length);
#endif

		in.close();

		return code;
	}


	bool ShaderProgram::load_shader_from_file(ShaderType type, const std::string& file_name) {
		std::string code = _read_file(file_name);
		if (code.empty()) {
			std::cerr << "failed reading shader file \'" << file_name << "\'" << std::endl;
			return false;
		}
		return load_shader_from_code(type, code);
	}


	bool ShaderProgram::load_shader_from_code(ShaderType type, const std::string& str) {
		if (str.empty()) {
			std::cerr << "NULL code for " << spStringShaderTypes[type] << std::endl;
			return false;
		}
		GLuint shader = glCreateShader(spGLShaderTypes[type]);	mpl_debug_gl_error;
		if (!glIsShader(shader)) {
			mpl_debug_gl_error;
			std::cerr << "failed creating shader" << std::endl;
			return false;
		}

		const char* code = str.c_str();
		glShaderSource(shader, 1, &code, NULL);	mpl_debug_gl_error;
		glCompileShader(shader);	mpl_debug_gl_error;

		//we must check the compilation result
		std::string log;
		if (!shader_info_log(log, shader)) {
			std::cerr << log << std::endl;
			glDeleteShader(shader);	mpl_debug_gl_error;
			return false;
		}

		if (program_ == 0) {
			program_ = glCreateProgram();	mpl_debug_gl_error;
		}

		glAttachShader(program_, shader);	mpl_debug_gl_error;
		mpl_debug_gl_error;	mpl_debug_gl_error;
		pShader[type] = shader;
		return true;
	}


	bool ShaderProgram::link_program() {
		glLinkProgram(program_);	mpl_debug_gl_error;

		//we must check the linkage result
		std::string log;
		if (!program_info_log(log)) {
			mpl_debug_gl_error;
			std::cerr << log << std::endl;
			clear(); // we don't need the program anymore; Also don't leak shaders either.
			return false;
		}

		_add_uniforms();	mpl_debug_gl_error;
		_add_blocks();		mpl_debug_gl_error;

#ifndef NDEBUG
        // print some hints for user
        if (OpenglInfo::is_supported("GL_ARB_program_interface_query")) {
            std::cout << "--------------- shader program: " << name() << " ---------------" << std::endl;
            print_active_attributes();
            print_active_uniforms();
            print_active_uniform_blocks();
        }
#endif

		return true;
	}


	/// returns a string with a shader's infolog
	bool ShaderProgram::shader_info_log(std::string& log, GLuint shader) {
		log.clear();
		int compile_status = GL_TRUE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);	mpl_debug_gl_error;
		if (compile_status != GL_TRUE) {
			int log_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);	mpl_debug_gl_error;
			log.append(log_length + 1, '\0');

			glGetShaderInfoLog(shader, log_length, 0, &log[0]);	mpl_debug_gl_error;
			return false;
		}
		else
			return true;
	}


	/// returns a string with the program's infolog
	bool ShaderProgram::program_info_log(std::string& log) {
		log.clear();
		int link_status = GL_TRUE;
		glGetProgramiv(program_, GL_LINK_STATUS, &link_status);	mpl_debug_gl_error;
		if (link_status != GL_TRUE) {
			int log_length = 0;
			glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);	mpl_debug_gl_error;
			log.append(log_length + 1, '\0');
			glGetProgramInfoLog(program_, log_length, 0, &log[0]);	mpl_debug_gl_error;
			return false;
		}
		else
			return true;
	}


	void ShaderProgram::set_program_output(int index, const std::string& name) {
		glBindFragDataLocation(program_, index, name.c_str());	mpl_debug_gl_error;
	}


	GLint ShaderProgram::program_output(const std::string& name) {
		return glGetFragDataLocation(program_, name.c_str());	mpl_debug_gl_error;
	}


	void ShaderProgram::set_attrib_name(ShaderProgram::AttribType at, const std::string& name) {
		glBindAttribLocation(program_, at, name.c_str());	mpl_debug_gl_error;
	}

	void ShaderProgram::set_attrib_name(const ShaderProgram::Attribute& attribute) {
		set_attrib_name(attribute.first, attribute.second);
	}

	void ShaderProgram::set_attrib_names(const std::vector<ShaderProgram::Attribute>& attributes) {
		for (std::size_t i = 0; i < attributes.size(); ++i) {
			set_attrib_name(attributes[i]);
		}
	}

	int ShaderProgram::get_attribute_location(const std::string& name) const {
		int loc = glGetAttribLocation(program_, name.c_str());	mpl_debug_gl_error;
		return loc;
	}


	GLuint ShaderProgram::get_program() const {
		return program_;
	}


	GLuint ShaderProgram::get_shader(ShaderType aType) const {
		return pShader[aType];
	}


	void ShaderProgram::set_block(const std::string& name, const void *value) {
		if (spBlocks.count(name) == 0) {
			std::cerr << "warning: block " << name << " does not exist or is not active" << std::endl;
			return;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, spBlocks[name].buffer);	mpl_debug_gl_error;
		glBufferSubData(GL_UNIFORM_BUFFER, 0, spBlocks[name].size, value);	mpl_debug_gl_error;
		glBindBuffer(GL_UNIFORM_BUFFER, 0);	mpl_debug_gl_error;
	}


	void ShaderProgram::set_block_uniform(const std::string& blockName, const std::string& uniformName, const void *value) {
		//assert(spBlocks.count(blockName) && spBlocks[blockName].uniformOffsets.count(uniformName));
		if (spBlocks.count(blockName) == 0) {
			std::cerr << "warning: block " << blockName << " does not exist or is not active" << std::endl;
			return;
		}

		UniformBlock b = spBlocks[blockName];
		if (b.uniformOffsets.count(uniformName) == 0) {
			std::cerr << "warning: block/uniform " << blockName << "/" << uniformName << " does not exist or is not active" << std::endl;
			return;
		}

		BlockUniform bUni = b.uniformOffsets[uniformName];
		glBindBuffer(GL_UNIFORM_BUFFER, b.buffer);	mpl_debug_gl_error;
		glBufferSubData(GL_UNIFORM_BUFFER, bUni.offset, bUni.size, value);	mpl_debug_gl_error;
		glBindBuffer(GL_UNIFORM_BUFFER, 0);	mpl_debug_gl_error;
	}


	void ShaderProgram::set_block_uniform_array_element(const std::string& blockName, const std::string& uniformName, int arrayIndex, const void * value) {
		//assert(spBlocks.count(blockName) && spBlocks[blockName].uniformOffsets.count(uniformName));
		if (spBlocks.count(blockName) == 0) {
			std::cerr << "warning: block " << blockName << " does not exist or is not active" << std::endl;
			return;
		}

		UniformBlock b = spBlocks[blockName];
		if (b.uniformOffsets.count(uniformName) == 0) {
			std::cerr << "warning: block/uniform " << blockName << "/" << uniformName << " does not exist or is not active" << std::endl;
			return;
		}

		BlockUniform bUni = b.uniformOffsets[uniformName];
		glBindBuffer(GL_UNIFORM_BUFFER, b.buffer);	mpl_debug_gl_error;
		glBufferSubData(GL_UNIFORM_BUFFER, bUni.offset + bUni.arrayStride * arrayIndex, bUni.arrayStride, value);	mpl_debug_gl_error;
		glBindBuffer(GL_UNIFORM_BUFFER, 0);	mpl_debug_gl_error;
	}


	void ShaderProgram::set_uniform(const std::string& name, int value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
			std::cerr << "warning: uniform " << name << " does not exist or is not active" << std::endl;
			return;
		}

		Uniform u = pUniforms[name];
		glProgramUniform1i(program_, u.location, value);	mpl_debug_gl_error;
	}


	void ShaderProgram::set_uniform(const std::string& name, unsigned int value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
			std::cerr << "warning: uniform " << name << " does not exist or is not active" << std::endl;
			return;
		}

		Uniform u = pUniforms[name];
		glProgramUniform1ui(program_, u.location, value);	mpl_debug_gl_error;
	}


	void ShaderProgram::set_uniform(const std::string& name, float value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
			std::cerr << "warning: uniform " << name << " does not exist or is not active" << std::endl;
			return;
		}

		Uniform u = pUniforms[name];
		glProgramUniform1f(program_, u.location, value);	mpl_debug_gl_error;
	}


	void ShaderProgram::set_uniform(const std::string& name, const void *value) {
		//	assert(pUniforms.count(name) != 0);
		if (pUniforms.count(name) == 0) {
			std::cerr << "warning: uniform " << name << " does not exist or is not active" << std::endl;
			return;
		}

		Uniform u = pUniforms[name];

		switch (u.type) {

			// Floats
		case GL_FLOAT:
			glProgramUniform1fv(program_, u.location, u.size, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_VEC2:
			glProgramUniform2fv(program_, u.location, u.size, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_VEC3:
			glProgramUniform3fv(program_, u.location, u.size, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_VEC4:
			glProgramUniform4fv(program_, u.location, u.size, (const GLfloat *)value);	mpl_debug_gl_error;
			break;

			// Doubles
		case GL_DOUBLE:
			glProgramUniform1dv(program_, u.location, u.size, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_VEC2:
			glProgramUniform2dv(program_, u.location, u.size, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_VEC3:
			glProgramUniform3dv(program_, u.location, u.size, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_VEC4:
			glProgramUniform4dv(program_, u.location, u.size, (const GLdouble *)value);	mpl_debug_gl_error;
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
			glProgramUniform1iv(program_, u.location, u.size, (const GLint *)value);	mpl_debug_gl_error;
			break;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			glProgramUniform2iv(program_, u.location, u.size, (const GLint *)value);	mpl_debug_gl_error;
			break;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			glProgramUniform3iv(program_, u.location, u.size, (const GLint *)value);	mpl_debug_gl_error;
			break;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			glProgramUniform4iv(program_, u.location, u.size, (const GLint *)value);	mpl_debug_gl_error;
			break;

			// Unsigned ints
		case GL_UNSIGNED_INT:
			glProgramUniform1uiv(program_, u.location, u.size, (const GLuint *)value);	mpl_debug_gl_error;
			break;
		case GL_UNSIGNED_INT_VEC2:
			glProgramUniform2uiv(program_, u.location, u.size, (const GLuint *)value);	mpl_debug_gl_error;
			break;
		case GL_UNSIGNED_INT_VEC3:
			glProgramUniform3uiv(program_, u.location, u.size, (const GLuint *)value);	mpl_debug_gl_error;
			break;
		case GL_UNSIGNED_INT_VEC4:
			glProgramUniform4uiv(program_, u.location, u.size, (const GLuint *)value);	mpl_debug_gl_error;
			break;

			// Float Matrices
		case GL_FLOAT_MAT2:
			glProgramUniformMatrix2fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT3:
			glProgramUniformMatrix3fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT4:
			glProgramUniformMatrix4fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT2x3:
			glProgramUniformMatrix2x3fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT2x4:
			glProgramUniformMatrix2x4fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT3x2:
			glProgramUniformMatrix3x2fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT3x4:
			glProgramUniformMatrix3x4fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT4x2:
			glProgramUniformMatrix4x2fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;
		case GL_FLOAT_MAT4x3:
			glProgramUniformMatrix4x3fv(program_, u.location, u.size, false, (const GLfloat *)value);	mpl_debug_gl_error;
			break;

			// Double Matrices
		case GL_DOUBLE_MAT2:
			glProgramUniformMatrix2dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT3:
			glProgramUniformMatrix3dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT4:
			glProgramUniformMatrix4dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT2x3:
			glProgramUniformMatrix2x3dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT2x4:
			glProgramUniformMatrix2x4dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT3x2:
			glProgramUniformMatrix3x2dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT3x4:
			glProgramUniformMatrix3x4dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT4x2:
			glProgramUniformMatrix4x2dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		case GL_DOUBLE_MAT4x3:
			glProgramUniformMatrix4x3dv(program_, u.location, u.size, false, (const GLdouble *)value);	mpl_debug_gl_error;
			break;
		}
	}


	bool ShaderProgram::is_program_valid() {
		GLint b = GL_FALSE;
		if (program_) {
			glValidateProgram(program_);		mpl_debug_gl_error;
			glGetProgramiv(program_, GL_VALIDATE_STATUS, &b);	mpl_debug_gl_error;
		}
		return (b != GL_FALSE);
	}


	bool ShaderProgram::is_shader_compiled(ShaderType aType) {
		GLint b = GL_FALSE;
		if (pShader[aType]) {
			glGetShaderiv(pShader[aType], GL_COMPILE_STATUS, &b);	mpl_debug_gl_error;
		}
		return (b != GL_FALSE);
	}


	bool ShaderProgram::is_bound() const {
		GLint current_program = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);	mpl_debug_gl_error;
		return (current_program == (GLint)program_);
	}


	bool ShaderProgram::is_program_linked() {
		GLint b = GL_FALSE;
		if (program_) {
			glGetProgramiv(program_, GL_LINK_STATUS, &b);	mpl_debug_gl_error;
		}
		return (b != GL_FALSE);
	}


	void ShaderProgram::bind() {
		glUseProgram(program_);	mpl_debug_gl_error;
	}


	void ShaderProgram::unbind() {
		glUseProgram(0);		mpl_debug_gl_error;
	}


	void ShaderProgram::bind_texture(const std::string& name, GLuint texture, GLint unit, GLenum target /* = GL_TEXTURE_2D*/) {
		glActiveTexture(GL_TEXTURE0 + unit);	mpl_debug_gl_error;
		glBindTexture(target, texture);			mpl_debug_gl_error;
		set_uniform(name, unit);				mpl_debug_gl_error;
	}


	void ShaderProgram::unbind_texture(GLenum target /* = GL_TEXTURE_2D*/) {
		glActiveTexture(GL_TEXTURE0);			mpl_debug_gl_error;
		glBindTexture(target, 0);				mpl_debug_gl_error;
	}


	bool ShaderProgram::is_attribute_used(const std::string& name) {
		if (name.empty()) {
			std::cerr << "empty attribute name" << std::endl;
			return false;
		}

		if (!is_program_linked()) {
			std::cerr << "program not linked" << std::endl;
			return false;
		}

		GLint loc = glGetAttribLocation(program_, name.c_str());	mpl_debug_gl_error;
		return (loc != -1);
	}


	bool ShaderProgram::is_uniform_used(const std::string& name) {
		if (name.empty()) {
			std::cerr << "empty uniform name" << std::endl;
			return false;
		}

		if (!is_program_linked()) {
			std::cerr << "program not linked" << std::endl;
			return false;
		}

		GLint loc = glGetUniformLocation(program_, name.c_str());	mpl_debug_gl_error;
		return (loc != -1);
	}


	void ShaderProgram::print_active_attributes() {
		if (!is_program_linked()) {
			std::cerr << "program not linked" << std::endl;
			return;
		}

        if (!OpenglInfo::is_supported("GL_ARB_program_interface_query")) {
            std::cerr << "querying active attributes requires OpenGL >= 4.3" << std::endl;
            return;
        }

		GLint numAttribs;
		glGetProgramInterfaceiv(program_, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

		GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

		std::cout << "Active attributes:" << std::endl;
		for (int i = 0; i < numAttribs; ++i) {
			GLint results[3];
			glGetProgramResourceiv(program_, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);	mpl_debug_gl_error;

			GLint nameBufSize = results[0] + 1;
			char * name = new char[nameBufSize];
			glGetProgramResourceName(program_, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name);	mpl_debug_gl_error;
			std::cout << "   " << results[2] << "\t" << name << "\t" << _type_string(results[1]) << std::endl;
			delete[] name;
		}
	}


	void ShaderProgram::print_active_uniforms() {
		if (!is_program_linked()) {
			std::cerr << "program not linked" << std::endl;
			return;
		}

        if (!OpenglInfo::is_supported("GL_ARB_program_interface_query")) {
            std::cerr << "querying active uniforms requires OpenGL >= 4.3" << std::endl;
            return;
        }

		GLint numUniforms = 0;
		glGetProgramInterfaceiv(program_, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);	mpl_debug_gl_error;

		GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

		std::cout << "Active uniforms:" << std::endl;
		for (int i = 0; i < numUniforms; ++i) {
			GLint results[4];
			glGetProgramResourceiv(program_, GL_UNIFORM, i, 4, properties, 4, NULL, results);	mpl_debug_gl_error;
			if (results[3] != -1)
				continue;  // Skip uniforms in blocks 
			GLint nameBufSize = results[0] + 1;
			char * name = new char[nameBufSize];
			glGetProgramResourceName(program_, GL_UNIFORM, i, nameBufSize, NULL, name);	mpl_debug_gl_error;
			std::cout << "   " << results[2] << "\t" << name << "\t" << _type_string(results[1]) << "" << std::endl;
			delete[] name;
		}
	}


	void ShaderProgram::print_active_uniform_blocks() {
		if (!is_program_linked()) {
			std::cerr << "program not linked" << std::endl;
			return;
		}

        if (!OpenglInfo::is_supported("GL_ARB_program_interface_query")) {
            std::cerr << "querying active uniform blocks requires OpenGL >= 4.3" << std::endl;
            return;
        }

		GLint numBlocks = 0;

		glGetProgramInterfaceiv(program_, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);	mpl_debug_gl_error;
		GLenum blockProps[] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH };
		GLenum blockIndex[] = { GL_ACTIVE_VARIABLES };
		GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX };

		for (int block = 0; block < numBlocks; ++block) {
			GLint blockInfo[2];
			glGetProgramResourceiv(program_, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, NULL, blockInfo);	mpl_debug_gl_error;
			GLint numUnis = blockInfo[0];

			char * blockName = new char[blockInfo[1] + 1];
			glGetProgramResourceName(program_, GL_UNIFORM_BLOCK, block, blockInfo[1] + 1, NULL, blockName);	mpl_debug_gl_error;
			std::cout << "Uniform block \"" << blockName << "\"\t" << std::endl;
			delete[] blockName;

			GLint * unifIndexes = new GLint[numUnis];
			glGetProgramResourceiv(program_, GL_UNIFORM_BLOCK, block, 1, blockIndex, numUnis, NULL, unifIndexes);	mpl_debug_gl_error;

			for (int unif = 0; unif < numUnis; ++unif) {
				GLint uniIndex = unifIndexes[unif];
				GLint results[3];
				glGetProgramResourceiv(program_, GL_UNIFORM, uniIndex, 3, props, 3, NULL, results);	mpl_debug_gl_error;

				GLint nameBufSize = results[0] + 1;
				char * name = new char[nameBufSize];
				glGetProgramResourceName(program_, GL_UNIFORM, uniIndex, nameBufSize, NULL, name);	mpl_debug_gl_error;
				std::cout << "   " << results[2] << "\t" << name << "\t" << _type_string(results[1]) << "" << std::endl;
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

		glGetProgramiv(program_, GL_ACTIVE_UNIFORM_BLOCKS, &count);	mpl_debug_gl_error;

		for (int i = 0; i < count; ++i) {
			// Get buffers name
			glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &actualLen);	mpl_debug_gl_error;
			name = (char *)malloc(sizeof(char) * actualLen);
			glGetActiveUniformBlockName(program_, i, actualLen, NULL, name);	mpl_debug_gl_error;

			if (!spBlocks.count(name)) {
				// Get buffers size
				block = spBlocks[name];
				glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);	mpl_debug_gl_error;
				//printf("DataSize:%d\n", dataSize);
				glGenBuffers(1, &block.buffer);	mpl_debug_gl_error;
				glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);	mpl_debug_gl_error;
				glBufferData(GL_UNIFORM_BUFFER, dataSize, NULL, GL_DYNAMIC_DRAW);	mpl_debug_gl_error;
				glUniformBlockBinding(program_, i, spBlockCount);	mpl_debug_gl_error;
				glBindBufferRange(GL_UNIFORM_BUFFER, spBlockCount, block.buffer, 0, dataSize);	mpl_debug_gl_error;

				glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUnif);	mpl_debug_gl_error;

				unsigned int *indices;
				indices = (unsigned int *)malloc(sizeof(unsigned int) * activeUnif);
				glGetActiveUniformBlockiv(program_, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (int *)indices);	mpl_debug_gl_error;

				glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);	mpl_debug_gl_error;
				name2 = (char *)malloc(sizeof(char) * maxUniLength);

				for (int k = 0; k < activeUnif; ++k) {

					BlockUniform bUni;

					glGetActiveUniformName(program_, indices[k], maxUniLength, &actualLen, name2);	mpl_debug_gl_error;
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_TYPE, &uniType);	mpl_debug_gl_error;
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_SIZE, &uniSize);	mpl_debug_gl_error;
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_OFFSET, &uniOffset);	mpl_debug_gl_error;
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_MATRIX_STRIDE, &uniMatStride);	mpl_debug_gl_error;
					glGetActiveUniformsiv(program_, 1, &indices[k], GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);	mpl_debug_gl_error;

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
						auxSize = _type_size(uniType);

					bUni.offset = uniOffset;
					bUni.type = uniType;
					bUni.size = auxSize;
					bUni.arrayStride = uniArrayStride;

					block.uniformOffsets[name2] = bUni;
				}
				free(name2);

				block.size = dataSize;
				block.bindingIndex = spBlockCount;
				spBlocks[name] = block;
				spBlockCount++;
			}
			else
				glUniformBlockBinding(program_, i, spBlocks[name].bindingIndex);	mpl_debug_gl_error;
		}
	}


	void  ShaderProgram::_add_uniforms() {
		int count;
		GLsizei actualLen;
		GLint size;
		GLint uniArrayStride;
		GLenum type;
		char *name;

		int maxUniLength;
		glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);	mpl_debug_gl_error;

		glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniLength);	mpl_debug_gl_error;

		name = (char *)malloc(sizeof(char) * maxUniLength);

		unsigned int loc;
		for (int i = 0; i < count; ++i) {
			glGetActiveUniform(program_, i, maxUniLength, &actualLen, &size, &type, name);	mpl_debug_gl_error;
			// -1 indicates that is not an active uniform, although it may be present in a uniform block
			loc = glGetUniformLocation(program_, name);	mpl_debug_gl_error;
			if (loc != -1) {
				glGetActiveUniformsiv(program_, 1, (GLuint*)&i, GL_UNIFORM_ARRAY_STRIDE, &uniArrayStride);	mpl_debug_gl_error;
				_add_uniform(name, type, size);
			}
		}
		free(name);
	}


	void ShaderProgram::_add_uniform(const std::string& name, GLenum type, unsigned int size) {
		Uniform u;
		u.type = type;
		u.location = glGetUniformLocation(program_, name.c_str());	mpl_debug_gl_error;
		u.size = size;
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
        if (!OpenglInfo::is_supported("GL_ARB_get_program_binary")) {
            std::cerr << "load binary program requires OpenGL >= 4.1" << std::endl;
            return false;
        }

		std::string code = _read_file(file_name);
		if (code.empty()) {
			std::cerr << "empty program in file: " << file_name << std::endl;
			return false;
		}

		if (program_)
			clear();
		program_ = glCreateProgram();
		if (!program_) {
			std::cerr << "failed create program" << std::endl;
			return false;
		}

		const char* bindata = &code[0];
		glProgramBinary(program_, *(GLenum*)bindata, bindata + 4, GLsizei(code.size() - 4));
		
		// check the program linked or not
		std::string log;
		if (program_info_log(log)) {
			_add_uniforms();	mpl_debug_gl_error;
			_add_blocks();		mpl_debug_gl_error;
#ifndef NDEBUG
            // print some hints for user
            if (OpenglInfo::is_supported("GL_ARB_program_interface_query")) {
                std::cout << "--------------- shader program: " << name() << " ---------------" << std::endl;
                print_active_attributes();
                print_active_uniforms();
                print_active_uniform_blocks();
            }
#endif
			return true;
		}
		else {
			std::cerr << "load program failed: " << log << std::endl;
			clear(); // we don't need the program anymore; Also don't leak shaders either.
			return false;
		}
	}


    bool ShaderProgram::save_binary(const std::string& file_name) {
        if (!OpenglInfo::is_supported("GL_ARB_get_program_binary")) {
            std::cerr << "save binary program requires OpenGL >= 4.1" << std::endl;
            return false;
        }

		// check the program linked or not
		std::string log;
		if (!program_info_log(log)) {
            std::cerr << "program not linked yet." <<
                         (!log.empty() ? " " + log  : "") << std::endl;
            return false;
		}

		GLint datasize;
		GLint datasize2;
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
