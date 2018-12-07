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

#include <easy3d/model/surface_mesh.h>

using namespace easy3d;


// This example shows how to
//		- load a surface mesh from a file;
//		- save a surface mesh into a file.


void main() {
	// Create a surface mesh
	Surface_mesh* mesh = new Surface_mesh;

	// Read a mesh specified by its file name
	std::string file_name = "../../../data/building.off";
	bool success = mesh->read(file_name);
	if (!success) {
		std::cerr << "file does not exist or invalid file format" << std::endl;
		delete mesh;
		return;
	}
	std::cout << "mesh loaded. " << std::endl;
	std::cout << "\tvertices: " << mesh->n_vertices() << std::endl;
	std::cout << "\tedges: " << mesh->n_edges() << std::endl;
	std::cout << "\tfaces: " << mesh->n_faces() << std::endl;

	// ...
	// Do fancy stuff with the mesh
	// ...

	// Write the mesh to a new file.
	std::string save_file_name = "../../../data/building-copy.obj";
	success = mesh->write(save_file_name);
	if (success)
		std::cout << "mesh saved" << std::endl;
	else
		std::cerr << "failed create the new file" << std::endl;

	// delete the mesh (i.e., release memory)
	delete mesh;
}
