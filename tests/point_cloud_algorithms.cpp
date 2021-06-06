/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/resources.h>


using namespace easy3d;

bool test_algo_point_cloud_normal_estimation() {
    const std::string file = resource::directory() + "/data/bunny.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return false;
    }

    PointCloudNormals algo;
    if (algo.estimate(cloud))
        return true;

    return false;
}


bool test_algo_point_cloud_plane_extraction() {
    const std::string file = resource::directory() + "/data/polyhedron.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

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
        return true;
    }

    return false;
}


bool test_algo_point_cloud_poisson_reconstruction() {
    const std::string file = resource::directory() + "/data/polyhedron.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (!normals) {
        std::cerr << "Poisson surface reconstruction method requires normal information." << std::endl;
        return false;
    }

    const int depth = 6;
    PoissonReconstruction algo;
    algo.set_depth(depth);
    std::cout << "reconstruction depth: " << depth << std::endl;
    Model *surface = algo.apply(cloud);
    if (surface)
        return true;

    return false;
}


int test_point_cloud_algorithms() {
    if (!test_algo_point_cloud_normal_estimation())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_plane_extraction())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_poisson_reconstruction())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}