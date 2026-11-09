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

#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_112_PolyMesh/main.cpp
 *
 * \brief Constructing a PolyMesh (a single tetrahedron) — a volumetric mesh made of 3D cells.
 *
 * <b>Overview</b>
 * This console tutorial builds the simplest polyhedral mesh: one tetrahedral cell from four
 * vertices. It then prints the element counts, the position of every vertex, and the corner
 * indices of the cell. It is the entry point to the PolyMesh series (113 connectivity, 114
 * properties, 115 I/O) and the 3D analogue of Tutorial_104_SurfaceMesh.
 *
 * <b>The technique</b>
 * PolyMesh is a half-face data structure for polyhedral (volumetric) meshes. It lifts the
 * halfedge idea of a SurfaceMesh one dimension up: the elements are vertices, edges, halffaces,
 * faces and cells. A cell is a solid volume (a tetrahedron, hexahedron, or general polyhedron)
 * bounded by polygonal faces; each face is represented by two oppositely oriented halffaces, one
 * per adjacent cell, with normals pointing outward from their cell. A face on the boundary of the
 * volume has a halfface whose opposite is invalid (the "outside"). This is the key difference
 * from a SurfaceMesh, whose faces are 2D patches living in 3D space: a PolyMesh fills the interior
 * with 3D cells, which is what volumetric simulation (FEM), tetrahedralization and solid modeling
 * need. A single tetrahedron has 4 vertices, 6 edges, 4 triangular faces and 1 cell.
 * add_tetra(v0,v1,v2,v3) creates that cell together with its faces and halffaces in one call.
 *
 * <b>Easy3D API</b>
 * - PolyMesh: the volumetric mesh; add_vertex(pos) returns a Vertex, add_tetra(...) returns a Cell.
 * - n_vertices()/n_edges()/n_faces()/n_cells(): the four element counts.
 * - vertices()/cells(): ranges over all vertices / cells; vertices(c): the corners of a cell.
 * - position(v): the vec3 coordinates of vertex v.
 *
 * <b>Running it</b>
 * Console-only. It prints "#cell: 1", "#face: 4", "#vertex: 4", "#edge: 6", then a line with each
 * vertex's coordinates and a line listing the cell's vertex indices. The mesh is a local object
 * and is freed when main returns.
 *
 * \see PolyMesh, Tutorial_113_PolyMesh_Connectivity, Tutorial_114_PolyMesh_Property, Tutorial_104_SurfaceMesh
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create an empty polyhedral mesh. Unlike a SurfaceMesh it will hold 3D cells (volumes).
    PolyMesh mesh;

    // Add four vertices
    auto v0 = mesh.add_vertex(vec3(-1.0, 0.0, 0.0));
    auto v1 = mesh.add_vertex(vec3(0.0, 0.0, 1.0));
    auto v2 = mesh.add_vertex(vec3(1.0, 0.0, 0.0));
    auto v3 = mesh.add_vertex(vec3(0.0, 0.0, -1.0));

    // Add the only tetrahedron. add_tetra() builds the cell plus its 4 triangular faces and the
    // outward-oriented halffaces in one call, returning a Cell handle.
    mesh.add_tetra(v0, v1, v2, v3);

    // Report the topology: one tetrahedral cell has 4 faces, 4 vertices and 6 edges.
    std::cout << "#cell:   " << mesh.n_cells() << std::endl;
    std::cout << "#face:   " << mesh.n_faces() << std::endl;
    std::cout << "#vertex: " << mesh.n_vertices() << std::endl;
    std::cout << "#edge:   " << mesh.n_edges() << std::endl;

    // Print positions of vertices to std out
    for (auto v : mesh.vertices())
        std::cout << "coordinates of " << v << ": " << mesh.position(v) << std::endl;

    // For each cell, list the indices of its corner vertices (a tetrahedron has four).
    for (auto c : mesh.cells()) {
        std::cout << "vertex indices of " << c << ": ";
        for (auto v : mesh.vertices(c))
            std::cout << v.idx() << " ";
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
