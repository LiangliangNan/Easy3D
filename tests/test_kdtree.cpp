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

#include <iostream>

#include <easy3d/core/point_cloud.h>
#include <easy3d/kdtree/kdtree_search_ann.h>
#include <easy3d/kdtree/kdtree_search_eth.h>
#include <easy3d/kdtree/kdtree_search_flann.h>
#include <easy3d/kdtree/kdtree_search_nanoflann.h>
#include <easy3d/util/resource.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/util/stop_watch.h>

using namespace easy3d;


void evaluate(const PointCloud* cloud, KdTreeSearch* tree) {
    std::cout << "\tquerying closest vertex (for each point in the point cloud)...";
    StopWatch w;
    for (auto v : cloud->vertices())
        tree->find_closest_point(cloud->position(v));
    std::cout << " done. time = " << w.time_string() << std::endl;

    std::cout << "\tquerying K(=16) closest vertex (for each point in the point cloud)...";
    const unsigned int k = 16;
    std::vector<int> neighbors;
    w.restart();
    for (auto v : cloud->vertices()) {
        tree->find_closest_k_points(cloud->position(v), k, neighbors);
    }
    std::cout << " done. time = " << w.time_string() << std::endl;

    std::cout << "\tquerying the nearest neighbors within a fixed range (for each point in the point cloud). ";
    const float radius = cloud->bounding_box().radius() * 0.0001f;
    std::cout << " radius = " << radius << "...";
    w.restart();
    for (auto v : cloud->vertices())
        tree->find_points_in_range(cloud->position(v), radius, neighbors);
    std::cout << " done. time = " << w.time_string() << std::endl;
}


// This examples shows how to use the kd-tree.
int test_kdtree() {
    std::cout << "testing kd-tree..." << std::endl;
    const std::string file = resource::directory() + "/data/bunny.bin";
    PointCloud* cloud = PointCloudIO::load(file);
    if (!cloud) {
        LOG(ERROR) << "point cloud doesn't exist: " << file;
        return EXIT_FAILURE;
    }
    std::cout << "\tnumber of points in point cloud: " << cloud->n_vertices() << std::endl;

    std::cout << "------- kd-tree using ANN --------" << std::endl;
    std::cout << "\tconstructing kd-tree...";
    StopWatch w;
    KdTreeSearch_ANN ann(cloud);
    std::cout << " done. time = " << w.time_string() << std::endl;
    evaluate(cloud, &ann);

    std::cout << "------- kd-tree using ETH --------" << std::endl;
    std::cout << "\tconstructing kd-tree...";
    w.restart();
    KdTreeSearch_ETH eth(cloud);
    std::cout << " done. time = " << w.time_string() << std::endl;
    evaluate(cloud, &eth);

    std::cout << "------- kd-tree using FLANN --------" << std::endl;
    std::cout << "\tconstructing kd-tree...";
    w.restart();
    KdTreeSearch_FLANN flann(cloud);
    std::cout << " done. time = " << w.time_string() << std::endl;
    evaluate(cloud, &flann);

    std::cout << "------- kd-tree using NANOFLANN --------" << std::endl;
    std::cout << "\tconstructing kd-tree...";
    w.restart();
    KdTreeSearch_NanoFLANN nanoflann(cloud);
    std::cout << " done. time = " << w.time_string() << std::endl;
    evaluate(cloud, &nanoflann);

    return EXIT_SUCCESS;
}
