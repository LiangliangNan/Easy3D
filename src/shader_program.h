/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*
*	This file is part of Mapple: software for processing and rendering
*   meshes and point clouds.
*
*	Mapple is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Mapple is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


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

#ifndef _OPENGL_SHADER_PROGRAM_H_
#define _OPENGL_SHADER_PROGRAM_H_

#include <map>
#include <vector>
#include <string>

#include <GL/glew.h>  


// Liangliang: about Shader Compilation
// https://www.khronos.org/opengl/wiki/Shader_Compilation#Error_handling

class ShaderProgram
{
public:
	
	// Types of Vertex Attributes
	enum AttribType {
		POSITION,
		COLOR,
		NORMAL,
		TEXCOORD,
		ATTRIB_1, // if you need more
		ATTRIB_2,
		ATTRIB_3,
		ATTRIB_4
	};
	typedef std::pair<ShaderProgram::AttribType, std::string> Attribute;

	// Types of Shaders
	enum ShaderType {
		VERTEX,
		FRAGMENT,
		GEOMETRY,
		TESS_CONTROL,
		TESS_EVALUATION,
		COMPUTE,
		NUM_SHADER_TYPES
	};


	// An unique 'name' of a shader will be convenient for determining if the 
	// shader is the required one (e.g, provide the shader source file name.
	ShaderProgram(const std::string& name = "unknown");
	~ShaderProgram();

	static bool is_supported();

	static std::string title() { return "ShaderProgram"; }

	void set_name(const std::string& name) { name_ = name; }
	const std::string& name() const { return name_; }

	// ---------------------------------------------------------------

	// returns the program index
	GLuint get_program() const;
	// returns a shader index
	GLuint get_shader(ShaderType) const;

	// remove (delete) all shaders
	void clear();
	
	//---------------------- Creation ---------------------------

	/** Loads the text in the file to the source of the specified shader 
	  *
	  * \param st one of the enum values of ShaderType
	  *	\param filename the file where the source is to be found
	*/
	bool load_shader_from_file(ShaderType st, const std::string& file);
	
	/** Loads the code to the source of the specified shader
	*
	* \param st one of the enum values of ShaderType
	* \param code the string of the shader source code
	*/
	bool load_shader_from_code(ShaderType st, const std::string& code);

	/** Defines semantics for the input vertex attributes. This is
	  * required for other libraries to know how to send data to the shader
	  * Note: To specify a location for an attribute you must do it prior 
	  * to linking the program, or, if the program is already linked, to 
	  * link it again afterwards (call method link_program)
	  *
	  * \param the semantic of the attribute
	  * \param the name of the vertex attribute
	*/
	void set_attrib_name(ShaderProgram::AttribType at, const std::string& name);
	void set_attrib_name(const ShaderProgram::Attribute& attribute);
	void set_attrib_names(const std::vector<ShaderProgram::Attribute>& attributes);

	/** Prepares program for usage. Links it and collects information
	  * about uniform variables and uniform blocks
	*/
	bool link_program();

	/** bind a user-defined varying out variable to a fragment shader color number
	  * Note: linking is required for this operation to take effect
	  * (call method prepare_program afterwards)
	  * \param index the fragment color number
	  * \param the name of the fragment's shader variable
	*/
	void set_program_output(int index, const std::string& name);

	/** returns the fragment shader color number bound to a user-defined varying out variable
	  * Note: linking is required for this operation to take effect
	  * (call method prepare_program afterwards)
	  * \param the name of the fragment's shader variable
	  * \returns the fragment color number
	*/
	GLint program_output(const std::string& name);

	//---------------------- Rendering ---------------------------

	void bind();	// start using the shader

	// generic function to set the uniform <name> to value
	// NOTE: if your uniform is an array type, be careful to use the correct uniform names. For example, you have 
	//		 'uniform vec2/float values[8]' in your shader code, the uniform name is 'values[0]' (not 'values').	
	//		 So calling to this function looks like: program->set_uniform("values[0]", valueArray);
	void set_uniform(const std::string& name, const void *value);

	// For int and bool uniforms. Sets the uniform <name> to the int value
	void set_uniform(const std::string& name, int value);
	
	// For unsigned int uniforms. Sets the uniform <name> to the unsigned int value
	void set_uniform(const std::string& name, unsigned int value);

	// For float uniforms. Sets the uniform <name> to the float value
	void set_uniform(const std::string& name, float value);

	// sets a uniform block as a whole
	void set_block(const std::string& name, const void *value);

	/*  Warning: be careful when using uniform blocks. Please refer to OpenGL Specification Version 4.5 (Core Profile) - May 28, 2015
	*			 (https://www.opengl.org/registry/doc/glspec45.core.pdf#page=159)
	*			 In "Standard Uniform Block Layout", it says "If the member is a three-component vector with components consuming N 
	*			 basic machine units, the base alignment is 4N (see rule 3 of section 7.6.2.2 in page.159). 
	*			 That means the array stride (the bytes between array elements) is always rounded up to the size of a vec4? (ie: 16-bytes).
	*			 So please avoid using vec3/mat3 (use vec4/mat4/mat43 instead) in a uniform block. Otherwise, you have to manually pad your 
	*			 structures/arrays out.	*/
	// sets a uniform inside a named block
	void set_block_uniform(const std::string& blockName, const std::string& uniformName, const void *value);
	
	// sets an element of an array of uniforms inside a block
	void set_block_uniform_array_element(const std::string& blockName, const std::string& uniformName, int arrayIndex, const void* value);

	void bind_texture(const std::string& name, GLuint texture, GLint unit, GLenum target = GL_TEXTURE_2D);
	void unbind_texture(GLenum target = GL_TEXTURE_2D);

	void unbind();	// end using the shader

	// ---------------------- Other info -------------------------------

	int  get_attribute_location(const std::string& name) const;

	// methods to inquire as to what uniforms/attributes are used by this shader. This can 
	// save some compute time if the uniforms or attributes are expensive to compute.
	// Note: the program must be compiled and linked.
	bool is_uniform_used(const std::string& name);
	bool is_attribute_used(const std::string& name);

	// returns GL_VALIDATE_STATUS for the program 
	bool is_program_valid();

	// returns true if program linked, false otherwise
	bool is_program_linked();

	// returns true if shader compiled, false otherwise
	bool is_shader_compiled(ShaderType type);

	// returns true if the program is being used (A program is being used
	//  if you call this between bind() and unbind().
	bool is_bound() const;

	// return true if shader compilation was successfully, false otherwise. 
	// 'log' returns the shader's infolog
	bool shader_info_log(std::string& log, GLuint shader);

	// return true if program linkage was successfully, false otherwise. 
	// 'log' returns the program's infolog
	bool program_info_log(std::string& log);

	// useful for debug. More detailed version are implemented in OpenglInfo class.
	void print_active_attributes();
	void print_active_uniforms();
	void print_active_uniform_blocks();

protected:

	// AUX STRUCTURES

	// stores information for uniforms
	struct Uniform {
		GLenum type;
		GLuint location;
		GLuint size;
		GLuint stride;
	};

	// stores information for block uniforms
	struct BlockUniform {
		GLenum type;
		GLuint offset;
		GLuint size;
		GLuint arrayStride;
	};

	// Uniform blocks are a very convenient feature for two reasons :
	//  (1) Allow uniform sharing between programs �C set once, use many times
	//	(2) Allow setting multiple values at once
	struct UniformBlock { // stores information for a block and its uniforms
		// size of the uniform block
		int size;
		// buffer bound to the index point
		GLuint buffer;
		// binding index
		GLuint bindingIndex;
		// uniforms information
		// maps of std::string can be super slow when calling find with a string literal or const char* 
		// as find forces construction/copy/destruction of a std::sting copy of the const char*.
		std::map<std::string, BlockUniform> uniformOffsets;
	};

	// VARIABLES

	// blockCount is used to assign binding indexes
	static int spBlockCount;

	// Stores info on all blocks found
	// maps of std::string can be super slow when calling find with a string literal or const char* 
	// as find forces construction/copy/destruction of a std::sting copy of the const char*.
	static std::map<std::string, UniformBlock> spBlocks;

	// stores the OpenGL shader types
	static GLenum spGLShaderTypes[ShaderProgram::NUM_SHADER_TYPES];

	// stores the text string related to each type
	static std::string spStringShaderTypes[ShaderProgram::NUM_SHADER_TYPES];

	// aux string used to return the shaders infologs
	std::string pResult;

	// stores the shaders
	GLuint pShader[ShaderProgram::NUM_SHADER_TYPES];
	GLuint program_;

	// stores info on the uniforms
	// maps of std::string can be super slow when calling find with a string literal or const char* 
	// as find forces construction/copy/destruction of a std::sting copy of the const char*.
	std::map<std::string, Uniform> pUniforms;

	// AUX FUNCTIONS

	// aux function to get info on the uniforms referenced by the shaders
	void _add_uniforms();
	
	// aux function to store the info of a uniform
	void _add_uniform(const std::string& name, GLenum type, unsigned int size);

	// aux function to get info on the blocks referenced by the shaders
	void _add_blocks();

	// aux function to determine the size in bytes based on the OpenGL type
	int _type_size(GLenum type) const;

	std::string _type_string(GLenum type) const;

	// aux function to read the shader's source code from file
	bool _read_shader_file(const std::string& file, std::string& shader_code);

private:
	std::string name_;

	static bool	is_supported_;
	static bool initialized_;

	//copying disabled
	ShaderProgram(const ShaderProgram&);
	const ShaderProgram& operator=(const ShaderProgram&);
};

	

#endif
