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

#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    DrawableType TrianglesDrawable::type() const {
        return DT_TRIANGLES;
    }


	void TrianglesDrawable::set_triangle_indices(const std::vector< std::vector<unsigned int> >& indices) {
		indices_ = indices;
		std::size_t count = 0;
		for (const auto& ids : indices) {
			count += ids.size();
		}
		//Selectable::set_num_primitives(count);
	}


	void TrianglesDrawable::set_selected(std::size_t face_idx, bool b) {
        const std::vector<unsigned int>& indices = indices_[face_idx];
        for (auto id : indices) {
//            Selectable::set_selected(id, b);
        }
	}


	bool TrianglesDrawable::is_selected(std::size_t face_idx) const {
        const std::vector<unsigned int>& indices = indices_[face_idx];
        for (auto id : indices) {
//            if (Selectable::is_selected(id))
//                return false;
        }
		return true;
	}


	int TrianglesDrawable::num_selected() const {
		int count = 0;
		for (std::size_t i = 0; i < indices_.size(); ++i) {
			if (is_selected(i))
				++count;
		}
		return count;
	}


	void TrianglesDrawable::get_highlighted_trangles_range(int& tri_min, int& tri_max) const {
        tri_min = highlight_id_;
        tri_max = highlight_id_;
        LOG(WARNING) << "only triangle meshes are supported";

//		if (highlight_id_ >= 0 && highlight_id_ < indices_.size()) {
//			tri_min = indices_[highlight_id_].front();
//			tri_max = indices_[highlight_id_].back();
//		}
//		else {
//			tri_min = -1;
//			tri_max = -1;
//		}
    }


    void TrianglesDrawable::draw(const Camera* camera, bool  with_storage_buffer /* = false */) const {
        if (texture_)
            _draw_triangles_with_texture(camera, with_storage_buffer);
        else
            _draw_triangles(camera, with_storage_buffer);
    }


    void TrianglesDrawable::_draw_triangles(const Camera* camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }

        ShaderProgram* program = ShaderManager::get_program("surface/surface_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("surface/surface_color", attributes);
        }

        if (!program)
            return;

        const mat4& MVP = camera->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3& wCamPos = camera->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4& MV = camera->modelViewMatrix();
        const vec4& wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("lighting", lighting())
			->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("two_sides_lighting", true)
                ->set_uniform("distinct_back_color", true)
                ->set_uniform("smooth_shading", smooth_shading())
                ->set_uniform("ssaoEnabled", false)
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("default_color", default_color())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

//        int min_id, max_id;
//        get_highlighted_trangles_range(min_id, max_id);
        program->set_uniform("hightlight_id_min", highlight_id_);
        program->set_uniform("hightlight_id_max", highlight_id_);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void TrianglesDrawable::_draw_triangles_with_texture(const Camera* camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
            return;
        }

        ShaderProgram* program = ShaderManager::get_program("surface/surface_texture");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("surface/surface_texture", attributes);
        }

        if (!program)
            return;

        const mat4 &MVP = camera->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4 &MV = camera->modelViewMatrix();
        const vec4 &wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("lighting", lighting())
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("two_sides_lighting", false)
                ->set_uniform("smooth_shading", smooth_shading())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess)
                ->bind_texture("textureID", texture()->id(), 0)
                ->set_uniform("texture_repeat", 1.0f)      // TODO: make this a parameter
                ->set_uniform("fractional_repeat", 0.0f);  // TODO: make this a parameter
        gl_draw(with_storage_buffer);
        program->release_texture();
        program->release();
    }

}
