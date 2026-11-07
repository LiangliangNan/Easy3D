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
#include <easy3d/core/model.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/per_pixel_linked_lists_oit.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>


using namespace easy3d;

// \cond

TutorialTransparency::TutorialTransparency(const std::string& title) : Viewer(title) {
	transparency_ = new DualDepthPeeling(camera());
	method_ = 2;
    hint_string_ = "Press 'space' to switch between different transparency techniques\n"
				   "Press 'up/down' to increase/decrease the transparency\n\nMethod: ";
	Viewer::set_usage("default", hint_string_ + "Dual Depth Peeling");
}


TutorialTransparency::~TutorialTransparency() {
    delete transparency_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}


bool TutorialTransparency::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
		if (transparency_) {
			delete transparency_;
			transparency_ = nullptr;
		}

		method_ = (method_ + 1) % 4;
		switch (method_)
		{
		case 0:
			set_usage("default", hint_string_ + "NULL (transparency disabled)");
			std::cout << "Method: NULL (transparency disabled)" << std::endl;
			break;
		case 1:
			transparency_ = new AverageColorBlending(camera());
			set_usage("default", hint_string_ + "Average Color Blending");
			std::cout << "Method: Average Color Blending" << std::endl;
			break;
		case 2:
			transparency_ = new DualDepthPeeling(camera());
			set_usage("default", hint_string_ + "Dual Depth Peeling");
			std::cout << "Method: Dual Depth Peeling" << std::endl;
			break;
		case 3:
			transparency_ = new PerPixelLinkedListsOIT(camera());
#if defined(__APPLE__) && !defined(EASY3D_HAS_VULKAN)
			set_usage("default", hint_string_ + "Per Pixel Linked Lists OIT - NOT available on macOS with OpenGL");
			std::cerr << "PerPixelLinkedListsOIT: requires OpenGL 4.3+, current version is 4.1. macOS OpenGL is limited to 4.1. Use the Vulkan backend or DualDepthPeeling for transparency. " << std::endl;;
#else
			set_usage("default", hint_string_ + "Per Pixel Linked Lists OIT");
			std::cout << "Method: Per Pixel Linked Lists OIT" << std::endl;
#endif
			break;
		}
		update();
        return true;
    }
    else if (key == KEY_DOWN) {
        auto faces = current_model()->renderer()->get_triangles_drawable("faces");
        if (faces) {
            float o = faces->opacity();
            if (o > 0)
                faces->set_opacity(o - 0.1f);
            // make sure it is valid
            if (faces->opacity() <= 0)
                faces->set_opacity(0.1f);
            std::cout << "opacity: " << faces->opacity() << std::endl;
            update();
        }
        return true;
    }
    else if (key == KEY_UP) {
        auto faces = current_model()->renderer()->get_triangles_drawable("faces");
        if (faces) {
            float o = faces->opacity();
            if (o > 0)
                faces->set_opacity(o + 0.1f);
            // make sure it is valid
            if (faces->opacity() >= 1.0f)
                faces->set_opacity(1.0f);
            std::cout << "opacity: " << faces->opacity() << std::endl;
            update();
        }
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


bool TutorialTransparency::draw() {
    if (!current_model()) {
        return false;
    }

    std::vector<TrianglesDrawable*> surfaces;
    for (auto m : models_) {
        for (auto d : m->renderer()->triangles_drawables()) {
            if (d->type() == Drawable::DT_TRIANGLES)
                    surfaces.push_back(dynamic_cast<TrianglesDrawable*>(d.get()));
        }
    }

	if (method_ != 0)
		transparency_->draw(surfaces);
	else
		return Viewer::draw();
	return true;
}

// \endcond