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
 * \example{lineno} Tutorial_503_SoftShadow/main.cpp
 *
 * \brief Rendering a scene with soft shadows using Percentage-Closer Soft Shadows (PCSS).
 *
 * <b>Overview</b>
 * This tutorial upgrades the hard shadow mapping of Tutorial_502_HardShadow to soft shadows.
 * The program structure is identical — subclass Viewer, own an effect object, wrap the scene
 * draw() with begin()/end() — only the effect class changes from Shadow to SoftShadow (a
 * subclass of Shadow). The entry point loads the same indoor scene (room.obj) with a neutral
 * gray material; all shadow logic lives in the TutorialSoftShadow class (shown after this file).
 *
 * <b>The technique</b>
 * Real light sources have a finite area, so shadow boundaries are not sharp: near a shadow
 * edge there is a penumbra where the receiver sees only part of the light. Percentage-Closer
 * Soft Shadows (Randima Fernando, GPU Gems, 2005) approximates this on top of a standard
 * shadow map in three steps per pixel: (1) blocker search — sample a neighborhood of the
 * shadow map to estimate the average depth of the occluders between the light and the
 * receiver; (2) penumbra estimation — from similar triangles, the farther the receiver is
 * from the blocker, the wider the penumbra, which sets the filter kernel size; (3) Percentage-
 * Closer Filtering (PCF) — average many binary depth comparisons over that kernel to get a
 * fractional visibility. The result is shadows that are crisp where an object touches the
 * floor and grow softer with distance ("contact hardening"), which is a strong realism cue.
 * Easy3D exposes a softness parameter (scales the simulated light size) and several sample
 * patterns trading quality against speed. See the SoftShadow class for the implementation
 * and links to the original NVIDIA paper.
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override draw() (to insert the effect) and
 *   key_press_event() (to control it).
 * - SoftShadow: the PCSS effect, derived from Shadow, so it uses the same begin(surfaces)/end()
 *   protocol and the same virtual-background receiver plane. set_softness() and
 *   set_sample_pattern() tune quality (not used here; defaults apply).
 * - DrawableTriangles: the surface drawable; set_uniform_coloring() assigns a constant material
 *   color and set_smooth_shading(true) interpolates normals, so the penumbra gradient is easy
 *   to read.
 *
 * <b>Running it</b>
 * Press 'space' to toggle shadowing on/off. Look at where objects meet the floor: the shadow
 * is sharp at contact and softens with distance — compare with the uniformly hard edges of
 * Tutorial_502_HardShadow. Standard Viewer navigation (mouse drag, wheel) applies.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_503_SoftShadow/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_503_SoftShadow/viewer.cpp
 *
 * \see SoftShadow, Shadow, Viewer, DrawableTriangles, Tutorial_502_HardShadow
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The same scene as Tutorial_502_HardShadow: furniture above a floor, so the
    // contact-hardening behavior of the soft shadows is easy to observe.
    const std::string file = resource::directory() + "/data/room.obj";

    // Create our custom viewer (a Viewer subclass that applies PCSS soft shadows).
    TutorialSoftShadow viewer(EXAMPLE_TITLE);

    // Load the model and add it to the viewer. add_model() creates the model's renderer;
    // the second argument (true) centers and scales the model to fit the scene.
    auto model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Give the surface a neutral light-gray material with smooth (interpolated-normal)
    // shading so the soft penumbra gradients stand out. "faces" is the standard triangles
    // drawable of a surface mesh.
    auto drawable = model->renderer()->get_triangles_drawable("faces");
    drawable->set_uniform_coloring(vec4(0.9f, 0.9f, 0.9f, 1.0f));
    drawable->set_smooth_shading(true);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
