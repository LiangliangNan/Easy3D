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

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;


// This example shows how to
//		- construct a mesh from its vertices and known connectivity


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

	// Easy3D provides two options to construct a surface mesh.
    //  - Option 1: use the add_vertex() and add_[face/triangle/quad]() functions of SurfaceMesh. You can only choose
    //              this option if you are sure that the mesh is manifold.
    //  - Option 2: use the SurfaceMeshBuilder that can resolve non-manifoldness during the construction of a mesh. This
    //              is the default option in Easy3D and client code is highly recommended to use SurfaceMeshBuilder.

    // You can easily change an option.
    const int option = 2;

    // In this example, we create a surface mesh representing a tetrahedron (i.e., 4 triangle faces, 4 vertices).
    //
    //                 v0
    //                /|\
    //               / | \
    //              /  |  \
    //          v1 /_ _|_ _\ v2
    //             \   |   /
    //              \  |  /
    //               \ | /
    //                 v3
    //
    const std::vector<vec3> points = {
            vec3(0, 0, 0),
            vec3(1, 0, 0),
            vec3(0, 1, 0),
            vec3(0, 0, 1)
    };

    // Create a surface mesh
    SurfaceMesh mesh;

    if (option == 1) {  // Option 1: use the built-in functions of SurfaceMesh.
        // Add vertices
        SurfaceMesh::Vertex v0 = mesh.add_vertex(points[0]);
        SurfaceMesh::Vertex v1 = mesh.add_vertex(points[1]);
        SurfaceMesh::Vertex v2 = mesh.add_vertex(points[2]);
        SurfaceMesh::Vertex v3 = mesh.add_vertex(points[3]);
        // Add faces
        mesh.add_triangle(v0, v1, v3);
        mesh.add_triangle(v1, v2, v3);
        mesh.add_triangle(v2, v0, v3);
        mesh.add_triangle(v0, v2, v1);
    }

    else if (option == 2) { // Option 2: use SurfaceMeshBuilder.
        // Add vertices
        SurfaceMeshBuilder builder(&mesh);
        builder.begin_surface();
        SurfaceMesh::Vertex v0 = builder.add_vertex(vec3(0, 0, 0));
        SurfaceMesh::Vertex v1 = builder.add_vertex(vec3(1, 0, 0));
        SurfaceMesh::Vertex v2 = builder.add_vertex(vec3(0, 1, 0));
        SurfaceMesh::Vertex v3 = builder.add_vertex(vec3(0, 0, 1));
        // Add faces
        builder.add_triangle(v0, v1, v3);
        builder.add_triangle(v1, v2, v3);
        builder.add_triangle(v2, v0, v3);
        builder.add_triangle(v0, v2, v1);
        builder.end_surface(false);
    }
    else
        LOG(ERROR) << "option must be 1 or 2";

    std::cout << "#face:   " << mesh.n_faces() << std::endl;
	std::cout << "#vertex: " << mesh.n_vertices() << std::endl;
	std::cout << "#edge:   " << mesh.n_edges() << std::endl;

    return EXIT_SUCCESS;
}

