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

#include "viewer.h"
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/setting.h>


using namespace easy3d;

TutorialEyeDomeLighting::TutorialEyeDomeLighting(const std::string& title) : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();

	edl_ = new EyeDomeLighting(camera());
	edl_enabled_ = true;

    usage_string_ =
            "---------- Eye Dome Lighting usage --------- \n"
            "Press key 'space' to toggle Eye Dome Lighting\n"
            "-------------------------------------------- \n";
}


TutorialEyeDomeLighting::~TutorialEyeDomeLighting() {
    delete edl_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}


bool TutorialEyeDomeLighting::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
		edl_enabled_ = !edl_enabled_;
		std::cout << "Eye Dome Lighting " << (edl_enabled_ ? "enabled" : "disabled") << std::endl;
		update();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialEyeDomeLighting::draw() const {
    if (!current_model()) {
        return;
    }

	if (edl_enabled_) {
		const mat4& MVP = camera_->modelViewProjectionMatrix();
		// camera position is defined in world coordinate system.
		const vec3& wCamPos = camera_->position();
		// it can also be computed as follows:
		//const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
		const mat4& MV = camera_->modelViewMatrix();
		const vec4& wLightPos = inverse(MV) * setting::light_position;

        ShaderProgram* program = ShaderManager::get_program("points/points_plain_color");
		if (!program) {
			std::vector<ShaderProgram::Attribute> attributes;
			attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
			attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
			attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("points/points_plain_color", attributes);
		}
		if (!program)
			return;

        auto drawable = current_model()->renderer()->get_points_drawable("vertices");
        float point_size = drawable->point_size();
        glPointSize(point_size);

        edl_->begin();
		program->bind();
		program->set_uniform("MVP", MVP);
		program->set_uniform("wLightPos", wLightPos);
		program->set_uniform("wCamPos", wCamPos);
		program->set_uniform("lighting", drawable->normal_buffer());
		program->set_uniform("per_vertex_color", drawable->coloring_method() != State::UNIFORM_COLOR && drawable->color_buffer());
		program->set_uniform("default_color", drawable->color());
        drawable->gl_draw();

		program->release();
		edl_->end();
	}
	else
		Viewer::draw();
}
