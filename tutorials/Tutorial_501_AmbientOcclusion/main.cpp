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
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_501_AmbientOcclusion/main.cpp
 *
 * \brief Rendering a surface mesh with screen-space ambient occlusion (AO) to improve depth perception.
 *
 * <b>Overview</b>
 * This tutorial adds ambient occlusion to the standard Easy3D Viewer. The pattern is the one
 * used by most rendering-effect tutorials: subclass Viewer, own an effect object, and wrap the
 * base-class scene draw() with the effect's begin()/end() pair. The entry point below simply
 * loads a model, gives it a neutral white material, and runs the custom viewer; all of the
 * effect logic lives in the TutorialAmbientOcclusion class (shown after this file).
 *
 * <b>The technique</b>
 * Ambient occlusion approximates how much diffuse ambient light reaches each surface point.
 * A point sitting in a crease, corner, or contact region is surrounded by nearby geometry that
 * blocks part of the incoming hemisphere, so it receives less light and should look darker;
 * a point on an exposed convex region receives almost full ambient light. Darkening occluded
 * regions gives a strong cue of shape, depth, and contact — even with no explicit light source.
 * Easy3D computes this in screen space (a GTAO-style pass): the scene depth is rendered first,
 * then a post-process samples the neighborhood of each pixel in the depth buffer to estimate
 * how much of the hemisphere is blocked, and multiplies the shaded color by the resulting
 * per-pixel occlusion factor. The "radius" parameter sets the world-space extent of that
 * neighborhood: a larger radius yields broader, softer occlusion; a smaller radius emphasizes
 * fine creases. See the AmbientOcclusion class for the implementation.
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override draw() (to insert the effect) and
 *   key_press_event() (to control it).
 * - AmbientOcclusion: the effect. begin()/end() bracket the scene render; radius()/set_radius()
 *   tune the sampling extent.
 * - DrawableTriangles: the surface drawable; set_uniform_coloring() assigns a constant material
 *   color so that the shading variation comes purely from the occlusion, not from texture.
 *
 * <b>Running it</b>
 * Press 'space' to toggle ambient occlusion on/off and compare the depth cues; press 'up'/'down'
 * to increase/decrease the occlusion radius (the current value is printed to the console).
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_501_AmbientOcclusion/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_501_AmbientOcclusion/viewer.cpp
 *
 * \see AmbientOcclusion, Viewer, DrawableTriangles
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The model to render. general_open.obj is an open surface whose folds and contacts
    // make the occlusion darkening easy to see.
    const std::string file = resource::directory() + "/data/general_open.obj";

    // Create our custom viewer (a Viewer subclass that applies ambient occlusion).
    TutorialAmbientOcclusion viewer(EXAMPLE_TITLE);

    // Load the model and add it to the viewer. add_model() creates the model's renderer;
    // the second argument (true) centers and scales the model to fit the scene.
    auto model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Give the surface a neutral white material so the only shading variation is the
    // ambient occlusion. "faces" is the standard triangles drawable of a surface mesh.
    auto drawable = model->renderer()->get_triangles_drawable("faces");
    drawable->set_uniform_coloring(vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}