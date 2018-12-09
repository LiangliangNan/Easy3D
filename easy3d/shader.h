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


#ifndef EASY3D_SHADER_H
#define EASY3D_SHADER_H


#include <string>
#include <unordered_map>

#include <3rd_party/glew/include/GL/glew.h>


namespace easy3d {


    class Shader
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

    public:
        Shader(const std::string& name = "unknown");
        virtual ~Shader();

        // Create program from two or three shader files with same base filename.
        /** Path and extensions (.vert, .frag, and .geom) are automatically
        added to shader base filename (shortcut to Shader::load_from_files).
        \param path - shader files path
        \param base_name - shader base filename
        \param has_geom - do you have a geometry shader?
        \return success
        NOTE: Geometry shader is optional
        **/
        bool load_from_base_name(const std::string& path, const std::string& base_name, bool has_geom = false);

        // Create program from two or three shader files.
        /** Filenames must be absolute (full path).
        \param vert_file - vertex shader file name
        \param frag_file - fragment shader file name
        \param geom_file - geometry shader file name
        \return success
        NOTE: Geometry shader is optional
        **/
        bool load_from_files(const std::string& vert_file, const std::string& frag_file, const std::string& geom_file = "");
        bool load_from_codes(const std::string& vert_code, const std::string& frag_code, const std::string& geom_code = "");

		void set_attrib_name(AttribType at, const std::string& name);

        // Use or not
        void bind();

		// For int and bool uniforms. Sets the uniform <name> to the int value
		void set_uniform(const std::string& name, int value);

		// For unsigned int uniforms. Sets the uniform <name> to the unsigned int value
		void set_uniform(const std::string& name, unsigned int value);

		// For float uniforms. Sets the uniform <name> to the float value
		void set_uniform(const std::string& name, float value);

		// generic function to set the uniform <name> to value
		// NOTE: for array types, e.g., "uniform vec2 values[8]", use "values[0]"
		void set_uniform(const std::string& name, const void *value);

        void unbind();



        /*----------------------- Uniforms -----------------------*/

    private:
        // Reads shader code from a file
        bool read_shader_file(const std::string& file_name, std::string& shader_code);

        // Loads a shader from the code. Returns the program object
        unsigned int load_shader(unsigned int type, const std::string& shader_code);

        void cleanup();

		// get info on the uniforms referenced by the shaders
		void add_uniforms();

		// store the info of a uniform
		void add_uniform(const std::string& name, GLenum type, unsigned int size);

    private:
		std::string		name_;

        // Program ID
        unsigned int	prog_id_;

		// information about a uniform
		struct Uniform {
			GLenum type;
			GLuint location;
			GLuint size;
			GLuint stride;
		};

		// stores info on the uniforms
		// maps of std::string can be super slow when calling find with a string
		// literal or const char* as find forces construction/copy/destruction 
		// of a std::sting copy of the const char*.
		std::unordered_map<std::string, Uniform> uniforms_;
    };

}

#endif
