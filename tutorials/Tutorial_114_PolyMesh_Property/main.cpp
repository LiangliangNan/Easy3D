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


// This example shows how to create and access properties defined on a polyhedral mesh (consisting of a single tetrahedron).
// We use per-face properties as example, you should be able to do similarly for per-edge/vertex/cell properties as well.


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


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

    // Create mesh object
    PolyMesh *mesh = old_mesh_from_previous_example();

	// We add a per-face property "f:normal" storing the normal of each face
    PolyMesh::FaceProperty<vec3> center = mesh->add_face_property<vec3>("f:center");

	// for each face, we compute the face center.
	for (auto f : mesh->faces()) {
        vec3 c(0, 0, 0);
        for (auto v : mesh->vertices(f))
            c += mesh->position(v);
        center[f] = c / mesh->vertices(f).size();
	}

    // now we print the face center for each face.
    for (auto f : mesh->faces()) {
        std::cout << "center of " << f << ": " << center[f] << std::endl;
    }

	delete mesh;

    return EXIT_SUCCESS;
}

