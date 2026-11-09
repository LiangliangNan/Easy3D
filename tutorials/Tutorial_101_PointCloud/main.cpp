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

#include <easy3d/core/point_cloud.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_101_PointCloud/main.cpp
 *
 * \brief Creating a point cloud from raw 3D points — the simplest possible Easy3D program.
 *
 * <b>Overview</b>
 * This tutorial builds a PointCloud directly in memory from a set of coordinates, without
 * loading a file and without opening a viewer. It is the "hello world" of Easy3D and shows
 * the skeleton that almost every Easy3D program follows: initialize the library, create or
 * load some data, do something with it, then release it.
 *
 * <b>The technique</b>
 * A point cloud is an unordered collection of samples in 3D space. Each sample is just a
 * position (x, y, z); there are no edges or faces connecting the points. Point clouds are the
 * raw output of most 3D acquisition pipelines (LiDAR, depth cameras, photogrammetry), so they
 * are the natural starting point for geometry processing. Here we synthesize a tiny cloud:
 * 100 points laid out on a regular 10 x 10 grid that lies flat in the XY plane (z = 0). In
 * Easy3D a point cloud stores its samples as vertices — but unlike a SurfaceMesh there is no
 * connectivity between them.
 *
 * <b>Easy3D API</b>
 * - initialize(): bootstraps the library (logging, resource paths, ...). Call it once before
 *   using any other Easy3D facility.
 * - PointCloud: the core data structure for point sets. add_vertex() appends a point and
 *   returns a vertex handle; n_vertices() reports how many points the cloud holds.
 * - vec3: Easy3D's single-precision 3-component vector, used here for the point positions.
 *
 * <b>Running it</b>
 * This example is console-only: it prints "point cloud has 100 points" and exits. No window
 * appears because no Viewer is created. The cloud is a local object here, so it is destroyed
 * automatically when main returns. (When a model is added to a Viewer instead, the Viewer
 * takes ownership of it — see Tutorial_201_Viewer_default.)
 *
 * \see PointCloud, vec3, Tutorial_102_PointCloud_Property, Tutorial_201_Viewer_default
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create an empty point cloud.
    PointCloud cloud;

    // Add some points. Here we add 100 points on a 10 x 10 grid in the XY plane.
    for (int i = -5; i < 5; ++i) {
        for (int j = -5; j < 5; ++j)
            // add_vertex() appends one sample; vec3 is Easy3D's 3D position type.
            cloud.add_vertex(
                vec3(static_cast<float>(i), static_cast<float>(j), 0)); // z = 0: all points are on the XY plane
    }

    // n_vertices() is the number of points currently stored in the cloud.
    std::cout << "point cloud has " << cloud.n_vertices() << " points" << std::endl;

    return EXIT_SUCCESS;
}