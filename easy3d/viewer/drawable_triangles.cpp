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
#include <easy3d/viewer/clipping_plane.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    DrawableType TrianglesDrawable::type() const {
        return DT_TRIANGLES;
    }


    void TrianglesDrawable::draw(const Camera *camera, bool with_storage_buffer /* = false */) const {
        if (use_texture_ && texture_)
            _draw_triangles_with_texture(camera, with_storage_buffer);
        else
            _draw_triangles(camera, with_storage_buffer);
    }


    void TrianglesDrawable::_draw_triangles(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("surface/surface_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("surface/surface_color", attributes);
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
                ->set_uniform("two_sides_lighting", lighting_two_sides())
                ->set_uniform("distinct_back_color", distinct_back_color())
                ->set_uniform("backside_color", back_color())
                ->set_uniform("smooth_shading", smooth_shading())
                ->set_uniform("ssaoEnabled", false)
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("default_color", default_color())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess)
                ->set_uniform("highlight", highlight())
                ->set_uniform("hightlight_id_min", highlight_range_.first)
                ->set_uniform("hightlight_id_max", highlight_range_.second);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void TrianglesDrawable::_draw_triangles_with_texture(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("surface/surface_texture");
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
                ->set_uniform("two_sides_lighting", lighting_two_sides())
                ->set_uniform("distinct_back_color", distinct_back_color())
                ->set_uniform("backside_color", back_color())
                ->set_uniform("smooth_shading", smooth_shading())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess)
                ->bind_texture("textureID", texture()->id(), 0)
                ->set_uniform("texture_repeat", texture_repeat())
                ->set_uniform("fractional_repeat", texture_fractional_repeat())
                ->set_uniform("highlight", highlight())
                ->set_uniform("hightlight_id_min", highlight_range_.first)
                ->set_uniform("hightlight_id_max", highlight_range_.second);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);

        program->release_texture();
        program->release();
    }

}
