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
#include <easy3d/renderer/camera.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;

// This example shows how to
//		- estimate normal information of a point cloud.
//      - re-orient the normals.

bool estimate(Viewer* viewer, Model* model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    if (PointCloudNormals::estimate(cloud)) {
        auto normals = cloud->get_vertex_property<vec3>("v:normal");
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        // Upload the vertex normals to the GPU.
        drawable->update_normal_buffer(normals.vector());
        viewer->update();
        return true;
    }
    else
        return false;
}

bool reorient(Viewer* viewer, Model* model) {
    if (!viewer || !model)
        return false;

    auto cloud = dynamic_cast<PointCloud *>(model);
    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (!normals) {
        LOG(WARNING) << "normal information does not exist";
        return false;
    }

    if (PointCloudNormals::reorient(cloud)) {
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        // Upload the vertex normals to the GPU.
        drawable->update_normal_buffer(normals.vector());
        viewer->update();
        return true;
    }
    else
        return false;
}

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    const std::string file = resource::directory() + "/data/bunny.bin";

    // create the viewer.
    Viewer viewer(EXAMPLE_TITLE);

    Model *model = viewer.add_model(file, true);
    if (!model) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }
    // to view the bunny from its front
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(1, 0, 0));

    // setup rendering parameters
    auto drawable = model->renderer()->get_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(0.6f, 0.6f, 1.0f, 1.0f));
    drawable->set_point_size(3.0f);
    drawable->set_lighting_two_sides(false);

    // usage
    viewer.set_usage("Ctrl + e: estimate normals\n"
                     "Ctrl + r: reorient normals");
    // set up the functions to be executed and their corresponding shortcuts
    viewer.bind(estimate, model, Viewer::KEY_E, Viewer::MODIF_CTRL);
    viewer.bind(reorient, model, Viewer::KEY_R, Viewer::MODIF_CTRL);

    // run the viewer
    return viewer.run();
}

