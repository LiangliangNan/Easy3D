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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_702_Cloud_SurfaceReconstruction/main.cpp
 *
 * \brief Reconstructing a smooth surface mesh from an oriented point cloud (Poisson reconstruction).
 *
 * <b>Overview</b>
 * This interactive tutorial loads the polyhedron point cloud (which already carries normals) and,
 * on Ctrl+R, reconstructs a continuous surface from it. The point cloud is then replaced by the
 * resulting triangle mesh in the viewer. It builds directly on Tutorial_701_Cloud_NormalEstimation:
 * Poisson reconstruction needs consistently oriented normals as input.
 *
 * <b>The technique</b>
 * Poisson surface reconstruction (Kazhdan, Bolitho, and Hoppe, "Poisson Surface Reconstruction",
 * Symposium on Geometry Processing 2006) casts reconstruction as a spatial Poisson problem. The
 * oriented normals define a vector field; the method finds an indicator function (1 inside the
 * object, 0 outside) whose gradient best matches that field, then extracts an isosurface as a
 * triangle mesh. It solves on an adaptive octree, so the "depth" parameter caps the resolution:
 * depth d corresponds to a voxel grid no finer than 2^d x 2^d x 2^d. Higher depth captures more
 * detail at greater memory/time cost. The result is a smooth, watertight SurfaceMesh.
 *
 * <b>Easy3D API</b>
 * - PoissonReconstruction: the algorithm wrapper. set_depth(d) caps the octree depth; apply(cloud)
 *   runs the reconstruction and returns a new SurfaceMesh (or nullptr on failure).
 * - PointCloud::get_vertex_property<vec3>("v:normal"): checks that normals exist before reconstructing.
 * - Viewer::add_model() / delete_model(): swap the cloud for the reconstructed surface.
 * - TrianglesDrawable::set_coloring_method(State::UNIFORM_COLOR): draw the mesh in a single color.
 *
 * <b>Running it</b>
 * Press Ctrl+R to run the reconstruction (depth 6, printed to the console). The point cloud is
 * removed and the reconstructed surface is shown in its place. If the cloud has no normals, an
 * error message points you to Tutorial_701 for estimating them first.
 *
 * \see PoissonReconstruction, SurfaceMesh, Tutorial_701_Cloud_NormalEstimation
 */

using namespace easy3d;

// Callback for Ctrl+R: reconstruct a surface from the oriented point cloud.
bool reconstruction(Viewer *viewer, Model *model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    // Poisson reconstruction requires oriented normals; bail out with guidance if they are absent.
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (!normals) {
        std::cerr << "Poisson surface reconstruction method requires normal information."
                  << " Please provide normal information. Alternatively, you can use the "
                  << " Tutorial_701_Cloud_NormalEstimation for normal estimation" << std::endl;
        return false;
    }

    // Depth caps the octree resolution (a 2^6 = 64-wide voxel grid here). Higher = more detail.
    const int depth = 6;
    PoissonReconstruction algo;
    algo.set_depth(depth);
    std::cout << "reconstruction depth: " << depth << std::endl;
    // apply() solves the Poisson equation and extracts an isosurface as a new SurfaceMesh.
    Model *surface = algo.apply(cloud);
    if (surface != nullptr) {
        // Show the reconstructed mesh, then remove the original cloud.
        viewer->add_model(std::shared_ptr<Model>(surface), true);
        // Draw the surface in a single uniform color.
        auto faces = surface->renderer()->get_triangles_drawable("faces");
        faces->set_coloring_method(State::UNIFORM_COLOR);
        viewer->delete_model(cloud);
        viewer->update(); // request a redraw of the new scene
    }

    return true;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // This sample cloud already includes normals, so it is ready for reconstruction.
    const std::string file = resource::directory() + "/data/polyhedron.bin";

    // Create the viewer that will display the cloud and host the keyboard callback.
    Viewer viewer(EXAMPLE_TITLE);

    Model *model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Register the reconstruction step on Ctrl+R, bound to this model.
    viewer.bind(reconstruction, model, Viewer::KEY_R, Viewer::MODIF_CTRL);
    // Show the shortcut in the on-screen help overlay.
    viewer.set_usage("default", "Ctrl + r: run reconstruction");

    // Enter the event loop; returns when the window is closed.
    return viewer.run();
}
