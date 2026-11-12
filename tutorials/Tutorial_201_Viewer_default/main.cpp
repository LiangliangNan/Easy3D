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
#include <easy3d/viewer/viewer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_201_Viewer_default/main.cpp
 *
 * \brief Opening the default Easy3D Viewer to interactively inspect a 3D model.
 *
 * <b>Overview</b>
 * This is the first interactive tutorial: it loads a model from disk and hands it to the
 * built-in Viewer, which opens a window with a complete navigation interface (rotate, pan,
 * zoom, picking, snapshots, ...). Almost every graphical Easy3D example builds on this same
 * skeleton — initialize the library, create a Viewer, add one or more models, then call run().
 * The later tutorials (202-206) extend this viewer with custom GUI, multiple views, offscreen
 * rendering, and camera control.
 *
 * <b>The technique</b>
 * An interactive 3D viewer is driven by a render loop: the windowing system delivers input
 * events (mouse drags, key presses, resize), the viewer updates the camera and scene state in
 * response, and then redraws the framebuffer. Viewer::run() enters this loop and only returns
 * when the user closes the window. The Camera is the piece that maps the 3D scene onto the 2D
 * image: it holds a position and orientation (its Frame), a field of view, and near/far clipping
 * planes, and from these it builds the view and projection matrices used by the GPU. Here we
 * orient the camera before loading the model so the scene is presented from a known direction:
 * looking down the -Z axis with +Y up, i.e. a standard "front" view.
 *
 * <b>Easy3D API</b>
 * - initialize(): bootstraps the library (logging, resource paths, ...); call it once first.
 * - Viewer: the interactive window. add_model() loads a file and takes ownership of the model;
 *   run() starts the event/render loop.
 * - Camera: obtained via Viewer::camera(). set_view_direction() and set_up_vector() orient it.
 * - resource::directory(): the path to Easy3D's bundled sample data.
 *
 * <b>Running it</b>
 * A window opens showing the "easy3d" logo model. Drag with the left mouse button to rotate,
 * the right button (or wheel) to pan/zoom, and use the built-in shortcuts for snapshots and
 * display options. Close the window to exit; run() then returns and the program ends.
 *
 * \see Viewer, Camera, Tutorial_202_Viewer_imgui, Tutorial_203_MultiView
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A bundled sample model shipped with Easy3D's resource data.
    const std::string file_name = resource::directory() + "/data/easy3d.ply";

    // Create the default interactive viewer. EXAMPLE_TITLE is set by the build system.
    Viewer viewer(EXAMPLE_TITLE);

    // Orient the camera to a standard front view: look along -Z with +Y pointing up.
    viewer.camera()->set_view_direction(vec3(0, 0, -1));
    viewer.camera()->set_up_vector(vec3(0, 1, 0));

    // Load the model into the viewer (which takes ownership). add_model() returns nullptr
    // on failure, so we bail out with an error message if the file is missing or invalid.
    if (!viewer.add_model(file_name)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive render loop; returns only when the window is closed.
    return viewer.run();
}
