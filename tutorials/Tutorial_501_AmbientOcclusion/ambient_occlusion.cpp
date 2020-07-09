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

#include "ambient_occlusion.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/setting.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialAmbientOcclusion::TutorialAmbientOcclusion(const std::string& title) : Viewer(title) {
	ao_ = new AmbientOcclusion(camera());
	ao_enabled_ = true;
}


std::string TutorialAmbientOcclusion::usage() const {
    return ("----------------- Ambient Occlusion usage ----------------- \n"
            "Press key 'space' to switch between Ambient Occlusion and normal rendering\n"
            "Press 'up/down' to increase/decrease the radius\n"
            "----------------------------------------------------------- \n");
}


bool TutorialAmbientOcclusion::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
		ao_enabled_ = !ao_enabled_;
		update();
        return true;
    }
	else if (key == GLFW_KEY_DOWN) {
		if (ao_enabled_) {
			float r = ao_->radius();
			if (r > 0) 
				ao_->set_radius(r - 0.1f);
			// make sure it is valid
			if (ao_->radius() <= 0)
				ao_->set_radius(0.1f);
			std::cout << "radius: " << ao_->radius() << std::endl;
			update();
		}
		return true;
	}
	else if (key == GLFW_KEY_UP) {
		if (ao_enabled_) {
			float r = ao_->radius();
			if (r > 0)
				ao_->set_radius(r + 0.1f);
			// make sure it is valid
			if (ao_->radius() >= 1.0f)
				ao_->set_radius(1.0f);
			std::cout << "radius: " << ao_->radius() << std::endl;
			update();
		}
		return true;
	}
    else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialAmbientOcclusion::draw() const {
    if (!current_model()) {
        return;
    }

    auto drawable = current_model()->renderer()->get_triangles_drawable("faces");
    auto faces = dynamic_cast<TrianglesDrawable*>(drawable);
    if (!faces)
        return;

	if (ao_enabled_) {
		ao_->generate(models_);

		const mat4& MVP = camera_->modelViewProjectionMatrix();
		// camera position is defined in world coordinate system.
		const vec3& wCamPos = camera_->position();
		// it can also be computed as follows:
		//const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
		const mat4& MV = camera_->modelViewMatrix();
		const vec4& wLightPos = inverse(MV) * setting::light_position;

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

		program->bind();
		program->bind();
		program->set_uniform("MVP", MVP)
			->set_uniform("wLightPos", wLightPos)
			->set_uniform("wCamPos", wCamPos)
			->set_uniform("ssaoEnabled", true)
			->bind_texture("ssaoTexture", ao_->ssao_texture(), 0);

        program->set_uniform("smooth_shading", faces->smooth_shading())
            ->set_block_uniform("Material", "ambient", faces->material().ambient)
            ->set_block_uniform("Material", "specular", faces->material().specular)
            ->set_block_uniform("Material", "shininess", &faces->material().shininess)
            ->set_uniform("per_vertex_color", faces->coloring_method() != State::UNIFORM_COLOR && drawable->color_buffer())
            ->set_uniform("default_color", faces->color());

        const auto& range = faces->highlight_range();
		program->set_uniform("hightlight_id_min", range.first)
			->set_uniform("hightlight_id_max", range.second);

        faces->gl_draw(false);

		program->release_texture();
		program->release();

        const int x = 20 * dpi_scaling();
        const int y = 40 * dpi_scaling();
        const int w = width() / 4 * dpi_scaling();
        const int h = height() / 4 * dpi_scaling();
        const Rect quad(x, x+w, y, y+h);
        opengl::draw_depth_texture(quad, ao_->ssao_texture(), width() * dpi_scaling(), height() * dpi_scaling(), -0.9f);
        opengl::draw_quad_wire(quad, vec4(1, 0,0, 1), width() * dpi_scaling(), height() * dpi_scaling(), -0.99f);
    }
	else
		Viewer::draw();
}


void TutorialAmbientOcclusion::cleanup() {
	if (ao_)
		delete ao_;

	Viewer::cleanup();
}
