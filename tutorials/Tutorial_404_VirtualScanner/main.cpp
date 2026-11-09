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
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_404_VirtualScanner/main.cpp
 *
 * \brief Simulating a 3D scanner: capturing the currently visible surface of a model as a point cloud.
 *
 * <b>Overview</b>
 * This tutorial turns the viewer into a virtual range scanner. The user navigates to a
 * viewpoint and presses 'space'; everything visible from that viewpoint (and only that) is
 * captured as a new PointCloud and added to the scene. Repeating this from different
 * viewpoints mimics acquiring multiple scans of a real object, the raw input of a typical
 * 3D reconstruction pipeline. An optional Gaussian noise mode imitates sensor noise.
 *
 * <b>The technique</b>
 * A range scanner (LiDAR, structured light, depth camera) measures the distance to the
 * nearest surface along many viewing directions. Rendering already computes exactly that:
 * the depth buffer stores, for every pixel, the nearest visible surface point — hidden
 * surfaces are removed by the depth test. So a virtual scan is simply: render the scene,
 * read back the depth buffer, and back-project each valid pixel into world space. The math
 * is the inverse of the usual rendering pipeline: a pixel (x, y) with depth d is mapped to
 * normalized device coordinates (2(x+0.5)/w - 1, 1 - 2(y+0.5)/h, 2d - 1), multiplied by the
 * inverse model-view-projection matrix, and divided by the resulting homogeneous w. The
 * half-pixel offsets sample pixel centers. Pixels with no geometry (background) are skipped,
 * so the scan contains only visible surface samples — one point per pixel, like a depth
 * image. Adding zero-mean Gaussian noise with a small standard deviation simulates the
 * measurement error of a real scanner.
 *
 * <b>Easy3D API</b>
 * - Viewer::capture(): renders the scene off screen and returns a graphics::FrameImage with
 *   color, per-pixel depth, and object/primitive IDs (used here to reject background pixels).
 * - Camera::modelViewProjectionMatrix() and mat4 inverse(): the transform (and its inverse)
 *   used for back-projection.
 * - PointCloud: the scan result, added to the viewer with add_model().
 * - GaussianNoise::apply(): optional sensor-noise simulation; ClippingPlane: preserved across
 *   the scene change so the scan view is not disturbed.
 *
 * <b>Running it</b>
 * Navigate with the mouse as usual. Press 'space' to capture a scan from the current view
 * (a new model named "Virtual scan" appears); press 'n' to toggle Gaussian noise on the
 * captured points. Scan from several viewpoints to build up multi-view coverage.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_404_VirtualScanner/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_404_VirtualScanner/viewer.cpp
 *
 * \see Viewer, PointCloud, Camera, GaussianNoise, Tutorial_403_PointSelection
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Our custom viewer: a Viewer subclass that captures a scan when 'space' is pressed.
    TutorialVirtualScanner viewer(EXAMPLE_TITLE);

    // The model to scan. add_model() centers and scales it to fit the scene (second argument).
    const std::string file_name = resource::directory() + "/data/house/house.obj";
    auto model = viewer.add_model(file_name, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // "locks" is an auxiliary points drawable marking locked vertices (if the mesh has any).
    // Hide it so such markers are not captured as part of the scanned surface.
    auto d = model->renderer()->get_points_drawable("locks");
    if (d)
        d->set_visible(false);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
