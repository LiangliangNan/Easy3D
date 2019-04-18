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

#include "eye_dome_lighting.h"
#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/eye_dome_lighting.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialEyeDomeLighting::TutorialEyeDomeLighting(const std::string& title) : Viewer(title) {
	camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();

	edl_ = new EyeDomeLighting(camera());
	edl_enabled_ = true;

    std::cout << "------------ Eye Dome Lighting ----------" << std::endl
              << "Press key 'space' to switch between Eye Dome Lighting and normal rendering" << std::endl;
}


bool TutorialEyeDomeLighting::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
		edl_enabled_ = !edl_enabled_;
		std::cout << "Eye Dome Lighting " << (edl_enabled_ ? "enabled" : "disabled") << std::endl;
		update();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialEyeDomeLighting::draw() {
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

		ShaderProgram* program = ShaderManager::get_program("points_color");
		if (!program) {
			std::vector<ShaderProgram::Attribute> attributes;
			attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
			attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
			attributes.push_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
			program = ShaderManager::create_program_from_files("points_color", attributes);
		}
		if (!program)
			return;

		edl_->begin();
		program->bind();
		program->set_uniform("MVP", MVP);
		program->set_uniform("wLightPos", wLightPos);
		program->set_uniform("wCamPos", wCamPos);

		auto drawable = current_model()->points_drawable("vertices");
		program->set_uniform("lighting", drawable->normal_buffer());
		program->set_uniform("per_vertex_color", drawable->per_vertex_color() && drawable->color_buffer());
		program->set_uniform("default_color", drawable->default_color());
		drawable->draw(false);

		program->release();
		edl_->end();
	}
	else
		Viewer::draw();
}


void TutorialEyeDomeLighting::cleanup() {
	if (edl_)
		delete edl_;

	Viewer::cleanup();
}
