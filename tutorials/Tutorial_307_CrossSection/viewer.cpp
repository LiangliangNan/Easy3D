/** \cond LICENSE
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
 \endcond */

#include "viewer.h"
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>

namespace easy3d {

    CrossSection::CrossSection(const std::string &title) : Viewer(title) {
        // Code of interest: both backends use the shared clipping-plane controller.
        // ClippingPlane is a singleton: there is exactly one clipping plane per program,
        // and every drawable's shader consults it, so enabling it here clips all models.
        ClippingPlane::instance()->set_enabled(true);
        // hint_ is displayed in the viewer HUD; it documents the built-in Alt shortcuts
        // (toggle clipping, show/hide the plane guide, toggle cross-section mode).
        hint_ =
            "Left button:  rotate the clipping plane    \n" "Right button: translate the clipping plane \n" "Alt + 'c':    enable/disable clipping plane\n" "Alt + 'v':    show/hide clipping plane     \n" "Alt + 'x':    toggle cross-section";
    }

    // Route mouse drags to the plane's manipulator instead of the camera.
    bool CrossSection::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        // This tutorial manipulates the plane directly, without the Alt modifier
        // required by the default viewer controls.
        // The manipulator owns a ManipulatedFrame: a local 3D frame attached to the plane.
        // Its action_* helpers convert 2D mouse deltas (dx, dy) into 3D rotations/translations
        // expressed in camera space, so the plane moves intuitively from the current viewpoint.
        ManipulatedFrame *frame = ClippingPlane::instance()->manipulator()->frame();
        if (button == BUTTON_LEFT)
            frame->action_rotate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
        else if (button == BUTTON_RIGHT)
            frame->action_translate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
        else
            // Any other button (e.g. middle) keeps the default navigation behavior.
            return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
        return false;
    }

} // namespace easy3d
