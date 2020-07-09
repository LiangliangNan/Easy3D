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

#include "hard_shadow.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

TutorialHardShadow::TutorialHardShadow(const std::string& title) : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, 1));

	shadow_ = new Shadow(camera());
    shadow_->set_background_color(background_color());
	shadow_enabled_ = true;
}


std::string TutorialHardShadow::usage() const {
    return ("------------------- Hard Shadow usage ------------------- \n"
            "Press key 'space' to switch between Shadowing and normal rendering\n"
            "--------------------------------------------------------- \n");
}


bool TutorialHardShadow::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
		shadow_enabled_ = !shadow_enabled_;
		update();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialHardShadow::draw() const {
    if (!current_model()) {
        return;
    }

	std::vector<TrianglesDrawable*> surfaces;
	for (auto m : models_) {
        for (auto d : m->renderer()->triangles_drawables()) {
            surfaces.push_back(d);
        }
	}
	if (shadow_enabled_)
		shadow_->draw(surfaces);
	else
		Viewer::draw();
}


void TutorialHardShadow::cleanup() {
	if (shadow_)
		delete shadow_;

	Viewer::cleanup();
}
