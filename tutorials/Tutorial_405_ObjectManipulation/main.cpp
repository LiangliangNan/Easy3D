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
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_405_ObjectManipulation/main.cpp
 *
 * \brief Moving and rotating an individual model in 3D with the mouse, using the Manipulator class.
 *
 * <b>Overview</b>
 * This tutorial combines picking (Tutorial_401_ModelPicker) with interactive object
 * manipulation: click a letter of the "easy3d" logo to select it, then drag with Alt held to
 * rotate or translate that model while the camera stays put. It is the standard "select and
 * transform" interaction of 3D editors and games, and the same machinery drives the
 * interactive collision demo in Tutorial_406_CollisionDetection.
 *
 * <b>The technique</b>
 * The core idea is a manipulated frame: a local coordinate frame attached to the object
 * (origin at the object center) whose rotation and translation accumulate the user's drag
 * gestures. The frame's matrix becomes the model's transform, so the geometry itself is
 * never edited — moving an object is just changing one 4x4 matrix. Mouse motion is converted
 * to frame motion by the classic trackball technique (see e.g. Chen et al., "A Study in
 * Interactive 3-D Rotation Using 2-D Control Devices", 1988): a 2D drag is mapped onto a
 * virtual sphere surrounding the object, giving intuitive free rotation; translation drags
 * the object in the screen-parallel plane. Holding 'X', 'Y', or 'O' during a drag constrains
 * the motion to a horizontal, vertical, or view-orthogonal screen axis. The action_start() /
 * action_end() pair brackets a drag so the frame can snapshot its state and apply incremental
 * updates consistently. Which object receives the gestures is decided by GPU picking
 * (Viewer::pick(), see Tutorial_401_ModelPicker), and a visual gizmo (the manipulated frame's
 * axes) is drawn as a screen overlay so the user sees what is being manipulated.
 *
 * <b>Easy3D API</b>
 * - Manipulator / ManipulatedFrame: attach with Model::set_manipulator(); frame() exposes
 *   action_start(), action_rotate(), action_translate(), action_end(), and the ScreenAxis
 *   constraints (NONE / HORIZONTAL / VERTICAL / ORTHOGONAL).
 * - Viewer::pick(): selects the model under the cursor (GPU primitive-ID picking).
 * - Manipulator::draw_frame() + Viewer::set_overlays(): render the manipulation gizmo.
 * - Renderer::set_selected(): tracks which model is currently selected.
 *
 * <b>Running it</b>
 * Left-click a model to select it (click again to deselect). With a model selected:
 * Alt + left-drag rotates it, Alt + right-drag translates it; hold 'X'/'Y'/'O' during the
 * drag to constrain the motion to a screen axis. Without Alt, the mouse controls the camera
 * as usual.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_405_ObjectManipulation/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_405_ObjectManipulation/viewer.cpp
 *
 * \see Manipulator, ManipulatedFrame, Viewer, Tutorial_401_ModelPicker, Tutorial_406_CollisionDetection
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Our custom viewer: a Viewer subclass that lets the user pick and then move a model.
    TutorialObjectManipulation viewer(EXAMPLE_TITLE);

    // Load the six letters of the "easy3d" logo; each becomes independently manipulable.
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_e.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_a.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_s.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_y.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_3.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_d.ply");

    // add_model() returns nullptr on failure; if nothing loaded, there is nothing to manipulate.
    if (viewer.models().empty()) {
        std::cerr << "failed to load the model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
