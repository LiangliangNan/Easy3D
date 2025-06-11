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
	 * \details This class creates programs, loads shaders from files, associates vertex attribute names with
	 *		locations, and works with uniforms, including blocks. It also provides access to the info logs, and many
	 *		more useful functionalities.
     *		To use the shader program class, you need to do the following:
     *		- Call load_shader_from_code(ShaderProgram::VERTEX, vert_file) to create vertex shader and
     *        load_shader_from_code(ShaderProgram::FRAGMENT, frag_file) to create fragment shader
     *        (you may also need to create other types of shaders depending on your needs).
     *		- Call set_attrib_name(ShaderProgram::POSITION, "position") for vertex attribute "position".
     *        You may also need to set other attributes like normal, color, etc. To know what vertex
     *        attributes need to be set, check your shader code or call print_active_attributes().
     *		- Call link_program() to link the program.
     *
     *		For rendering
     *		- Call bind();
     *		- Call set_uniform() to set all the necessary uniforms. You may also need to call set_block(),
     *        set_block_uniform(), and/or set_block_uniform_array_element(). To know what uniforms need to
     *        be set, check your shader code or call print_active_uniforms(), print_active_uniform_blocks().
     *		- Call the drawable's draw() function
     *		- Call release();
     *
     *		To retrieve the model view projection matrix, call camera's <code> modelViewProjectionMatrix() </code>
     */
	class ShaderProgram
	{
	public:
		/// Types of vertex attributes
		enum AttribType {
			POSITION,	///< Position
			COLOR,		///< Color
			NORMAL,		///< Normal
			TEXCOORD	///< Texture coordinates
		};
		/// Attribute: a pair of attribute type and attribute name
		typedef std::pair<AttribType, std::string> Attribute;

		/// Types of Shaders
		enum ShaderType {
			VERTEX,				///< Vertex shader
			FRAGMENT,			///< Fragment shader
			GEOMETRY,			///< Geometry shader
			TESS_CONTROL,		///< Tessellation control shader
			TESS_EVALUATION,	///< Tessellation evaluation shader
			COMPUTE,			///< Compute shader
			NUM_SHADER_TYPES	///< Number of shader types
		};

		/**
		 * \brief Is shader program supported?
		 * \return \c true if supported, \c false otherwise.
		 */
        static bool is_supported();

		/**
		 * Constructor.
		 * \param name A unique name for a program, convenient to distinguish multiple programs. This is quite helpful
		 *      in identifying issues when multiple programs are used. It is suggested to use the source file name.
		 */
		explicit ShaderProgram(const std::string& name = "unknown");
		/// Destructor
		~ShaderProgram();

		/**
		 * \brief Set the name of the shader program.
		 * \param name The name to set.
		 */
		void set_name(const std::string& name) { name_ = name; }
		/**
		 * \brief Get the name of the shader program.
		 * \return The name of the shader program.
		 */
		const std::string& name() const { return name_; }

		/**
		 * \brief Set verbose mode.
		 * \param v True to enable verbose mode (log any issues found), false to disable.
		 */
        void set_verbose(bool v) { verbose_ = v; }

		// ---------------------------------------------------------------

		/**
		 * \brief Get the program index.
		 * \return The program index.
		 */
        unsigned int get_program() const;

		/**
		 * \brief Remove (delete) all shaders.
		 */
		void clear();

		//---------------------- Creation ---------------------------

		/**
		 * \brief Load the text in the file to the source of the specified shader.
		 * \param st One of the enum values of ShaderType.
		 * \param file_name The full path of the file where the source is to be found.
		 * \param inc_id The include identifier string (e.g., "#include").
		 * \return True if the shader was loaded successfully, false otherwise.
		 */
		bool load_shader_from_file(ShaderType st, const std::string& file_name, const std::string& inc_id = "#include");

		/**
		 * \brief Load the code to the source of the specified shader.
		 * \param st One of the enum values of ShaderType.
		 * \param code The string of the shader source code.
		 * \return True if the shader was loaded successfully, false otherwise.
		 */
		bool load_shader_from_code(ShaderType st, const std::string& code);

		/**
		 * \brief Define semantics for the input vertex attributes. This is required for other libraries to know how to
		 * send data to the shader.
		 * \param at The semantic of the attribute.
		 * \param name The name of the vertex attribute.
		 * \note To specify a location for an attribute you must do it prior to linking the program, or, if the
		 *      program is already linked, to link it again afterward (call method link_program).
		 */
		void set_attrib_name(ShaderProgram::AttribType at, const std::string& name) const;
		/**
		 * \brief Define semantics for multiple input vertex attributes.
		 * \param attributes A vector of attributes to set.
		 */
		void set_attrib_names(const std::vector<ShaderProgram::Attribute>& attributes) const;

		/**
		 * \brief Prepare program for usage. Links it and collects information about uniform variables and uniform blocks.
		 * \return True if the program was linked successfully, false otherwise.
		 */
		bool link_program();

		/**
		 * \brief Bind a user-defined varying out variable to a fragment shader color number.
		 * \note Linking is required for this operation to take effect (call method prepare_program afterward).
		 * \param index The fragment color number.
		 * \param name The name of the fragment's shader variable.
		 */
		void set_program_output(int index, const std::string& name) const;

		/**
		 * \brief Get the fragment shader color number bound to a user-defined varying out variable.
		 * \note Linking is required for this operation to take effect (call method prepare_program afterward).
		 * \param name The name of the fragment's shader variable.
		 * \return The fragment color number.
		 */
        int program_output(const std::string& name) const;

		//---------------------- Rendering ---------------------------

		/**
		 * \brief Start using the program.
		 */
		void bind() const;

		/**
		 * \brief Set the uniform to value.
		 * \param name The name of the uniform.
		 * \param value The value to set.
		 * \return A pointer to the ShaderProgram object.
		 * \note If your uniform is an array type, be careful to use the correct uniform names. For example, you have
		 *		'uniform vec2/float values[8]' in your shader code, the uniform name is 'values[0]' (not 'values').
		 *		So calling to this function looks like: program->set_uniform("values[0]", valueArray);
		 */
		ShaderProgram* set_uniform(const std::string& name, const void *value);

		/**
		 * \brief Set the uniform to the int value.
		 * \param name The name of the uniform.
		 * \param value The int value to set.
		 * \return A pointer to the ShaderProgram object.
		 */
		ShaderProgram* set_uniform(const std::string& name, int value);

		/**
		 * \brief Set the uniform to the unsigned int value.
		 * \param name The name of the uniform.
		 * \param value The unsigned int value to set.
		 * \return A pointer to the ShaderProgram object.
		 */
		ShaderProgram* set_uniform(const std::string& name, unsigned int value);

		/**
		 * \brief Set the uniform to the float value.
		 * \param name The name of the uniform.
		 * \param value The float value to set.
		 * \return A pointer to the ShaderProgram object.
		 */
		ShaderProgram* set_uniform(const std::string& name, float value);

		/**
		 * \brief Set a uniform block as a whole.
		 * \param name The name of the uniform block.
		 * \param value The value to set.
		 * \return A pointer to the ShaderProgram object.
		 */
		ShaderProgram* set_block(const std::string& name, const void *value);

		/**
		 * \brief Set a uniform inside a named block.
		 * \param blockName The name of the uniform block.
		 * \param uniformName The name of the uniform.
		 * \param value The value to set.
		 * \return A pointer to the ShaderProgram object.
		 * \warning Be careful when using uniform blocks. Please refer to OpenGL Specification Version 4.5
		 *		(Core Profile) - May 28, 2015 (https://www.opengl.org/registry/doc/glspec45.core.pdf#page=159)
		 *		In "Standard Uniform Block Layout", it says "If the member is a three-component vector with components
		 *		consuming N basic machine units, the base alignment is 4N" (see rule 3 of section 7.6.2.2 in page.159).
		 *		That means the array stride (the bytes between array elements) is always rounded up to the size of a
		 *		vec4 (i.e., 16-bytes).
		 *		So we have to avoid using vec3/mat3 (use vec4/mat4/mat43 instead) in a uniform block. Otherwise,
		 *		you have to manually pad your structures/arrays out.
		 */
		ShaderProgram* set_block_uniform(const std::string& blockName, const std::string& uniformName, const void *value);

		/**
		 * \brief Set an element of an array of uniforms inside a block.
		 * \param blockName The name of the uniform block.
		 * \param uniformName The name of the uniform.
		 * \param arrayIndex The index of the array element.
		 * \param value The value to set.
		 * \return A pointer to the ShaderProgram object.
		 */
		ShaderProgram* set_block_uniform_array_element(const std::string& blockName, const std::string& uniformName, int arrayIndex, const void* value);

		/**
		 * \brief Bind a texture to the shader program.
		 * \param name The name of the texture.
		 * \param tex_id The texture ID.
		 * \param unit The texture unit.
		 * \param tex_target The texture target, which can be GL_TEXTURE_2D (default), GL_TEXTURE_2D_MULTISAMPLE, or
		 *		GL_TEXTURE_2D_ARRAY.
		 * \return A pointer to the ShaderProgram object.
		 */
		// default value is GL_TEXTURE_2D (0x0DE1, just to eliminate the inclusion of gl header file).
		ShaderProgram* bind_texture(const std::string& name, unsigned int tex_id, int unit, unsigned int tex_target = 0x0DE1);
		/**
		 * \brief Release a texture from the shader program.
		 * \param tex_target The texture target (default is GL_TEXTURE_2D).
		 * \return A pointer to the ShaderProgram object.
		 */
		ShaderProgram* release_texture(unsigned int tex_target = 0x0DE1);

		/**
		 * \brief End using the program.
		 */
        void release() const;

		// ---------------------- Other info -------------------------------

		/**
		 * \brief Get the location of an attribute.
		 * \param name The name of the attribute.
		 * \return The location of the attribute.
		 */
		int get_attribute_location(const std::string& name) const;

		/**
		 * \brief Check if a uniform is used by the shader.
		 * \details This can save some compute time if the uniforms or attributes are expensive to compute.
		 * \param name The name of the uniform.
		 * \return True if the uniform is used, false otherwise.
		 * \note The program must be compiled and linked.
		 */
		bool is_uniform_used(const std::string& name);

		/**
		 * \brief Check if an attribute is used by the shader.
		 * \param name The name of the attribute.
		 * \return True if the attribute is used, false otherwise.
		 */
		bool is_attribute_used(const std::string& name);

		/**
		 * \brief Check if the program is valid.
		 * \return True if the program is valid, false otherwise.
		 * \note glValidateProgram() is meant to be called directly before a draw call (i.e., glDraw*())
		 *		with that shader bound and all the bindings (VAO, textures) set. Its purpose is to ensure
		 *		that the shader can execute given the current GL state.
		 */
		bool is_program_valid() const;

		/**
		 * \brief Check if the program is linked.
		 * \return True if the program is linked, false otherwise.
		 */
		bool is_program_linked() const;

		/**
		 * \brief Check if the program is bound.
		 * \return True if the program is bound (i.e., between bind() and  release()), false otherwise.
		 */
		bool is_bound() const;

		/**
		 * \brief Get the shader info log.
		 * \param log The string to store the shader's info log.
		 * \param shader The shader ID.
		 * \return True if shader compilation was successfully, false otherwise.
		 */
        bool shader_info_log(std::string& log, unsigned int shader) const;

		/**
		 * \brief Get the program info log.
		 * \param log The string to store the program's info log.
		 * \return True if program linkage was successfully, false otherwise.
		 */
		bool program_info_log(std::string& log) const;

		/**
		 * \brief Print active attributes.
		 * \note Requires OpenGL >= 4.3.
		 */
		void print_active_attributes();
		/**
		 * \brief Print active uniforms.
		 * \note Requires OpenGL >= 4.3.
		 */
		void print_active_uniforms();
		/**
		 * \brief Print active uniform blocks.
		 * \note Requires OpenGL >= 4.3.
		 */
		void print_active_uniform_blocks();

		// ---------------------- Load/Save binary -------------------------------
		/**
		 * \brief Load a binary (i.e., compiled and linked) program.
		 * \param file_name The name of the file to load the binary from.
		 * \return True if the binary was loaded successfully, false otherwise.
		 * \note Requires OpenGL >= 4.1.
		 */
		bool load_binary(const std::string& file_name);
		/**
		 * \brief Save the program to a binary file.
		 * \param file_name The name of the file to save the binary to.
		 * \return True if the binary was saved successfully, false otherwise.
		 * \note Requires OpenGL >= 4.1.
		 */
        bool save_binary(const std::string& file_name);

		/**
		 * \brief Auxiliary function that loads the shader source code from a file.
		 * \param file_name The full path of the file where the source is to be found.
		 * \param inc_id The include identifier string (e.g., "#include").
		 * \return The shader source code as a string.
		 */
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
