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
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>


namespace easy3d {

    CrossSection::CrossSection(const std::string &title) : Viewer(title) {
        // Code of interest:
        ClippingPlane::instance()->set_enabled(true);
    }

    CrossSection::~CrossSection() {
        // Disable it (otherwise it affects the visualization in subsequent tests)
        ClippingPlane::instance()->set_enabled(false);
    }

    void CrossSection::post_draw() {
        // Code of interest:
        ClippingPlane::instance()->draw(camera());

        Viewer::post_draw();
    }

    bool CrossSection::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        ManipulatedFrame* frame = ClippingPlane::instance()->manipulator()->frame();

        if (pressed_button_ == BUTTON_LEFT)
            frame->action_rotate(x, y, dx, dy, camera_, ManipulatedFrame::NONE);
        else if (pressed_button_ == BUTTON_RIGHT)
            frame->action_translate(x, y, dx, dy, camera_, ManipulatedFrame::NONE);
        else
            return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);

        return false;
    }

}
