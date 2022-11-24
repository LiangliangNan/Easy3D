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

#ifndef EASY3D_RENDERER_SHADER_PROGRAM_H
#define EASY3D_RENDERER_SHADER_PROGRAM_H

#include <unordered_map>
#include <vector>
#include <string>


namespace easy3d {


    /**
     * \brief OpenGL Shader Compilation
     *
     * \class ShaderProgram easy3d/renderer/shader_program.h
     *
     * To use the shader program class, you need to do the following:
     * - Call load_shader_from_code(ShaderProgram::VERTEX, vert_file) to create vertex shader and
     *        load_shader_from_code(ShaderProgram::FRAGMENT, frag_file) to create fragment shader
     *        (you may also need to create other types of shaders depending on your needs).
     * - Call set_attrib_name(ShaderProgram::POSITION, "position") for vertex attribute "position".
     *        You may also need to set other attributes like normal, color, etc. To know what vertex
     *        attributes need to be set, check your shader code or call print_active_attributes().
     * - Call link_program() to link the program.
     *
     * For rendering
     * - Call bind();
     * - Call set_uniform() to set all the necessary uniforms. You may also need to call set_block(),
     *        set_block_uniform(), and/or set_block_uniform_array_element(). To know what uniforms need to
     *        be set, check your shader code or call print_active_uniforms(), print_active_uniform_blocks().
     * - Call the drawable's draw() function
     * - Call release();
     *
     * To retrieve the model view projection matrix, call camera's <code> modelViewProjectionMatrix() </code>
     */

	class ShaderProgram
	{
	public:

		// Types of Vertex Attributes
		enum AttribType {
			POSITION,
			COLOR,
			NORMAL,
			TEXCOORD
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

		/**
		 * Constructor.
		 * \param name A unique name for a program, convenient to distinguish multiple programs. This is quite helpful
		 *      in identifying issues when multiple programs are used. It is suggested to use the source file name.
		 */
		explicit ShaderProgram(const std::string& name = "unknown");
		~ShaderProgram();

		void set_name(const std::string& name) { name_ = name; }
		const std::string& name() const { return name_; }

        /// Sets true to log any issues found.
        void set_verbose(bool v) { verbose_ = v; }

		// ---------------------------------------------------------------

        /// Returns the program index.
        unsigned int get_program() const;

        /// Removes (deletes) all shaders
		void clear();

		//---------------------- Creation ---------------------------

		/**
		 * Loads the text in the file to the source of the specified shader
		 * \param st one of the enum values of ShaderType
         * \param file_name the full path of the file where the source is to be found
		 * \param inc_id the include identifier string (e.g., "#include")
		 */
		bool load_shader_from_file(ShaderType st, const std::string& file_name, const std::string& inc_id = "#include");

		/**
		 * Loads the code to the source of the specified shader
		 * \param st one of the enum values of ShaderType
		 * \param code the string of the shader source code
		 */
		bool load_shader_from_code(ShaderType st, const std::string& code);

		/**
		 * Defines semantics for the input vertex attributes. This is required for other libraries to know how to
		 * send data to the shader.
		 * \param at The semantic of the attribute
		 * \param name The name of the vertex attribute
		 * \note To specify a location for an attribute you must do it prior to linking the program, or, if the
		 *      program is already linked, to link it again afterwards (call method link_program)
		 */
		void set_attrib_name(ShaderProgram::AttribType at, const std::string& name) const;
		void set_attrib_names(const std::vector<ShaderProgram::Attribute>& attributes) const;

		/**
		 * Prepares program for usage. Links it and collects information about uniform variables and uniform blocks.
		 */
		bool link_program();

		/**
		 * Bind a user-defined varying out variable to a fragment shader color number
		 * Note: linking is required for this operation to take effect
		 * (call method prepare_program afterwards)
		 * \param index The fragment color number
		 * \param name The name of the fragment's shader variable
		 */
		void set_program_output(int index, const std::string& name) const;

		/**
		 * Returns the fragment shader color number bound to a user-defined varying out variable
		 * Note: linking is required for this operation to take effect
		 * (call method prepare_program afterwards)
		 * \param the name of the fragment's shader variable
		 * \returns the fragment color number
		 */
        int program_output(const std::string& name) const;

		//---------------------- Rendering ---------------------------

        /// Starts using the program.
		void bind() const;

		// generic function to set the uniform <name> to value
		// NOTE: if your uniform is an array type, be careful to use the correct uniform names. For example, you have 
		//		 'uniform vec2/float values[8]' in your shader code, the uniform name is 'values[0]' (not 'values').	
		//		 So calling to this function looks like: program->set_uniform("values[0]", valueArray);
		ShaderProgram* set_uniform(const std::string& name, const void *value);

		// For int and bool uniforms. Sets the uniform <name> to the int value
		ShaderProgram* set_uniform(const std::string& name, int value);

		// For unsigned int uniforms. Sets the uniform <name> to the unsigned int value
		ShaderProgram* set_uniform(const std::string& name, unsigned int value);

		// For float uniforms. Sets the uniform <name> to the float value
		ShaderProgram* set_uniform(const std::string& name, float value);

		/**
		 * Sets a uniform block as a whole
		 */
		ShaderProgram* set_block(const std::string& name, const void *value);

		/**
		 * Sets a uniform inside a named block
		 * Warning: Be careful when using uniform blocks. Please refer to OpenGL Specification Version 4.5
		 *      (Core Profile) - May 28, 2015 (https://www.opengl.org/registry/doc/glspec45.core.pdf#page=159)
		 *		In "Standard Uniform Block Layout", it says "If the member is a three-component vector with components
		 *		consuming N basic machine units, the base alignment is 4N" (see rule 3 of section 7.6.2.2 in page.159).
		 *		That means the array stride (the bytes between array elements) is always rounded up to the size of a
		 *		vec4 (i.e., 16-bytes).
		 *		So we have to avoid using vec3/mat3 (use vec4/mat4/mat43 instead) in a uniform block. Otherwise,
		 *		you have to manually pad your structures/arrays out.
		 */
		ShaderProgram* set_block_uniform(const std::string& blockName, const std::string& uniformName, const void *value);

		/**
		 * Sets an element of an array of uniforms inside a block
		 */
		ShaderProgram* set_block_uniform_array_element(const std::string& blockName, const std::string& uniformName, int arrayIndex, const void* value);

		// tex_target: GL_TEXTURE_2D, GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_ARRAY
		// default value is GL_TEXTURE_2D (0x0DE1, just to eliminate the inclusion of gl header file).
		ShaderProgram* bind_texture(const std::string& name, unsigned int tex_id, int unit, unsigned int tex_target = 0x0DE1);
		ShaderProgram* release_texture(unsigned int tex_target = 0x0DE1);

        /// Ends using the program.
        void release() const;

		// ---------------------- Other info -------------------------------

		int get_attribute_location(const std::string& name) const;

		// methods to inquire as to what uniforms/attributes are used by this shader. This can 
		// save some compute time if the uniforms or attributes are expensive to compute.
		// Note: the program must be compiled and linked.
		bool is_uniform_used(const std::string& name);
		bool is_attribute_used(const std::string& name);

		// returns GL_VALIDATE_STATUS for the program 
        // glValidateProgram() is meant to be called directly before a draw call (i.e., glDraw*())
        // with that shader bound and all the bindings (VAO, textures) set. Its purpose is to ensure
        // that the shader can execute given the current GL state.
		bool is_program_valid() const;

		/// Returns true if program linked, false otherwise.
		bool is_program_linked() const;

		/// Returns true if the program is being used (i.e., between bind() and  release()).
		bool is_bound() const;

		// return true if shader compilation was successfully, false otherwise. 
		// 'log' returns the shader's infolog
        bool shader_info_log(std::string& log, unsigned int shader) const;

		// return true if program linkage was successfully, false otherwise. 
		// 'log' returns the program's infolog
		bool program_info_log(std::string& log) const;

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

        /**
         * Auxiliary function that loads the shader source code from a file.
         * \param file_name the full path of the file where the source is to be found
         * \param inc_id the include identifier string (e.g., "#include")
         */
        // aux function to read the shader's source code from a file
        static std::string load_shader_source(const std::string& file_name, const std::string& inc_id = "#include");

	protected:

		// AUX STRUCTURES

		// stores information for uniforms
		struct Uniform {
            std::string  name;
            unsigned int type;
            int location;
            int size;
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

        // aux function to read a previously compiled and linked program code from a file
        std::string _read_linked_program(const std::string& file);

	private:
		std::string name_;
        bool verbose_; // log any issues found

		//copying disabled
		ShaderProgram(const ShaderProgram&);
        ShaderProgram& operator=(const ShaderProgram&);

    private:
	    friend class ShaderManager;
	};

}


#endif  // EASY3D_RENDERER_SHADER_PROGRAM_H
