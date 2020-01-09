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


#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/opengl_error.h>


namespace easy3d {

    PointsDrawable::PointsDrawable(const std::string& name /*= ""*/)
        : Drawable(name)
        , point_size_(2.0f)
        , impostor_type_(PLAIN)
    {
        default_color_ = vec3(0.0f, 1.0f, 0.0f);
    }


    DrawableType PointsDrawable::type() const {
        return DT_POINTS;
    }


    void PointsDrawable::draw(const Camera* camera, bool  with_storage_buffer /* = false */) const {
        switch (impostor_type_) {
        case PLAIN:
            if (texture_)
                _draw_plain_points_with_texture(camera, with_storage_buffer);
            else
                _draw_plain_points(camera, with_storage_buffer);
            break;

        case SPHERE:
#if 0
            if (texture_)
                _draw_spheres_with_texture_sprite(camera, with_storage_buffer);
            else
                _draw_sprite_spheres(camera, with_storage_buffer);
#else
            if (texture_)
                _draw_spheres_with_texture_geometry(camera, with_storage_buffer);
            else
                _draw_spheres_geometry(camera, with_storage_buffer);
#endif
            break;

        case SURFEL:
            if (texture_)
                _draw_surfels_with_texture(camera, with_storage_buffer);
            else
                _draw_surfels(camera, with_storage_buffer);
            break;
        }
    }


    void PointsDrawable::_draw_plain_points(const Camera* camera, bool with_storage_buffer) const {
        ShaderProgram* program = ShaderManager::get_program("points/points_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("points/points_plain_color", attributes);
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

        glPointSize(point_size());

        program->bind();
        program->set_uniform("MVP", MVP);
        program->set_uniform("wLightPos", wLightPos);
        program->set_uniform("wCamPos", wCamPos);
        program->set_uniform("lighting", normal_buffer());
        program->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());
        program->set_uniform("default_color", default_color());
        program->set_block_uniform("Material", "ambient", material().ambient);
        program->set_block_uniform("Material", "specular", material().specular);
        program->set_block_uniform("Material", "shininess", &material().shininess);
        gl_draw(with_storage_buffer);
        program->release();
    }


    void PointsDrawable::_draw_spheres_sprite(const Camera* camera, bool with_storage_buffer) const {
        ShaderProgram* program = ShaderManager::get_program("points/points_spheres_sprite_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("points/points_spheres_sprite_color", attributes);
        }

        if (!program)
            return;

        easy3d_debug_gl_error;
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); // starting from GL3.2, using GL_PROGRAM_POINT_SIZE

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE);

        program->set_uniform("MV", camera->modelViewMatrix());
        program->set_uniform("PROJ", camera->projectionMatrix());
        program->set_uniform("screen_width", camera->screenWidth());

        float ratio = camera->pixelGLRatio(camera->sceneCenter());
        program->set_uniform("sphere_radius", point_size() * ratio);

        program->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());
        program->set_uniform("default_color", default_color());
        program->set_uniform("eLightPos", setting::light_position);

        program->set_block_uniform("Material", "ambient", material().ambient);
        program->set_block_uniform("Material", "specular", material().specular);
        program->set_block_uniform("Material", "shininess", &material().shininess);
        gl_draw(with_storage_buffer);
        program->release();

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE); // starting from GL3.2, using GL_PROGRAM_POINT_SIZE
    }


    void PointsDrawable::_draw_spheres_geometry(const Camera* camera, bool with_storage_buffer) const {
        ShaderProgram* program = ShaderManager::get_program("points/points_spheres_geometry_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("points/points_spheres_geometry_color", attributes, std::vector<std::string>(), true);
        }
        if (!program)
            return;

        easy3d_debug_gl_error;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE);

        program->set_uniform("MV", camera->modelViewMatrix());
        program->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->sceneCenter());
        program->set_uniform("sphere_radius", point_size() * ratio);

        program->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());
        program->set_uniform("default_color", default_color());
        program->set_uniform("eLightPos", setting::light_position);

        program->set_block_uniform("Material", "ambient", material().ambient);
        program->set_block_uniform("Material", "specular", material().specular);
        program->set_block_uniform("Material", "shininess", &material().shininess);
        gl_draw(with_storage_buffer);
        program->release();
    }


    void PointsDrawable::_draw_plain_points_with_texture(const Camera* camera, bool with_storage_buffer) const {
        std::cerr << "to be implemented ..." << std::endl;
    }


    void PointsDrawable::_draw_spheres_with_texture_sprite(const Camera* camera, bool with_storage_buffer) const {
        std::cerr << "to be implemented ..." << std::endl;
    }


    void PointsDrawable::_draw_spheres_with_texture_geometry(const Camera* camera, bool with_storage_buffer) const {
        std::cerr << "to be implemented ..." << std::endl;
    }


    void PointsDrawable::_draw_surfels(const Camera *camera, bool with_storage_buffer) const {
        std::cerr << "to be implemented ..." << std::endl;
    }


    void PointsDrawable::_draw_surfels_with_texture(const Camera *camera, bool with_storage_buffer) const {
        std::cerr << "to be implemented ..." << std::endl;
    }


}
