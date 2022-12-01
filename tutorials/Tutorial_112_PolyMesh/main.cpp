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


// This example shows how to
//		- construct a polyhedral mesh (consisting of a single tetrahedron) from its vertices and known connectivity


int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create mesh object
    PolyMesh mesh;

    // Add four vertices
    auto v0 = mesh.add_vertex(vec3(-1.0, 0.0, 0.0));
    auto v1 = mesh.add_vertex(vec3(0.0, 0.0, 1.0));
    auto v2 = mesh.add_vertex(vec3(1.0, 0.0, 0.0));
    auto v3 = mesh.add_vertex(vec3(0.0, 0.0, -1.0));

    // Add the only tetrahedron
    mesh.add_tetra(v0, v1, v2, v3);

    std::cout << "#cell:   " << mesh.n_cells() << std::endl;
    std::cout << "#face:   " << mesh.n_faces() << std::endl;
    std::cout << "#vertex: " << mesh.n_vertices() << std::endl;
    std::cout << "#edge:   " << mesh.n_edges() << std::endl;

    // Print positions of vertices to std out
    for (auto v : mesh.vertices())
        std::cout << "coordinates of " << v << ": " << mesh.position(v) << std::endl;

    for (auto c : mesh.cells()) {
        std::cout << "vertex indices of " << c << ": ";
        for (auto v : mesh.vertices(c))
            std::cout << v.idx() << " ";
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}

