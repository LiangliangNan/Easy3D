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
 * \example{lineno} Tutorial_401_ModelPicker/main.cpp
 *
 * \brief Selecting (and deselecting) one of several models by clicking on it with the mouse.
 *
 * <b>Overview</b>
 * This tutorial loads six letter meshes (spelling "easy3d") and lets the user click a letter
 * to toggle its selection, shown by coloring it red. It is the first of the interaction
 * tutorials: the pattern is to subclass Viewer, override a mouse event handler, and ask the
 * viewer what lies under the cursor. The same pattern is refined in Tutorial_402_FacePicker
 * (picking a single face) and reused in Tutorial_405_ObjectManipulation (picking, then moving
 * the picked model).
 *
 * <b>The technique</b>
 * "Picking" means mapping a 2D cursor position back to the 3D content the user sees. The two
 * classic approaches are:
 * - Ray casting: unproject the cursor into a 3D ray and intersect it with the geometry on the
 *   CPU. This is independent of rendering, but it must traverse the geometry and can disagree
 *   with what is actually displayed (hidden objects, transforms, clipping).
 * - GPU rasterization ("primitive-ID" or "color coding" picking): re-render the scene off
 *   screen, but instead of shading colors each primitive writes an encoded identifier into
 *   the color target. The depth test stays enabled, so the ID stored at a pixel belongs to
 *   the nearest visible primitive. Reading back that single pixel answers the query without
 *   any CPU geometry traversal, and it automatically respects visibility, model transforms,
 *   and clipping — because it uses the very same pipeline that produced the visible image.
 * Easy3D's Viewer::pick() implements the GPU approach for the whole scene in one call: it
 * returns the nearest model, drawable, and primitive under a pixel, together with the depth
 * and the world-space surface point. For model-level selection we only need the model field
 * of the result; the primitive/face fields are used in Tutorial_402_FacePicker.
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override mouse_press_event() to react to
 *   clicks. Unknown events are forwarded to the base class so navigation keeps working.
 * - Viewer::pick() / Viewer::PickResult: the scene-wide GPU picking query and its result
 *   (model, drawable, primitive, face index, position, depth).
 * - Renderer::set_selected(): marks a model as selected; TrianglesDrawable::set_uniform_coloring()
 *   paints the picked model red for visual feedback.
 * - Camera: the view is set up to look straight at the front of the logo.
 *
 * <b>Running it</b>
 * Left-click a letter to select it (it turns red and its name is printed to the console);
 * click it again to deselect. All other mouse actions (drag to rotate, etc.) still control
 * the camera because the handler forwards unhandled events to Viewer.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_401_ModelPicker/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_401_ModelPicker/viewer.cpp
 *
 * \see Viewer, Renderer, Tutorial_402_FacePicker, Tutorial_405_ObjectManipulation
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Our custom viewer: a Viewer subclass that turns left clicks into selection toggles.
    TutorialModelPicker viewer(EXAMPLE_TITLE);

    // Load the six letters of the "easy3d" logo. Each add_model() call also creates the
    // model's renderer with the default drawables ("faces", "edges", ...), which is what
    // the GPU picking pass later rasterizes to identify the model under the cursor.
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_e.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_a.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_s.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_y.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_3.ply");
    viewer.add_model(resource::directory() + "/data/easy3d/easy3d_d.ply");

    // add_model() returns nullptr on failure; if nothing loaded, there is nothing to pick.
    if (viewer.models().empty()) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
