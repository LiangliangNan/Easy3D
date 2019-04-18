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

#include "transparency.h"
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/average_color_blending.h>
#include <easy3d/viewer/dual_depth_peeling.h>
#include <easy3d/viewer/camera.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialTransparency::TutorialTransparency(const std::string& title) : Viewer(title) {
	camera()->setViewDirection(vec3(0, 0, -1));
    camera()->showEntireScene();

	transparency_ = new AverageColorBlending(camera());
	std::cout << "method: Average Color Blending" << std::endl;
	method_ = 1;

	std::cout << "------------ TutorialTransparency ----------" << std::endl
		<< "Press key 'space' to turn on/off or switch between different transparency techniques" << std::endl;
}


bool TutorialTransparency::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
		if (transparency_) {
			delete transparency_;
			transparency_ = nullptr;
		}

		method_ = (method_ + 1) % 3;
		switch (method_)
		{
		case 0:
			std::cout << "method: NULL(transparency disabled)" << std::endl;
			break;
		case 1:
			transparency_ = new AverageColorBlending(camera());
			std::cout << "method: Average Color Blending" << std::endl;
			break;
		case 2:
			transparency_ = new DualDepthPeeling(camera());
			std::cout << "method: Dual Depth Peeling" << std::endl;
			break;
		}
		update();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialTransparency::draw() {
    if (!current_model()) {
        return;
    }

	std::vector<TrianglesDrawable*> surfaces;
	for (auto m : models_) {
		for (auto d : m->triangles_drawables())
			surfaces.push_back(d);
	}
	if (method_ != 0)
		transparency_->draw(surfaces);
	else
		Viewer::draw();
}


void TutorialTransparency::cleanup() {
	if (transparency_)
		delete transparency_;

	Viewer::cleanup();
}
