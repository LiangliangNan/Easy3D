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

#ifndef EASY3D_RENDERER_SHADER_MANAGER_H
#define EASY3D_RENDERER_SHADER_MANAGER_H

#include <string>
#include <unordered_map>

#include <easy3d/renderer/shader_program.h>


namespace easy3d {

    class ShaderProgram;

	/**
	 * \brief Management of shader programs.
	 * \class ShaderManager easy3d/renderer/shader_manager.h
	 * \note Make sure to call terminate() to destroy existing programs before the OpenGL context is deleted.
	 */
    class ShaderManager
    {
    public:
    	/**
		 * \brief Get the shader program if it exists and is working.
		 * \param shader_name The base name of the program's source file.
		 * \return The shader program if it exists and is working, otherwise nullptr.
		 */
        static ShaderProgram* get_program(const std::string& shader_name);

    	/**
		 * \brief Create a shader program from shader source files specified by the shader file's base name.
		 * \param file_base_name The base name of the shader files.
		 * \param attributes The attributes for the shader program.
		 * \param outputs The outputs for the shader program.
		 * \param geom_shader True if a geometry shader is included.
		 * \return The created shader program.
		 */
        static ShaderProgram* create_program_from_files(
            const std::string& file_base_name,
            const std::vector<ShaderProgram::Attribute>& attributes = std::vector<ShaderProgram::Attribute>(),
            const std::vector<std::string>& outputs = std::vector<std::string>(),
            bool geom_shader = false
        );

    	/**
		 * \brief Create a shader program from shader source files specified by individual file names.
		 * \param vert_file_name The vertex shader file name.
		 * \param frag_file_name The fragment shader file name.
		 * \param geom_file_name The geometry shader file name (optional).
		 * \param extra_vert_code Extra vertex shader code (optional).
		 * \param extra_frag_code Extra fragment shader code (optional).
		 * \param extra_geom_code Extra geometry shader code (optional).
		 * \param attributes The attributes for the shader program.
		 * \param outputs The outputs for the shader program.
		 * \return The created shader program.
		 */
		static ShaderProgram* create_program_from_files(
			const std::string& vert_file_name,
			const std::string& frag_file_name,
			const std::string& geom_file_name = "",
			const std::string& extra_vert_code = "",
			const std::string& extra_frag_code = "",
			const std::string& extra_geom_code = "",
			const std::vector<ShaderProgram::Attribute>& attributes = std::vector<ShaderProgram::Attribute>(),
			const std::vector<std::string>& outputs = std::vector<std::string>()
		);

    	/**
		 * \brief Create a shader program from completed shader source codes.
		 * \param vert_code The vertex shader code.
		 * \param frag_code The fragment shader code.
		 * \param geom_code The geometry shader code (optional).
		 * \param attributes The attributes for the shader program.
		 * \param outputs The outputs for the shader program.
		 * \return The created shader program.
		 */
        static ShaderProgram* create_program_from_codes(
                const std::string& vert_code,
                const std::string& frag_code,
                const std::string& geom_code = "",
                const std::vector<ShaderProgram::Attribute>& attributes = std::vector<ShaderProgram::Attribute>(),
                const std::vector<std::string>& outputs = std::vector<std::string>()
        );

    	/**
		 * \brief Get all shader programs.
		 * \return A vector of all shader programs.
		 */
        static std::vector<ShaderProgram*> all_programs();

    	/**
		 * \brief Destroy all shader programs.
		 */
        static void terminate();

    	/**
		 * \brief Reload all shader programs.
		 */
		static void reload();

    private:
        // maps of std::string can be super slow when calling find with a string literal or const char*
        // as find forces construction/copy/destruction of a std::sting copy of the const char*.
        static std::unordered_map<std::string, ShaderProgram*>	programs_;
        static std::unordered_map<std::string, bool>			attempt_load_program_; // avoid multiple attempt
    };

}


#endif  // EASY3D_RENDERER_SHADER_MANAGER_H
