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
 * \example{lineno} Tutorial_506_DepthMap/main.cpp
 *
 * \brief Visualizing the scene's depth buffer as an inset depth image.
 *
 * <b>Overview</b>
 * This tutorial shows how to access and display the depth information that the renderer
 * produces for every frame. The custom viewer draws a small depth-map inset in the corner of
 * the window, next to the normal color rendering of a point cloud. Unlike the other 5xx
 * effect tutorials, it does not wrap draw() with an effect object; instead it overrides
 * pre_draw() to register a screen-space overlay that samples the frame's depth texture on the
 * GPU (all logic lives in the TutorialDepthMap class, shown after this file).
 *
 * <b>The technique</b>
 * Every rasterized frame implicitly computes a depth (z-)buffer: for each pixel it stores the
 * distance of the nearest rendered fragment to the camera, which is how hidden-surface
 * removal works. That buffer is itself an image — a "depth map" or range image — where each
 * pixel encodes geometry instead of color. Depth maps are the raw product of depth cameras
 * and virtual scanners, and they are the input of many screen-space effects (shadow mapping,
 * ambient occlusion, eye-dome lighting). Visualizing one requires mapping the depth values to
 * grayscale: nearer surfaces appear brighter/darker than distant ones, so silhouettes and
 * surface relief become visible as intensity variations. Here the inset samples the current
 * frame's scene depth directly on the GPU (no readback to the CPU and no image upload), so it
 * updates live as you navigate. Note that the perspective depth stored by the GPU is
 * non-linear — precision is concentrated near the camera — which is why the intensity
 * gradient is steeper for close geometry.
 *
 * <b>Easy3D API</b>
 * - Viewer: the interactive viewer; subclass it and override pre_draw(), a hook that runs
 *   before the main scene render each frame.
 * - Viewer::set_overlays(): registers backend-independent graphics::DrawCommand overlays that
 *   are drawn on top of the scene.
 * - graphics::shape::draw_depth_texture() / draw_quad_wire(): build overlay commands that
 *   sample the frame's scene depth into a screen-space Rect and outline it, respectively.
 * - PointCloud / DrawablePoints: the data and its points drawable; set_point_size() enlarges
 *   the splats so the depth image has fewer holes.
 *
 * <b>Running it</b>
 * No special keys: navigate with the standard Viewer controls (mouse drag to rotate, wheel to
 * zoom) and watch the red-framed depth inset in the lower-left corner update with the view.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_506_DepthMap/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_506_DepthMap/viewer.cpp
 *
 * \see Viewer, PointCloud, DrawablePoints, Tutorial_204_Offscreen, Tutorial_505_EyeDomeLighting
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create our custom viewer (a Viewer subclass that shows a live depth-map inset).
    TutorialDepthMap viewer(EXAMPLE_TITLE);

    // A scanned point cloud: rich geometry whose depth image shows clear relief.
    const std::string file_name = resource::directory() + "/data/fountain/pointcloud.ply";
    // Load the cloud and add it to the viewer. add_model() creates the model's renderer;
    // the second argument (true) centers and scales the model to fit the scene.
    auto model = viewer.add_model(file_name, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // "vertices" is the standard points drawable of a point cloud. Larger splats cover more
    // pixels, so the depth buffer (and thus the depth inset) has fewer background holes.
    auto drawable = model->renderer()->get_points_drawable("vertices");
    drawable->set_point_size(5);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
