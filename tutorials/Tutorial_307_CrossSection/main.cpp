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

#include <easy3d/renderer/camera.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

#include "viewer.h"

/**
 * \example{lineno} Tutorial_307_CrossSection/main.cpp
 *
 * \brief Inspecting a model's interior with an interactively movable clipping plane and cross-sections.
 *
 * <b>Overview</b>
 * This tutorial shows how to cut a model open while rendering it: everything on one side of a
 * plane is hidden, so you can look inside the geometry. Easy3D ships a ready-made, manipulable
 * ClippingPlane; the custom CrossSection viewer below simply enables it and re-routes mouse
 * drags so the plane can be moved directly (without holding a modifier key). The entry point
 * loads the Stanford bunny and picks a viewing direction that makes the cut easy to see.
 *
 * <b>The technique</b>
 * A plane splits space into two half-spaces: for a plane with normal n and offset d, the sign
 * of dot(n, p) + d tells on which side a point p lies. Clipping evaluates this test per fragment
 * in the shader and discards fragments on the forbidden side — the mesh itself is never
 * modified, so the cut can be moved interactively at no geometry-processing cost. A
 * cross-section generalizes this to a thin slab between two parallel planes (plane0() and
 * plane1() in ClippingPlane): only fragments inside the slab survive, revealing a slice of the
 * model, like a CT scan slice. The slab thickness is defined relative to the scene size. A
 * translucent guide quad is drawn to show where the plane is, and a ManipulatedFrame (a local
 * coordinate frame with rotate/translate actions driven by mouse deltas) lets the user reorient
 * and slide the plane through the model.
 *
 * <b>Easy3D API</b>
 * - ClippingPlane: a program-wide singleton (instance()) controlling clipping; set_enabled()
 *   turns it on, and its built-in viewer shortcuts toggle visibility and cross-section mode.
 * - Manipulator / ManipulatedFrame: the interaction machinery; frame()->action_rotate() and
 *   action_translate() convert 2D mouse motion into 3D plane motion using the Camera.
 * - Viewer: subclassed here to override mouse_drag_event(); camera()->setViewDirection() aims
 *   the initial view.
 *
 * <b>Running it</b>
 * Drag with the left button to rotate the clipping plane, drag with the right button to
 * translate it. Alt+'c' enables/disables clipping, Alt+'v' shows/hides the plane guide, and
 * Alt+'x' toggles cross-section (slab) mode.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_307_CrossSection/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_307_CrossSection/viewer.cpp
 *
 * \see ClippingPlane, ManipulatedFrame, Viewer, Camera
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // The bunny is a closed surface, so clipping it reveals a hollow interior — a good
    // demonstration of what the plane cuts away.
    const std::string file_name = resource::directory() + "/data/bunny.ply";
    // Our viewer subclass enables the clipping plane in its constructor.
    CrossSection viewer(EXAMPLE_TITLE);

    // Look at the bunny from an oblique direction so the cut surface is visible.
    viewer.camera()->setViewDirection(vec3(-1, 1, -1));

    if (!viewer.add_model(file_name)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
