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
#include <easy3d/core/random.h>
#include <easy3d/util/initializer.h>

using namespace easy3d;


// This example shows how to
//		- add per-point properties to a point cloud;
//		- access existing properties.


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

	// Initialize random number generator.
	srand(0);

	// Create a point cloud.
	auto cloud = new PointCloud;

	// Add some points. Here we add 100 points on a 10*10 grid.
	for (int i=-5; i<5; ++i) {
		for (int j = -5; j < 5; ++j)
			cloud->add_vertex(vec3(static_cast<float>(i), static_cast<float>(j), 0));// z = 0: all points are on XY plane.
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
	for (auto v : cloud->vertices())	// iterate over all vertices
		colors[v] = random_color();		// assign a random color to point 'v'

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

