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
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_505_EyeDomeLighting/main.cpp
 *
 * \brief Rendering a point cloud without normals using Eye Dome Lighting (EDL) for depth perception.
 *
 * <b>Overview</b>
 * This tutorial renders a raw point cloud that has no normal information, yet still conveys
 * shape and depth clearly, by adding Eye Dome Lighting to the standard Easy3D Viewer. It uses
 * the familiar effect pattern (see Tutorial_501_AmbientOcclusion): subclass Viewer, own an
 * effect object, and wrap the base-class scene draw() with the effect's begin()/end() pair.
 * The entry point loads the bunny point cloud, gives it a uniform color and a visible point
 * size, and runs the custom viewer; the EDL logic lives in the TutorialEyeDomeLighting class
 * (shown after this file).
 *
 * <b>The technique</b>
 * A point cloud has no connectivity and often no normals, so classical lighting (Lambert,
 * Phong) cannot be evaluated — a flat-colored cloud looks like a confusing blob of dots.
 * Eye Dome Lighting (Boucheny, 2009; popularized by CloudCompare and Potree) is an image-space
 * shading trick that needs only the depth buffer: the cloud is first rendered normally into
 * color and depth buffers, then a full-screen post-process compares each pixel's depth with
 * the depths of a few neighbors at a small screen-space offset. If the neighbors are
 * consistently closer to the camera, the pixel lies on the "far side" of a depth
 * discontinuity (a silhouette or crease) and is darkened; pixels on flat, front-facing
 * regions stay bright. The darkened contours act like a shading cue and make silhouettes,
 * overlaps, and surface relief pop out, even though no surface or normal was ever computed.
 * It is cheap (one extra pass), works on any point set, and complements ambient occlusion
 * for meshes (Tutorial_501). See the EyeDomeLighting class for the implementation.
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override draw() (to insert the effect)
 *   and key_press_event() (to control it).
 * - EyeDomeLighting: the effect. begin()/end() bracket the scene render; the pass reads the
 *   depth produced between them.
 * - PointCloud / DrawablePoints: the data and its points drawable; set_uniform_coloring()
 *   gives all points one color (so all shading variation comes from EDL) and set_point_size()
 *   makes the splats large enough to cover the surface.
 *
 * <b>Running it</b>
 * Press 'space' to toggle Eye Dome Lighting on/off (the state is printed to the console) and
 * compare how much easier the shape is to read with the contours darkened. Standard Viewer
 * navigation (mouse drag to rotate, wheel to zoom) applies.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_505_EyeDomeLighting/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_505_EyeDomeLighting/viewer.cpp
 *
 * \see EyeDomeLighting, Viewer, PointCloud, DrawablePoints, Tutorial_501_AmbientOcclusion
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The Stanford bunny as a raw point cloud (Easy3D's binary format): positions only,
    // no connectivity and no normals — exactly the case EDL is designed for.
    const std::string file = resource::directory() + "/data/bunny.bin";

    // Create our custom viewer (a Viewer subclass that applies Eye Dome Lighting).
    TutorialEyeDomeLighting viewer(EXAMPLE_TITLE);

    // Load the point cloud and add it to the viewer. add_model() creates the model's
    // renderer; the second argument (true) centers and scales the model to fit the scene.
    auto model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // "vertices" is the standard points drawable of a point cloud. A single uniform color
    // means all shading variation comes from EDL, and 5-pixel splats are large enough to
    // fill the surface so the depth buffer has few holes for the EDL pass to sample.
    auto drawable = model->renderer()->get_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(0.6f, 0.6f, 1.0f, 1.0f));
    drawable->set_point_size(5.0f);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
