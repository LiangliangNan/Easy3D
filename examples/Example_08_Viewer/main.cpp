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

#include <easy3d/core/viewer.h>
#include <easy3d/model/point_cloud.h>
#include <easy3d/core/drawable.h>

using namespace easy3d;

// This example shows how to
//		- load a point cloud from a file
//		- create a drawable for rendering
//		- use the viewer to visualize the point cloud

int main(int /* argc */, char ** /* argv */) {
	// create the default Easy3D viewer
	Viewer viewer;

	// create a point cloud
	Point_cloud* cloud = new Point_cloud;

	// load point cloud data from a file
	cloud->read("../../../data/bunny.bin");
	// give the model a name (optional)
	cloud->set_name("bunny");

	// create a drawable (a set of points that can be rendered)
	PointsDrawable* drawable = cloud->add_point_drawable("points");

	// collect points, colors, and normals (if exist) and transfer them to GPU
	auto points = cloud->get_vertex_property<vec3>("v:point");
	drawable->update_vertex_buffer(points.vector());
	auto normals = cloud->get_vertex_property<vec3>("v:normal");
	if (normals)	// if normals exist
		drawable->update_normal_buffer(normals.vector());
	auto colors = cloud->get_vertex_property<vec3>("v:color");
	if (colors)		// if colors exist
		drawable->update_color_buffer(colors.vector());

	// add the model to the viewer
	viewer.add_model(cloud);

	// run the viewer
	viewer.run();

	return EXIT_SUCCESS;
}