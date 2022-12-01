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

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/algo/delaunay_2d.h>
#include <easy3d/algo/delaunay_3d.h>
#include <easy3d/algo/point_cloud_simplification.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/util/resource.h>


using namespace easy3d;

bool test_algo_point_cloud_normal_estimation() {
    const std::string file = resource::directory() + "/data/bunny.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return false;
    }

    PointCloudNormals algo;

    std::cout << "estimating point cloud normals..." << std::endl;
    if (algo.estimate(cloud, 16)) {
        std::cout << "reorienting point cloud normals..." << std::endl;
        if (algo.reorient(cloud, 16)) {
            delete cloud;
            return true;
        }
    }

    delete cloud;
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
    std::cout << "detecting planes using RANSAC..." << std::endl;

    // you can try different parameters of RANSAC (usually you don't need to tune them)
    const int num = algo.detect(cloud, 200, 0.005f, 0.02f, 0.8f, 0.001f);
    delete cloud;

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
    std::cout << "Poisson surface reconstruction (depth = " << depth << ")..." << std::endl;
    Model *surface = algo.apply(cloud);
    delete cloud;

    if (surface)
        return true;

    return false;
}


bool test_algo_point_cloud_delaunay_triangulation_2D() {
    const std::string file = resource::directory() + "/data/bunny.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return false;
    }

    const std::vector<vec3>& pts = cloud->points();

    std::vector<vec2> points;
    for (std::size_t i = 0; i < pts.size(); ++i) {
        points.push_back(vec2(pts[i]));
    }

    std::cout << "Delaunay triangulation 2D..." << std::endl;
    Delaunay2 delaunay;
    delaunay.set_vertices(points);

    delete cloud;
    return true;
}


bool test_algo_point_cloud_delaunay_triangulation_3D() {
    const std::string file = resource::directory() + "/data/bunny.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return false;
    }

    const std::vector<vec3>& points = cloud->points();

    std::cout << "Delaunay triangulation 3D..." << std::endl;
    Delaunay3 delaunay;
    delaunay.set_vertices(points);

    delete cloud;
    return true;
}


bool test_algo_point_cloud_downsampling() {
    const std::string file = resource::directory() + "/data/bunny.bin";
    PointCloud *cloud = PointCloudIO::load(file);
    if (!cloud) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return false;
    }

    int total_num = cloud->n_vertices();

    float threshold = 0.01f;
    std::cout << "grid downsampling using distance threshold " << threshold << "...";
    {
        PointCloud pcd = *cloud;
        auto points_to_remove = PointCloudSimplification::grid_simplification(&pcd, threshold);
        for (auto id : points_to_remove)
            pcd.delete_vertex(PointCloud::Vertex(id));
        pcd.collect_garbage();
        std::cout << " " << total_num << " -> " << pcd.n_vertices() << std::endl;
    }

    std::cout << "uniform downsampling using distance threshold " << threshold << "...";
    {
        PointCloud pcd = *cloud;
        auto points_to_remove = PointCloudSimplification::uniform_simplification(&pcd, threshold);
        for (auto id : points_to_remove)
            pcd.delete_vertex(PointCloud::Vertex(id));
        pcd.collect_garbage();
        std::cout << " " << total_num << " -> " << pcd.n_vertices() << std::endl;
    }

    auto expected_number = static_cast<unsigned int>(total_num * 0.5f);
    std::cout << "uniform downsampling to expected point number " << expected_number << ")...";
    {
        PointCloud pcd = *cloud;
        auto points_to_remove = PointCloudSimplification::uniform_simplification(&pcd, static_cast<unsigned int>(total_num * 0.5f));
        for (auto id : points_to_remove)
            pcd.delete_vertex(PointCloud::Vertex(id));
        pcd.collect_garbage();
        std::cout << " " << total_num << " -> " << pcd.n_vertices() << std::endl;
    }

    delete cloud;
    return true;
}


int test_point_cloud_algorithms() {
    if (!test_algo_point_cloud_normal_estimation())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_plane_extraction())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_poisson_reconstruction())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_delaunay_triangulation_2D())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_delaunay_triangulation_3D())
        return EXIT_FAILURE;

    if (!test_algo_point_cloud_downsampling())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
