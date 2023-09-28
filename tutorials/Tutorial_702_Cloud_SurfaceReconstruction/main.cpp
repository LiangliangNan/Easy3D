/********************************************************************
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
 ********************************************************************/

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;

// This example shows how to
//		- reconstruct a smooth surface from a point cloud using the Poisson surface reconstruction method


bool reconstruction(Viewer* viewer, Model* model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (!normals) {
        std::cerr << "Poisson surface reconstruction method requires normal information."
                  << " Please provide normal information. Alternatively, you can use the "
                  << " Tutorial_701_Cloud_NormalEstimation for normal estimation" << std::endl;
        return false;
    }

    const int depth = 6;
    PoissonReconstruction algo;
    algo.set_depth(depth);
    std::cout << "reconstruction depth: " << depth << std::endl;
    Model* surface = algo.apply(cloud);
    if (surface != nullptr) {
        viewer->add_model(surface, true);
        viewer->delete_model(cloud);
        viewer->update();
    }

    return true;
}


int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    const std::string file = resource::directory() + "/data/polyhedron.bin";

    // create the viewer.
    Viewer viewer(EXAMPLE_TITLE);

    Model *model = viewer.add_model(file, true);
    if (!model) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    // setup rendering parameters
    auto drawable = model->renderer()->get_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(0.6f, 0.6f, 1.0f, 1.0f));
    drawable->set_point_size(3.0f);

    // usage
    viewer.set_usage("'Ctrl + r': run reconstruction");
    // set up the function to be executed and its corresponding shortcut
    viewer.bind(reconstruction, model, Viewer::KEY_R, Viewer::MODIF_CTRL);

    // run the viewer
    return viewer.run();
}

