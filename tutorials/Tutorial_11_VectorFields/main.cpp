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

#include <easy3d/viewer.h>
#include <easy3d/point_cloud.h>
#include <easy3d/drawable.h>

using namespace easy3d;


// This example shows how to
//		- rendering a vector field defined on a point cloud;
//		- use the viewer to visualize the drawable.


int main() {
	// Create the default Easy3D viewer.
	// Note: a viewer must be created before creating any drawables. 
    Viewer viewer("Tutorial_11_VectorFields");

	// Create a point cloud
	Point_cloud* cloud = new Point_cloud;

	// Load point cloud data from a file
	cloud->read("../../../data/building_with_normals.bin");

	// Create a drawable for rendering the point cloud itself.
	PointsDrawable* points_drawable = cloud->add_point_drawable("points");

	// Collect points, colors, and normals (if exist) and transfer them to GPU
	auto points = cloud->get_vertex_property<vec3>("v:point");
	points_drawable->update_vertex_buffer(points.vector());
	auto normals = cloud->get_vertex_property<vec3>("v:normal");
	if (normals)	// if normals exist
		points_drawable->update_normal_buffer(normals.vector());
	auto colors = cloud->get_vertex_property<vec3>("v:color");
	if (colors)		// if colors exist
		points_drawable->update_color_buffer(colors.vector());

	points_drawable->set_per_vertex_color(colors); // set to true if has color property
	points_drawable->set_default_color(vec3(0.4f, 0.8f, 0.8f));
	points_drawable->set_point_size(6.0f);

	// Now let's create a drawable to visualize the point normals.
	if (normals) {
		// Get the bounding box of the model. Then we defined the length of the
		// normal vectors to be 1% of the bounding box diagonal.
		Box3 box;
		for (auto v : cloud->vertices())
			box.add_point(points[v]);
		float length = norm(box.max() - box.min()) * 0.01f;

		// Every consecutive two points represent a normal vector.
		std::vector<vec3> normal_points;
		for (auto v : cloud->vertices()) {
			const vec3& s = points[v];
			vec3 n = normals[v];
			n.normalize();
			const vec3& t = points[v] + n * length;
			normal_points.push_back(s);
			normal_points.push_back(t);
		}

		// Create a drawable for rendering the normal vectors.
		LinesDrawable* normals_drawable = cloud->add_line_drawable("normals");
		// Upload the data to the GPU.
		normals_drawable->update_vertex_buffer(normal_points);
		// We will draw the normal vectors in green color
		normals_drawable->set_per_vertex_color(false);
		normals_drawable->set_default_color(vec3(0.0f, 1.0f, 0.0f));
	}
	else {
		std::cerr << "This point cloud does not have normal information. "
			"No vector field can be visualized." << std::endl;
	}

	// Add the model to the viewer
	viewer.add_model(cloud);

	// Run the viewer
	viewer.run();
}
