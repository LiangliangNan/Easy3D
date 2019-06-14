/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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

#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>

using namespace easy3d;


// This example shows how to
//		- load a surface mesh from a file;
//		- save a surface mesh into a file.


int main(int /*argc*/, char** /*argv*/) {
	// Read a mesh specified by its file name
    const std::string file_name = "../../Easy3D/data/building.off";
    SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return EXIT_FAILURE;
	}
	std::cout << "mesh loaded. " << std::endl;
	std::cout << "\tvertices: " << mesh->n_vertices() << std::endl;
	std::cout << "\tedges: " << mesh->n_edges() << std::endl;
	std::cout << "\tfaces: " << mesh->n_faces() << std::endl;

	// ...
	// Do fancy stuff with the mesh
	// ...

	// Write the mesh to a new file.
    std::string save_file_name = "./building-copy.obj";
    if (SurfaceMeshIO::save(save_file_name, mesh))
		std::cout << "mesh saved" << std::endl;
	else
		std::cerr << "failed create the new file" << std::endl;

	// delete the mesh (i.e., release memory)
	delete mesh;

    return EXIT_SUCCESS;
}

