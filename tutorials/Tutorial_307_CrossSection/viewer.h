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

#ifndef EASY3D_TUTORIAL_CROSS_SECTION_H
#define EASY3D_TUTORIAL_CROSS_SECTION_H

#include "easy3d/viewer/viewer.h"


/**
 * \brief A Viewer with a directly manipulable clipping plane for inspecting model interiors.
 *
 * Easy3D's Viewer already knows how to render with the global ClippingPlane and provides
 * Alt-based shortcuts for it. This subclass makes the interaction more immediate: the
 * constructor enables the plane, and mouse_drag_event() is overridden so plain left/right
 * drags rotate/translate the plane itself instead of the camera.
 */
class CrossSection : public easy3d::Viewer {
public:
    // Enables the clipping plane and installs the on-screen key hints.
    CrossSection(const std::string &title);

protected:
    // Left drag rotates the plane, right drag translates it; other buttons keep the
    // default viewer behavior (see viewer.cpp).
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
};


#endif // EASY3D_TUTORIAL_CROSS_SECTION_H