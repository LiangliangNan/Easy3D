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

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_104_SurfaceMesh/main.cpp
 *
 * \brief Constructing a SurfaceMesh (a tetrahedron) from vertices and face connectivity, two ways.
 *
 * <b>Overview</b>
 * This is the first surface-mesh tutorial. Where a PointCloud is just a bag of points, a
 * SurfaceMesh also records how the points are connected into faces. We build the simplest closed
 * surface — a tetrahedron with 4 vertices and 4 triangular faces — and print its element counts.
 * The same mesh is built twice, once with each of Easy3D's two construction APIs, so you can
 * compare them. It is console-only; later tutorials (105/106/107) reuse this tetrahedron.
 *
 * <b>The technique</b>
 * SurfaceMesh is a halfedge data structure for orientable 2-manifold polygonal meshes. Every
 * edge is split into two oppositely oriented halfedges, and each halfedge stores links to its
 * target vertex, its opposite twin, the next halfedge around the face, and the face it bounds.
 * Those pointers make adjacency queries (neighbors of a vertex, faces around an edge, ...) cheap
 * and local — see Tutorial_105_SurfaceMesh_Connectivity. The structure assumes a manifold: each
 * edge is shared by exactly two faces (a boundary edge pairs with a null face) and the faces
 * around a vertex form a single fan. Because of that assumption there are two ways to build a
 * mesh: (1) call SurfaceMesh::add_vertex()/add_triangle() directly, which is only safe when you
 * already know the input is manifold; or (2) use SurfaceMeshBuilder, which detects and repairs
 * non-manifold configurations as it goes and is the recommended default. Faces must be given a
 * consistent winding (counter-clockwise seen from outside) so the surface stays orientable. A
 * tetrahedron has V=4, F=4 and E=6, satisfying Euler's formula V - E + F = 2 for a closed sphere-
 * like surface.
 *
 * <b>Easy3D API</b>
 * - SurfaceMesh: the halfedge mesh; add_vertex(pos) returns a Vertex handle, add_triangle(a,b,c)
 *   adds an oriented face; n_vertices()/n_edges()/n_faces() report the element counts.
 * - SurfaceMeshBuilder: wraps a SurfaceMesh and is driven by begin_surface() ... add_vertex() /
 *   add_triangle() ... end_surface(); it guarantees a 2-manifold result.
 * - vec3: the single-precision 3D position type used for the four corners.
 *
 * <b>Running it</b>
 * Console-only. With the default option (2) it prints "#face: 4", "#vertex: 4", "#edge: 6".
 * Change the 'option' variable to 1 to build the same mesh through the direct SurfaceMesh API
 * and observe identical counts. The mesh is a local object, so it is freed when main returns.
 *
 * \see SurfaceMesh, SurfaceMeshBuilder, Tutorial_105_SurfaceMesh_Connectivity, Tutorial_106_SurfaceMesh_Property
 */

using namespace easy3d;

int main(int argc, char **argv) {
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
    const std::vector<vec3> points = {vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)};

    // Create a surface mesh
    SurfaceMesh mesh;

    if (option == 1) { // Option 1: use the built-in functions of SurfaceMesh.
        // Add vertices
        SurfaceMesh::Vertex v0 = mesh.add_vertex(points[0]);
        SurfaceMesh::Vertex v1 = mesh.add_vertex(points[1]);
        SurfaceMesh::Vertex v2 = mesh.add_vertex(points[2]);
        SurfaceMesh::Vertex v3 = mesh.add_vertex(points[3]);
        // Add faces. Each triangle lists its corners in a consistent (outward) winding; the
        // direct API trusts you that the result is manifold and does no repair.
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
        // Finalize the connectivity. The 'false' argument suppresses the log of any non-manifold
        // issues the builder had to repair (there are none for this clean tetrahedron).
        builder.end_surface(false);
    } else
        std::cerr << "option must be 1 or 2" << std::endl;

    // Report the topology: a tetrahedron has 4 faces, 4 vertices and 6 edges (V - E + F = 2).
    std::cout << "#face:   " << mesh.n_faces() << std::endl;
    std::cout << "#vertex: " << mesh.n_vertices() << std::endl;
    std::cout << "#edge:   " << mesh.n_edges() << std::endl;

    return EXIT_SUCCESS;
}
