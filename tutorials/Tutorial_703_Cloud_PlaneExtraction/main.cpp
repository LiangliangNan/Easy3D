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
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_703_Cloud_PlaneExtraction/main.cpp
 *
 * \brief Detecting planar regions in a point cloud with RANSAC and coloring each detected plane.
 *
 * <b>Overview</b>
 * This interactive tutorial loads the polyhedron point cloud and, on Ctrl+E, extracts its planar
 * faces automatically. Each detected plane is assigned a distinct color so the segmentation is
 * visible. Like Tutorial_702, it requires per-point normals (see Tutorial_701 for estimating them).
 *
 * <b>The technique</b>
 * RANSAC (RANdom SAmple Consensus; Fischler and Bolles, 1981) fits a model robustly in the presence
 * of outliers: repeatedly draw a minimal random sample (three points define a plane), fit the
 * candidate plane, and count how many points lie within a distance threshold (the "inliers"). The
 * candidate with the most support wins; its inliers are removed and the search repeats for the next
 * primitive. Easy3D implements the efficient variant of Schnabel et al. ("Efficient RANSAC for
 * Point-Cloud Shape Detection", Computer Graphics Forum 2007), which accelerates inlier testing with
 * a spatial bitmap and rejects candidates whose normals disagree with the samples. The detect()
 * arguments here are: min_support = 200 points, distance threshold = 0.005, bitmap resolution =
 * 0.02, normal compatibility threshold = 0.8, and overlook probability = 0.001.
 *
 * <b>Easy3D API</b>
 * - PrimitivesRansac: the detector. add_primitive_type(PrimitivesRansac::PLANE) selects planes;
 *   detect(cloud, ...) returns the number of primitives found and stores a "v:primitive_index"
 *   per-point label.
 * - Renderer::color_from_segmentation(cloud, segments, coloring): maps integer segment labels to
 *   distinct colors in a vertex property.
 * - PointsDrawable::set_property_coloring(State::VERTEX, name): colors each point from that property.
 *
 * <b>Running it</b>
 * Press Ctrl+E to extract planes. The console reports how many primitives were found, and each
 * planar region is drawn in its own color (points not on any detected plane stay black).
 *
 * \see PrimitivesRansac, Renderer, Tutorial_701_Cloud_NormalEstimation
 */

using namespace easy3d;

// Callback for Ctrl+E: detect planes and color each one distinctly.
bool extract_plane(Viewer *viewer, Model *model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    // Efficient RANSAC uses normals to reject incompatible candidates, so they are required.
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (!normals) {
        std::cerr << "Plane extraction using RANSAC requires normal information but it is not available" << std::endl;
        return false;
    }

    // Configure the detector to look for planes (other types: sphere, cylinder, cone, torus).
    PrimitivesRansac algo;
    algo.add_primitive_type(PrimitivesRansac::PLANE);

    // Run detection. Arguments: min_support, distance threshold, bitmap resolution, normal
    // threshold, overlook probability. You can try different values (usually no tuning is needed).
    const int num = algo.detect(cloud, 200, 0.005f, 0.02f, 0.8f, 0.001f);
    if (num > 0) {
        std::cout << num << " primitives extracted" << std::endl;

        // detect() labeled each point with its plane id in "v:primitive_index". Turn those integer
        // labels into distinct RGB colors stored in a new "v:color-segments" vertex property.
        auto segments = cloud->vertex_property<int>("v:primitive_index");
        const std::string color_name = "v:color-segments";
        auto coloring = cloud->vertex_property<vec3>(color_name, vec3(0, 0, 0));
        Renderer::color_from_segmentation(cloud, segments, coloring);

        // Tell the points drawable to color each point from the segmentation-color property.
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        drawable->set_property_coloring(State::VERTEX, color_name);

        drawable->update(); // re-extract the color buffer for the GPU
        viewer->update();   // request a redraw
    }
    return true;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // This sample cloud already includes normals, so it is ready for RANSAC.
    const std::string file = resource::directory() + "/data/polyhedron.bin";

    // Create the viewer that will display the cloud and host the keyboard callback.
    Viewer viewer(EXAMPLE_TITLE);

    Model *model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Initial appearance: a uniform light-blue cloud (replaced by segment colors after detection).
    auto drawable = model->renderer()->get_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(0.6f, 0.6f, 1.0f, 1.0f));
    drawable->set_point_size(3.0f);

    // Register the plane-extraction step on Ctrl+E, bound to this model.
    viewer.bind_shortcut(extract_plane, model, Viewer::KEY_E, Viewer::MODIF_CTRL);
    // Show the shortcut in the on-screen help overlay.
    viewer.set_usage("default", "Ctrl + e: extract planes");

    // Enter the event loop; returns when the window is closed.
    return viewer.run();
}
