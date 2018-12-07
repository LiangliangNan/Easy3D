/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/model/point_cloud.h>

using namespace easy3d;

void main() {
	// Create a point cloud
	Point_cloud cloud;

	// Add some points. Here we add 100 points on a 10*10 grid
	for (float i = -5; i < 5; ++i) {
		for (float j = -5; j < 5; ++j) {
			cloud.add_vertex(vec3(i, j, 0));// z = 0: all points are on XY plane
		}
	}
	std::cout << "point cloud has " << cloud.n_vertices() << " points" << std::endl;

	// We can add a per-point property, e.g., normal

	// NOTE: it is "add" instead of "get"
	Point_cloud::Vertex_property<vec3> normals = cloud.add_vertex_property<vec3>("v:normal");
	for (auto v : cloud.vertices()) {
		normals[v] = vec3(0, 0, 1);	// All points have the same normal direction.
	}

	// Now let's print all point coordinates and their normals

	// By default, points are stored as a Vertex_property named "v:point".
	// The 'v:' is optional, but easy to distinguished from edge/face properties.  
	Point_cloud::Vertex_property<vec3> points = cloud.get_vertex_property<vec3>("v:point");

	int index = 0;
	for (auto v : cloud.vertices()) {
		std::cout << "point " << index++ << ": (" << points[v] << "), (" << normals[v] << ")" << std::endl;
	}

}
