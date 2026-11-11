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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_701_Cloud_NormalEstimation/main.cpp
 *
 * \brief Estimating per-point normals for a point cloud, then making their orientations consistent.
 *
 * <b>Overview</b>
 * This interactive tutorial loads the bunny point cloud and adds two keyboard-triggered steps:
 * estimate a normal at every point (Ctrl+E), then reorient the normals so they all point the same
 * way (Ctrl+R). The resulting normals are uploaded to the GPU so the points can be shaded. Normal
 * estimation is a prerequisite for many downstream tasks, notably Poisson surface reconstruction
 * (Tutorial_702_Cloud_SurfaceReconstruction) and RANSAC plane extraction (Tutorial_703).
 *
 * <b>The technique</b>
 * A raw point cloud has no normals — only positions. To estimate one, we look at a point's k
 * nearest neighbors and fit a local plane: the covariance matrix of the neighbor positions has
 * three eigenvectors, and the one with the smallest eigenvalue (least variance) is perpendicular
 * to the best-fit plane, i.e. the normal. This is Principal Component Analysis (PCA). PCA fixes
 * the normal's axis but not its sign: each normal may point "in" or "out" arbitrarily. Reorientation
 * resolves this by building a neighborhood graph and propagating a consistent flip assignment over
 * a minimum spanning tree, so neighboring normals agree (Hoppe et al., "Surface reconstruction from
 * unorganized points", SIGGRAPH 1992).
 *
 * <b>Easy3D API</b>
 * - PointCloudNormals::estimate(cloud, k): PCA normal estimation; stores the result in the "v:normal"
 *   vertex property. PointCloudNormals::reorient(cloud, k): makes the orientations consistent.
 * - PointCloud::get_vertex_property<vec3>("v:normal"): retrieves the computed normals.
 * - PointsDrawable::update_normal_buffer(...): uploads the normals to the GPU for shading.
 * - Viewer::bind_shortcut(func, model, key, modifier): registers a callback on a keyboard shortcut.
 *
 * <b>Running it</b>
 * Press Ctrl+E to estimate normals, then Ctrl+R to reorient them. Points use a distinct back color,
 * so inconsistently oriented normals are visible as differently colored patches before reorientation.
 *
 * \see PointCloudNormals, PointCloud, PointsDrawable, Tutorial_702_Cloud_SurfaceReconstruction, Tutorial_703_Cloud_PlaneExtraction
 */

using namespace easy3d;

// Callback for Ctrl+E: estimate a normal at every point via PCA on its k nearest neighbors.
bool estimate(Viewer *viewer, Model *model) {
    if (!viewer || !model)
        return false;

    // The bound model is a PointCloud; downcast from the generic Model handle.
    auto cloud = dynamic_cast<PointCloud *>(model);
    // estimate() writes the per-point normals into the "v:normal" vertex property.
    if (PointCloudNormals::estimate(cloud)) {
        auto normals = cloud->get_vertex_property<vec3>("v:normal");
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        // Upload the vertex normals to the GPU so the points can be shaded.
        drawable->update_normal_buffer(normals.vector());
        viewer->update(); // request a redraw to show the new shading
        return true;
    } else
        return false;
}

// Callback for Ctrl+R: flip normals so their orientations are globally consistent.
bool reorient(Viewer *viewer, Model *model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    // Reorientation needs existing normals; tell the user to estimate them first.
    if (!normals) {
        std::cerr << "normal information does not exist" << std::endl;
        return false;
    }

    // reorient() propagates a consistent sign over a minimum spanning tree of the neighborhood graph.
    if (PointCloudNormals::reorient(cloud)) {
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        // Re-upload the (now consistently oriented) normals to the GPU.
        drawable->update_normal_buffer(normals.vector());
        viewer->update();
        return true;
    } else
        return false;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    const std::string file = resource::directory() + "/data/bunny.bin";

    // Create the viewer that will display the cloud and host the keyboard callbacks.
    Viewer viewer(EXAMPLE_TITLE);

    // Load the bunny point cloud; the viewer takes ownership (second argument = true).
    auto model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }
    // Orient the camera to view the bunny from its front.
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(1, 0, 0));

    // Configure how the points are drawn.
    auto drawable = model->renderer()->get_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(0.6f, 0.6f, 1.0f, 1.0f)); // a single light-blue color
    drawable->set_point_size(3.0f);
    // Shade back-facing points differently so flipped normals are easy to spot.
    drawable->set_distinct_back_color(true);

    // Register the two processing steps on Ctrl+E and Ctrl+R, each bound to this model.
    viewer.bind_shortcut(estimate, model, Viewer::KEY_E, Viewer::MODIF_CTRL);
    viewer.bind_shortcut(reorient, model, Viewer::KEY_R, Viewer::MODIF_CTRL);
    // Show the shortcuts in the on-screen help overlay.
    viewer.set_usage("default", "Ctrl + e: estimate normals\n" "Ctrl + r: reorient normals");

    // Enter the event loop; returns when the window is closed.
    return viewer.run();
}
