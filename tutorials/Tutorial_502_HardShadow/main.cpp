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
 * \example{lineno} Tutorial_502_HardShadow/main.cpp
 *
 * \brief Rendering a scene with hard shadows using the shadow mapping technique.
 *
 * <b>Overview</b>
 * This tutorial adds hard shadows to the standard Easy3D Viewer. It follows the same pattern
 * as the other rendering-effect tutorials (e.g. Tutorial_501_AmbientOcclusion): subclass Viewer,
 * own an effect object, and wrap the base-class scene draw() with the effect's begin()/end()
 * pair. The entry point below loads an indoor scene (room.obj), gives it a neutral gray material,
 * and runs the custom viewer; all shadow logic lives in the TutorialHardShadow class (shown
 * after this file). Compare with Tutorial_503_SoftShadow, which uses the same scene but produces
 * physically more plausible soft shadow edges.
 *
 * <b>The technique</b>
 * Shadow mapping (Lance Williams, "Casting curved shadows on curved surfaces", SIGGRAPH 1978)
 * answers the visibility question "can the light see this point?" with a depth-buffer trick.
 * In a first pass the scene is rendered from the light's point of view into a depth texture
 * (the shadow map), which records the distance of the nearest surface to the light along every
 * light ray. In the normal camera pass, each surface point is re-projected into the light's
 * clip space and its depth is compared against the stored value: if the point is farther from
 * the light than the recorded nearest depth, something blocks the light and the point lies in
 * shadow. Because the test is a single binary comparison per pixel, the resulting shadow edges
 * are razor sharp ("hard") and show aliasing at the shadow-map resolution — real light sources
 * have area and produce soft penumbras (see Tutorial_503_SoftShadow). Easy3D's Shadow class also
 * supports a virtual background plane, perpendicular to the light direction at the far plane
 * of the light frustum, that acts as an extra shadow receiver so the object casts a visible
 * ground shadow even when the model has no floor.
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override draw() (to insert the effect) and
 *   key_press_event() (to control it).
 * - Shadow: the hard-shadow effect. begin(surfaces) registers the shadow-casting surfaces for
 *   the frame, end() finishes the registration; the shadow pass itself is applied to the scene
 *   render between the two calls. set_virtual_background_color() enables and colors the virtual
 *   receiver plane.
 * - DrawableTriangles: the surface drawable; set_uniform_coloring() assigns a constant material
 *   color and set_smooth_shading(true) interpolates normals, so shadowing is easy to read.
 *
 * <b>Running it</b>
 * Press 'space' to toggle shadowing on/off and compare. Use the standard Viewer navigation
 * (mouse drag to rotate, wheel to zoom) to inspect the shadow from different angles.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_502_HardShadow/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_502_HardShadow/viewer.cpp
 *
 * \see Shadow, Viewer, DrawableTriangles, Tutorial_503_SoftShadow
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The scene to render. room.obj contains furniture above a floor, so the objects
    // cast clearly readable shadows onto the ground.
    const std::string file = resource::directory() + "/data/room.obj";

    // Create our custom viewer (a Viewer subclass that applies hard shadows).
    TutorialHardShadow viewer(EXAMPLE_TITLE);

    // Load the model and add it to the viewer. add_model() creates the model's renderer;
    // the second argument (true) centers and scales the model to fit the scene.
    auto model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Give the surface a neutral light-gray material with smooth (interpolated-normal)
    // shading so the darkened shadow regions stand out. "faces" is the standard triangles
    // drawable of a surface mesh.
    auto drawable = model->renderer()->get_triangles_drawable("faces");
    drawable->set_uniform_coloring(vec4(0.9f, 0.9f, 0.9f, 1.0f));
    drawable->set_smooth_shading(true);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
