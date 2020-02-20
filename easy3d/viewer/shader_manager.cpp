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

#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/string.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    std::unordered_map<std::string, ShaderProgram*>     ShaderManager::programs_;
    std::unordered_map<std::string, bool>				ShaderManager::attempt_load_program_; // avoid multiple attempt


    ShaderProgram* ShaderManager::get_program(const std::string& shader_name) {
        std::unordered_map<std::string, ShaderProgram*>::iterator pos = programs_.find(shader_name);
        if (pos != programs_.end()) // program already exists
            return pos->second;
        return nullptr;
    }


    ShaderProgram* ShaderManager::create_program_from_files(
        const std::string& base_name,
        const std::vector<ShaderProgram::Attribute>& attributes /* = std::vector<ShaderProgram::Attribute>() */,
        const std::vector<std::string>& outputs /* = std::vector<std::string>() */,
        bool geom_shader /* = false */ )
    {
        std::unordered_map<std::string, bool>::iterator it = attempt_load_program_.find(base_name);
        if (it == attempt_load_program_.end())
            attempt_load_program_[base_name] = true;
        else if (!attempt_load_program_[base_name])
            return nullptr;

        const std::string dir = resource::directory() + "/shaders/";
        const std::string vs_file = dir + base_name + ".vert";
        if (!file_system::is_file(vs_file)) {
            LOG_FIRST_N(ERROR, 1) << "vertex shader file \'" << vs_file + " does not exist (this is the first record)";
            attempt_load_program_[base_name] = false;
            return nullptr;
        }
        const std::string fs_file = dir + base_name + ".frag";
        if (!file_system::is_file(fs_file)) {
            LOG_FIRST_N(ERROR, 1) << "fragment shader file \'" << fs_file + " does not exist (this is the first record)";
            attempt_load_program_[base_name] = false;
            return nullptr;
        }
        const std::string gs_file = dir + base_name + ".geom";
        if (geom_shader && !file_system::is_file(gs_file)) {
            LOG_FIRST_N(ERROR, 1) << "geometry shader file \'" << gs_file + " does not exist (this is the first record)";
            attempt_load_program_[base_name] = false;
            return nullptr;
        }

        ShaderProgram* program = new ShaderProgram(base_name);

        bool success = program->load_shader_from_file(ShaderProgram::VERTEX, vs_file);
        if (!success) {
            delete program;
            return nullptr;
        }

        success = program->load_shader_from_file(ShaderProgram::FRAGMENT, fs_file);
        if (!success) {
            delete program;
            return nullptr;
        }

        if (geom_shader) {
            success = program->load_shader_from_file(ShaderProgram::GEOMETRY, gs_file);
            if (!success) {
                delete program;
                return nullptr;
            }
        }

        program->set_attrib_names(attributes);	easy3d_debug_log_gl_error;
        for (std::size_t i = 0; i < outputs.size(); ++i)
            program->set_program_output(static_cast<int>(i), outputs[i]);

        success = program->link_program();	easy3d_debug_log_gl_error;
        if (!success) {
            delete program;
            return nullptr;
        }

        programs_[base_name] = program;
        return program;
    }


	// create a shader program from shader source files specified by the shader file's base name.
	// This function allows specifying individual file names and providing extra code.
	ShaderProgram* ShaderManager::create_program_from_files( 
		const std::string& vert_file_name,
		const std::string& frag_file_name,
		const std::string& geom_file_name /* = "" */,
		const std::string& extra_vert_code /* = "" */,
		const std::string& extra_frag_code /* = "" */,
		const std::string& extra_geom_code /* = "" */,
		const std::vector<ShaderProgram::Attribute>& attributes /* = std::vector<ShaderProgram::Attribute>() */, 
		const std::vector<std::string>& outputs /* = std::vector<std::string>() */ )
	{
		// just give a name
		const std::string name = vert_file_name + frag_file_name + geom_file_name;
		std::unordered_map<std::string, bool>::iterator it = attempt_load_program_.find(name);
		if (it == attempt_load_program_.end())
			attempt_load_program_[name] = true;
		else if (!attempt_load_program_[name])
			return nullptr;

        const std::string dir = resource::directory() + "/shaders/";
		const std::string vert_file = dir + vert_file_name;
        if (!file_system::is_file(vert_file)) {
            LOG_FIRST_N(ERROR, 1) << "vertex shader file \'" << vert_file_name + "\' does not exist (this is the first record)";
			attempt_load_program_[name] = false;
			return nullptr;
		}
		const std::string frag_file = dir + frag_file_name;
        if (!file_system::is_file(frag_file)) {
            LOG_FIRST_N(ERROR, 1) << "fragment shader file \'" << frag_file_name + "\' does not exist (this is the first record)";
			attempt_load_program_[name] = false;
			return nullptr;
		}
		const std::string geom_file = dir + geom_file_name;
        if (!geom_file_name.empty() && !file_system::is_file(geom_file)) {
            LOG_FIRST_N(ERROR, 1) << "geometry shader file \'" << geom_file_name + "\' does not exist (this is the first record)";
			attempt_load_program_[name] = false;
			return nullptr;
		}

		ShaderProgram* program = new ShaderProgram(name);

		std::string vert_code;
        file_system::read_file_to_string(vert_file, vert_code);
		if (!extra_vert_code.empty())
			string::replace_substring(vert_code, "//INSERT", extra_vert_code);
        bool success = program->load_shader_from_code(ShaderProgram::VERTEX, vert_code);
        if (!success) {
            delete program;
            return nullptr;
        }
		
		std::string frag_code;
        file_system::read_file_to_string(frag_file, frag_code);
		if (!extra_frag_code.empty())
			string::replace_substring(frag_code, "//INSERT", extra_frag_code);
        success = program->load_shader_from_code(ShaderProgram::FRAGMENT, frag_code);
        if (!success) {
            delete program;
            return nullptr;
        }

		if (!geom_file_name.empty()) {
			std::string geom_code;
            file_system::read_file_to_string(geom_file, geom_code);
			if (!extra_geom_code.empty())
				string::replace_substring(geom_code, "//INSERT", extra_geom_code);
            success = program->load_shader_from_code(ShaderProgram::GEOMETRY, geom_code);
            if (!success) {
                delete program;
                return nullptr;
            }
		}

		program->set_attrib_names(attributes);	easy3d_debug_log_gl_error;
		for (std::size_t i = 0; i < outputs.size(); ++i)
			program->set_program_output(static_cast<int>(i), outputs[i]);

        success = program->link_program();	easy3d_debug_log_gl_error;
        if (!success) {
            delete program;
            return nullptr;
        }

		programs_[name] = program;
		return program;

	}

    std::vector<ShaderProgram*> ShaderManager::all_programs() {
        std::vector<ShaderProgram*> result;

        std::unordered_map<std::string, ShaderProgram*>::iterator pos = programs_.begin();
        for (; pos != programs_.end(); ++pos)
            result.push_back(pos->second);
        return result;
    }


    void ShaderManager::terminate() {
		for (auto p : programs_)
			delete p.second;
        programs_.clear();
        attempt_load_program_.clear();
    }


	void ShaderManager::reload() {
		// simply delete all. the programs will be loaded if needed
		terminate();
	}

} // namespace easy3d
