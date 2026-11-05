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
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/eye_dome_lighting.h>


using namespace easy3d;

// \cond

TutorialEyeDomeLighting::TutorialEyeDomeLighting(const std::string& title) : Viewer(title) {
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();

	edl_ = new EyeDomeLighting(camera());
	edl_enabled_ = true;

    hint_ = "Press 'space' to toggle Eye Dome Lighting";
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


bool TutorialEyeDomeLighting::draw() {
    if (!current_model()) {
        return false;
    }

    if (edl_enabled_) {
        // Both backends render the scene inside begin()/end(): OpenGL into the
        // effect's offscreen target, Vulkan into the viewer frame that the effect
        // shades before presentation.
        edl_->begin();
        const bool ok = Viewer::draw();
        edl_->end();
        return ok;
    }
    else
        return Viewer::draw();
}

// \endcond