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

#include <easy3d/point_cloud.h>

#include <fstream>


using namespace easy3d;


// This example shows how to
//		- load a point cloud from a file;
//		- save a point cloud into a file.


int main(int /*argc*/, char** /*argv*/) {
	// Create a point cloud.
	PointCloud* cloud = new PointCloud;

	// Read the point cloud from a known file. 
	// We use PointCloud's built-in function read().
    bool success = cloud->read("../../../Easy3D/data/building_with_normals.bin");
	if (success) {
		std::cout << "point cloud has " << cloud->n_vertices() << " points" << std::endl;

		// Now let's save the model into a file with customized format. In each 
		// line we store the x, y, z coordinates, followed by the normal (nx, ny,
		// nz) and color (r, g, b) if they exist.
        std::ofstream output("../../../Easy3D/data/building_with_normals-copy.txt");
		if (output.is_open()) { // if the file has been successfully created
			// The point coordinates.
			PointCloud::Vertex_property<vec3> points = cloud->get_vertex_property<vec3>("v:point");
			// The point normals.
			PointCloud::Vertex_property<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
			// The point colors.
			PointCloud::Vertex_property<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
			std::cout << "saving the point cloud...";

			int prev_percent = -1;
			for (auto v : cloud->vertices()) {
				output << points[v];
				if (normals)	// if normals exist
					output << " " << normals[v];
				if (colors)		// if colors exist
					output << " " << colors[v];
				output << std::endl;

				// Show the progress
				int progress = int(100.0f * (v.idx() + 1) / float(cloud->n_vertices()));
				if (progress != prev_percent) {
					printf("%3d%%\b\b\b\b", progress);
					prev_percent = progress;
				}
			}
		}
	}
	else 
		std::cout << "file does not exist or invalid file format" << std::endl;

	// Delete the point cloud (i.e., release memory)
	delete cloud;

    return EXIT_SUCCESS;
}

