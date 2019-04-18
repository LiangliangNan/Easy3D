/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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

#ifndef EASY3D_SHADER_PROGRAM_H
#define EASY3D_SHADER_PROGRAM_H

#include <unordered_map>
#include <vector>
#include <string>


namespace easy3d {


	// Liangliang: about Shader Compilation
	// https://www.khronos.org/opengl/wiki/Shader_Compilation#Error_handling

    // To use the shader program class, you need to do the following:
    // - Call load_shader_from_code(ShaderProgram::VERTEX, vert_file/code) to create vertex shader and
    //        load_shader_from_code(ShaderProgram::FRAGMENT, frag_file/code) to create fragment shader
    //        (you may also need to create other types of shaders depending on your needs).
    // - Call set_attrib_name(ShaderProgram::POSITION, "position") for vertex attribute "position".
    //        You may also need to set other attribute like normal, color, etc. To know what vertex
    //        attributes need to be set, check your shader code or call print_active_attributes().
    // - Call link_program() to Link the program.
    //
    // For rendering
    // - Call bind();
    // - Call set_uniform() to set all the necessary uniforms. You may also need to call set_block(),
    //        set_block_uniform(), and/or set_block_uniform_array_element(). To know what uniforms need to
    //        be set, check your shader code or call print_active_uniforms(), print_active_uniform_blocks().
    // - Call the drawable's draw() function
    // - Call release();
    // To retrieve the model view projection matrix, call modelViewProjectionMatrix().

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

        static bool is_supported();

		// An unique 'name' of a shader will be convenient for determining if the 
		// shader is the required one (e.g, provide the shader source file name.
		ShaderProgram(const std::string& name = "unknown");
		~ShaderProgram();

		void set_name(const std::string& name) { name_ = name; }
		const std::string& name() const { return name_; }

		// ---------------------------------------------------------------

		// returns the program index
        unsigned int get_program() const;

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
        int program_output(const std::string& name);

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

		// tex_target: GL_TEXTURE_2D, GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_ARRAY
		// default value is GL_TEXTURE_2D (0x0DE1, just to eliminate the inclusion of gl header file).
		void bind_texture(const std::string& name, unsigned int texture, int unit, unsigned int tex_target = 0x0DE1);
        void release_texture(unsigned int tex_target = 0x0DE1);

        void release();	// end using the shader

		// ---------------------- Other info -------------------------------

		int  get_attribute_location(const std::string& name) const;

		// methods to inquire as to what uniforms/attributes are used by this shader. This can 
		// save some compute time if the uniforms or attributes are expensive to compute.
		// Note: the program must be compiled and linked.
		bool is_uniform_used(const std::string& name);
		bool is_attribute_used(const std::string& name);

		// returns GL_VALIDATE_STATUS for the program 
        // glValidateProgram() is meant to be called directly before a draw call (i.e., glDraw*())
        // with that shader bound and all the bindings (VAO, textures) set. Its purpose is to ensure
        // that the shader can execute given the current GL state.
		bool is_program_valid();

		// returns true if program linked, false otherwise
		bool is_program_linked();

		// returns true if the program is being used (A program is being used
        //  if you call this between bind() and release().
		bool is_bound() const;

		// return true if shader compilation was successfully, false otherwise. 
		// 'log' returns the shader's infolog
        bool shader_info_log(std::string& log, unsigned int shader);

		// return true if program linkage was successfully, false otherwise. 
		// 'log' returns the program's infolog
		bool program_info_log(std::string& log);

		// useful for debug. More detailed version are implemented in OpenglInfo class.
        // NOTE: require OpenGL >= 4.3
		void print_active_attributes();
		void print_active_uniforms();
		void print_active_uniform_blocks();

		// ---------------------- Load/Save binary -------------------------------
		// The ready-to-use (i.e., compiled and linked) program in a single binary file.
        // NOTE: require OpenGL >= 4.1
		bool load_binary(const std::string& file_name);
        bool save_binary(const std::string& file_name);

	protected:

		// AUX STRUCTURES

		// stores information for uniforms
		struct Uniform {
            std::string  name;
            unsigned int type;
            unsigned int location;
            unsigned int size;
            unsigned int stride;
		};

		// stores information for block uniforms
		struct BlockUniform {
            std::string name;
            unsigned int type;
            unsigned int offset;
            unsigned int size;
            unsigned int arrayStride;
		};

		// Uniform blocks are a very convenient feature for two reasons :
		//  (1) Allow uniform sharing between programs �C set once, use many times
		//	(2) Allow setting multiple values at once
		struct UniformBlock { // stores information for a block and its uniforms
            std::string name;
			// size of the uniform block
			int size;
			// buffer bound to the index point
            unsigned int buffer;
			// binding index
            unsigned int bindingIndex;
			// uniforms information
			// maps of std::string can be super slow when calling find with a string literal or const char* 
			// as find forces construction/copy/destruction of a std::sting copy of the const char*.
            std::unordered_map<std::string, BlockUniform> uniformOffsets;
		};

		// VARIABLES

		// blockCount is used to assign binding indexes
		static int spBlockCount;

		// Stores info on all blocks found
		// maps of std::string can be super slow when calling find with a string literal or const char* 
		// as find forces construction/copy/destruction of a std::sting copy of the const char*.
        static std::unordered_map<std::string, UniformBlock> spBlocks;

		// stores the OpenGL shader types
        static unsigned int spGLShaderTypes[ShaderProgram::NUM_SHADER_TYPES];

		// stores the text string related to each type
		static std::string spStringShaderTypes[ShaderProgram::NUM_SHADER_TYPES];

        // the program handle
        unsigned int program_;

		// stores info on the uniforms
		// maps of std::string can be super slow when calling find with a string literal or const char* 
		// as find forces construction/copy/destruction of a std::sting copy of the const char*.
        std::unordered_map<std::string, Uniform> pUniforms;

		// AUX FUNCTIONS

		// aux function to get info on the uniforms referenced by the shaders
		void _add_uniforms();

		// aux function to store the info of a uniform
        void _add_uniform(const std::string& name, unsigned int type, unsigned int size);

		// aux function to get info on the blocks referenced by the shaders
		void _add_blocks();

		// aux function to determine the size in bytes based on the OpenGL type
        int _type_size(unsigned int type) const;

        std::string _type_string(unsigned int type) const;

		// aux function to read the shader's source code from a file
		std::string _read_file(const std::string& file);

	private:
		std::string name_;

		//copying disabled
		ShaderProgram(const ShaderProgram&);
        ShaderProgram& operator=(const ShaderProgram&);
	};

}


#endif  // EASY3D_SHADER_PROGRAM_H
