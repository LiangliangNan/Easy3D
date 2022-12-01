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

#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;


// This example shows how to access the adjacency information of a polyhedral mesh (consisting of a single tetrahedron),
// i.e.,
//		- the incident vertices of each vertex
//		- the incident edges of each vertex
//		- the incident halffaces of each vertex
//		- the incident cells of each vertex
//		- the incident vertices of each edge
//		- the incident halffaces of each edge
//		- the incident cells of each edge
//		- the incident vertices of each halfface
//		- the incident edges of each halfface
//		- the associated cell of each halfface
//		- the opposite cell of each halfface
//		- the incident vertices of each cell
//		- the incident edges of each cell
//		- the incident halffaces of each cell
//		- the two halffaces of each face


// the mesh created in the previous tutorial (so you can skip it)
PolyMesh *old_mesh_from_previous_example() {
    // Create a polyhedral mesh
    auto mesh = new PolyMesh;

    // Add four vertices
    auto v0 = mesh->add_vertex(vec3(-1.0, 0.0, 0.0));
    auto v1 = mesh->add_vertex(vec3(0.0, 0.0, 1.0));
    auto v2 = mesh->add_vertex(vec3(1.0, 0.0, 0.0));
    auto v3 = mesh->add_vertex(vec3(0.0, 0.0, -1.0));

    // Add the only tetrahedron
    mesh->add_tetra(v0, v1, v2, v3);

    return mesh;
}

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create mesh object
    PolyMesh *mesh = old_mesh_from_previous_example();

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident vertices of vertex " << v << ": ";
        // loop over all incident vertices
        for (auto vv : mesh->vertices(v))
            std::cout << vv << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident edges of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident edges of vertex " << v << ": ";
        // loop over all incident edges
        for (auto e : mesh->edges(v))
            std::cout << e << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident halffaces of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident halffaces of vertex " << v << ": ";
        // loop over all incident edges
        for (auto h : mesh->halffaces(v))
            std::cout << h << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident cells of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident cells of vertex " << v << ": ";
        // loop over all incident cells
        for (auto c : mesh->cells(v))
            std::cout << c << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each edge" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all edges
    for (auto e : mesh->edges()) {
        std::cout << "incident vertices of edge " << e << ": " << mesh->vertex(e, 0) << " " << mesh->vertex(e, 1)
                  << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident halffaces of each edge" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all edges
    for (auto e : mesh->edges()) {
        std::cout << "incident halffaces of edge " << e << ": ";
        // loop over all incident halffaces
        for (auto h : mesh->halffaces(e))
            std::cout << h << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident cells of each edge" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all edges
    for (auto e : mesh->edges()) {
        std::cout << "incident cells of edge " << e << ": ";
        // loop over all incident cells
        for (auto c : mesh->cells(e))
            std::cout << c << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each halfface" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all halffaces
    for (auto h : mesh->halffaces()) {
        std::cout << "incident vertices of halfface " << h << ": ";
        // loop over all incident cells
        for (auto v : mesh->vertices(h))
            std::cout << v << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident edges of each halfface" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all halffaces
    for (auto h : mesh->halffaces()) {
        std::cout << "incident edges of halfface " << h << ": ";
        // loop over all incident edges
        for (auto e : mesh->edges(h))
            std::cout << e << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The associated cell of each halfface" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all halffaces
    for (auto h : mesh->halffaces()) {
        std::cout << "incident associated cell of halfface " << h << ": " << mesh->cell(h) << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The opposite halfface and cell of each halfface" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all halffaces
    for (auto h : mesh->halffaces()) {
        std::cout << "opposite halfface of halfface " << h << ": " << mesh->opposite(h) << ". ";
        std::cout << "opposite cell of halfface " << h << ": " << mesh->cell(mesh->opposite(h)) << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each cell" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all cells
    for (auto c : mesh->cells()) {
        std::cout << "incident vertices of cell " << c << ": ";
        // loop over all incident vertices
        for (auto v : mesh->vertices(c))
            std::cout << v << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident edges of each cell" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all cells
    for (auto c : mesh->cells()) {
        std::cout << "incident edges of cell " << c << ": ";
        // loop over all incident edges
        for (auto e : mesh->edges(c))
            std::cout << e << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The incident halffaces of each cell" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all cells
    for (auto c : mesh->cells()) {
        std::cout << "incident halffaces of cell " << c << ": ";
        // loop over all incident halffaces
        for (auto h : mesh->halffaces(c))
            std::cout << h << " ";
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------\n";
    std::cout << "The two halffaces of each face" << std::endl;
    std::cout << "----------------------------------------\n";
    // loop over all faces
    for (auto f : mesh->faces()) {
        std::cout << "incident halffaces of face " << f << ": " << mesh->halfface(f, 0) << " " << mesh->halfface(f, 1)
                  << std::endl;
    }

    return EXIT_SUCCESS;
}

