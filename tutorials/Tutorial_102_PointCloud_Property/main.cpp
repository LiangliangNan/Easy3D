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
#include <easy3d/core/random.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_102_PointCloud_Property/main.cpp
 *
 * \brief Attaching per-point attributes (here, colors) to a PointCloud using named properties.
 *
 * <b>Overview</b>
 * Building on Tutorial_101_PointCloud, this console tutorial shows how to store extra data
 * alongside each point. We synthesize the same 10 x 10 grid of points, give every point a random
 * color, then read back both the coordinates and the colors and print them. No viewer is opened;
 * the goal is to understand Easy3D's property system, which underpins almost all geometry
 * processing in the library.
 *
 * <b>The technique</b>
 * A property (a.k.a. a property map) is a parallel array: one value per mesh element, addressed
 * by the element's handle. Rather than a struct-of-fields per point, Easy3D keeps each attribute
 * in its own contiguous array, so adding or removing an attribute never disturbs the others and
 * iteration stays cache-friendly. Properties are identified by a unique string name. By
 * convention a prefix says which element type the property lives on: "v:" for vertices, "e:" for
 * edges, "f:" for faces (the prefix is optional but recommended). The coordinates themselves are
 * just a built-in vertex property named "v:point", so a position is accessed exactly like any
 * other attribute. Creating a property whose name is already in use triggers a warning, which is
 * why every property must be given a distinct name.
 *
 * <b>Easy3D API</b>
 * - PointCloud::add_vertex_property<T>(name): creates a new per-vertex property of type T and
 *   returns a VertexProperty<T> accessor; values are read/written with colors[v].
 * - PointCloud::get_vertex_property<T>(name): looks up an existing property (here "v:point").
 * - PointCloud::vertices(): a range of all vertex handles, usable in a range-based for loop.
 * - random_color() (easy3d/core/random.h): a random vec3 in [0,1]^3, used here as an RGB color.
 *
 * <b>Running it</b>
 * Console-only. It first prints "point cloud has 100 points", then one line per point giving its
 * index, xyz coordinates, and random color, e.g. "index: 0, xyz: ..., color: ...". The cloud is
 * heap-allocated here and explicitly deleted at the end to release its memory.
 *
 * \see PointCloud, vec3, Tutorial_101_PointCloud, Tutorial_106_SurfaceMesh_Property
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Seed the random number generator. A fixed seed (0) makes the "random" colors below
    // reproducible, so the printed output is identical on every run.
    srand(0);

    // Create a point cloud.
    auto cloud = new PointCloud;

    // Add some points. Here we add 100 points on a 10*10 grid.
    for (int i = -5; i < 5; ++i) {
        for (int j = -5; j < 5; ++j)
            cloud->add_vertex(
                vec3(static_cast<float>(i), static_cast<float>(j), 0)); // z = 0: all points are on XY plane.
    }
    std::cout << "point cloud has " << cloud->n_vertices() << " points" << std::endl;

    // In Easy3D, all actual 3D data and related information are stored as
    // per-element (e.g., vertex, edge, and face) properties. A model can have
    // multiple properties and the properties can be accessed by their names.
    // So it is important to give each property a unique name. Easy3D will pop
    // up a warning if you attempt to create a property with an already used name.

    // Let's add a random color to each point. We give this property a name
    // "v:color". Here "v:" is optional, and it just indicates that this property
    // is defined on the vertices. You will later see properties can be defined
    // one edges (use "e:") and faces (use "f:") for surface meshes.
    auto colors = cloud->add_vertex_property<vec3>("v:color");
    for (auto v : cloud->vertices()) // iterate over all vertices
        colors[v] = random_color();  // assign a random color to point 'v'

    // You can use the get_vertex_property() function to access the named properties.
    // Below I show you how to access the xyz coordinates and color of each point.
    // Here we simply print the coordinates and colors.

    // The point coordinates are stored as a per-point property 'v:point'.
    auto points = cloud->get_vertex_property<vec3>("v:point");
    for (auto v : cloud->vertices())
        std::cout << "index: " << v.idx() << ", xyz: " << points[v] << ", color: " << colors[v] << std::endl;

    // delete the point cloud (i.e., release memory)
    delete cloud;

    return EXIT_SUCCESS;
}
