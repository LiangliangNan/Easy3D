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
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;

// This example shows how to
//		- extract planes from a point cloud using RANSAC.


bool extract_plane(Viewer* viewer, Model* model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (!normals) {
        std::cerr << "Plane extraction using RANSAC requires normal information but it is not available" << std::endl;
        return false;
    }

    PrimitivesRansac algo;
    algo.add_primitive_type(PrimitivesRansac::PLANE);

    // you can try different parameters of RANSAC (usually you don't need to tune them)
    const int num = algo.detect(cloud, 200, 0.005f, 0.02f, 0.8f, 0.001f);
    if (num > 0) {
        std::cout << num << " primitives extracted" << std::endl;

        // assign each plane a unique color
        auto segments = cloud->vertex_property<int>("v:primitive_index");
        const std::string color_name = "v:color-segments";
        auto coloring = cloud->vertex_property<vec3>(color_name, vec3(0, 0, 0));
        Renderer::color_from_segmentation(cloud, segments, coloring);

        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        drawable->set_property_coloring(State::VERTEX, color_name);

        drawable->update();
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
    viewer.set_usage("'Ctrl + e': extract planes");
    // set up the function to be executed and its corresponding shortcut
    viewer.bind(extract_plane, model, Viewer::KEY_E, Viewer::MODIF_CTRL);

    // run the viewer
    return viewer.run();
}

