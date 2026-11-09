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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_504_Transparency/main.cpp
 *
 * \brief Rendering a surface mesh with order-independent transparency (OIT).
 *
 * <b>Overview</b>
 * This tutorial shows how to render a semi-transparent surface mesh so that all overlapping
 * layers remain visible and correctly composited. It renders a torus knot — a shape whose
 * tube overlaps itself many times from most viewpoints, which makes incorrect transparency
 * obvious. The custom viewer lets you cycle interactively between the techniques implemented
 * in Easy3D: Average Color Blending, Dual Depth Peeling, and Per Pixel Linked Lists OIT (plus
 * a "disabled" mode for comparison). The entry point below loads the model and configures its
 * material; the technique switching lives in the TutorialTransparency class (shown after this
 * file).
 *
 * <b>The technique</b>
 * Standard alpha blending is order dependent: fragments must be composited back-to-front for
 * the "over" operator to give the right color, but the GPU rasterizes triangles in submission
 * order and the depth buffer keeps only the nearest fragment. Order-independent transparency
 * (OIT) removes the need to sort. This tutorial demonstrates three approaches:
 * - Average Color Blending (Meshkin's method): accumulate the colors and the alphas of all
 *   transparent fragments per pixel in a single geometry pass, then blend their weighted
 *   average over the opaque scene. Fast and order independent, but it loses depth ordering,
 *   so overlapping layers look "averaged" rather than stacked.
 * - Depth peeling (Everitt 2001; dual depth peeling by Bavoil & Myers, NVIDIA 2008): extract
 *   the transparency layers one (or, with the dual min-max trick, two) at a time using the
 *   previous layer's depth as a clip boundary, and composite them in correct order. The
 *   result converges to exact transparency as the number of peels approaches the actual
 *   number of depth layers; Easy3D defaults to a small fixed number of peels for speed.
 * - Per-pixel linked lists (Yang et al. 2010): a build pass appends every transparent
 *   fragment to a per-pixel linked list in GPU memory using atomic operations; a resolve
 *   pass sorts each pixel's fragments by depth and composites them exactly. Perfect quality
 *   in one geometry pass, at the cost of large, scene-dependent GPU memory. Note this method
 *   needs OpenGL 4.3+ (unavailable on macOS's OpenGL 4.1; use the Vulkan backend or Dual
 *   Depth Peeling there).
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override draw() and key_press_event().
 * - Transparency: abstract base of the OIT effects. Unlike the shadow/AO effects, its
 *   interface is a single draw(surfaces) call that takes over rendering of the surfaces.
 * - AverageColorBlending, DualDepthPeeling, PerPixelLinkedListsOIT: the concrete techniques,
 *   all constructed with the viewer's Camera and interchangeable through the base pointer.
 * - DrawableTriangles: set_opacity() controls the material alpha interactively;
 *   set_distinct_back_color(false) gives front and back faces the same color.
 *
 * <b>Running it</b>
 * Press 'space' to cycle through the methods (disabled → Average Color Blending → Dual Depth
 * Peeling → Per Pixel Linked Lists); the active method is printed and shown in the usage
 * help. Press 'up'/'down' to increase/decrease the surface opacity. Rotate the torus knot and
 * compare how each method handles the self-overlapping tube.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_504_Transparency/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_504_Transparency/viewer.cpp
 *
 * \see Transparency, AverageColorBlending, DualDepthPeeling, PerPixelLinkedListsOIT, Viewer, DrawableTriangles
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A torus knot: its tube passes over and under itself, producing many transparency
    // layers per pixel — an ideal stress test for order-independent transparency.
    const std::string file_name = resource::directory() + "/data/torusknot.obj";

    // Create our custom viewer (a Viewer subclass that applies an OIT technique).
    TutorialTransparency viewer(EXAMPLE_TITLE);

    // Load the mesh and add it to the viewer. add_model() creates the model's renderer
    // (including the "faces" drawable); true centers and scales the model to fit the scene.
    if (!viewer.add_model(file_name, true)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Configure the material: smooth shading for a clean look, a uniform color, and the
    // same color for front and back faces (a distinct back color would confuse the
    // comparison of transparency methods). The actual opacity is adjusted interactively
    // with the 'up'/'down' keys inside the viewer.
    auto drawable = viewer.current_model()->renderer()->get_triangles_drawable("faces");
    drawable->set_smooth_shading(true);
    drawable->set_coloring_method(State::UNIFORM_COLOR);
    drawable->set_distinct_back_color(false);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
