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
 * \example{lineno} Tutorial_114_PolyMesh_Property/main.cpp
 *
 * \brief Storing per-element attributes on a PolyMesh (a polyhedral / volumetric mesh).
 *
 * <b>Overview</b>
 * This console tutorial attaches a custom attribute to every face of a polyhedral mesh and reads
 * it back. It reuses the single-tetrahedron mesh built in Tutorial_112_PolyMesh and computes the
 * centroid (geometric center) of each of its four triangular faces. The same pattern works for
 * per-vertex, per-edge, and per-cell properties — only the element type and the "v:" / "e:" /
 * "f:" / "c:" name prefix change.
 *
 * <b>The technique</b>
 * A PolyMesh is Easy3D's volumetric data structure: unlike a SurfaceMesh (which only has a boundary
 * surface), it also represents the interior as 3D cells — here a single tetrahedron. It exposes
 * four element types: vertices, edges, faces, and cells. As with all Easy3D models, attributes are
 * stored as named properties: one contiguous array per attribute, indexed by element handle. The
 * face centroid is simply the arithmetic mean of the face's corner positions,
 * c = (p0 + p1 + ... + pn-1) / n, a basic quantity used everywhere in geometry processing (e.g.,
 * for rendering, integration, or locating a face).
 *
 * <b>Easy3D API</b>
 * - PolyMesh: the polyhedral mesh class. add_vertex() adds a corner; add_tetra(v0,v1,v2,v3) adds a
 *   tetrahedral cell from four vertices.
 * - PolyMesh::add_face_property<T>(name): creates a per-face property of type T and returns a
 *   FaceProperty<T> accessor; values are read/written with center[f].
 * - PolyMesh::faces(): a range of all face handles. PolyMesh::vertices(f): the corner vertices of
 *   face f. PolyMesh::position(v): the 3D coordinates of vertex v.
 *
 * <b>Running it</b>
 * Console-only. It prints one line per face giving the face handle and its computed centroid,
 * e.g. "center of f0: ...". The mesh is heap-allocated and explicitly deleted at the end.
 *
 * \see PolyMesh, Tutorial_112_PolyMesh, Tutorial_113_PolyMesh_Connectivity, Tutorial_106_SurfaceMesh_Property
 */

using namespace easy3d;

// Rebuilds the single-tetrahedron mesh from Tutorial_112_PolyMesh so this example is self-contained.
PolyMesh *old_mesh_from_previous_example() {
    // Create an empty polyhedral mesh.
    auto mesh = new PolyMesh;

    // Add the four corner vertices of the tetrahedron (all lie in the XZ plane, y = 0).
    auto v0 = mesh->add_vertex(vec3(-1.0, 0.0, 0.0));
    auto v1 = mesh->add_vertex(vec3(0.0, 0.0, 1.0));
    auto v2 = mesh->add_vertex(vec3(1.0, 0.0, 0.0));
    auto v3 = mesh->add_vertex(vec3(0.0, 0.0, -1.0));

    // Add the single tetrahedral cell bounded by the four vertices. This also creates its faces.
    mesh->add_tetra(v0, v1, v2, v3);

    return mesh;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create the mesh (the tetrahedron from the previous tutorial).
    PolyMesh *mesh = old_mesh_from_previous_example();

    // Create a per-face property named "f:center" holding one vec3 per face. The "f:" prefix marks
    // it as a face property; add_face_property<T>() returns a typed accessor indexed by face handle.
    PolyMesh::FaceProperty<vec3> center = mesh->add_face_property<vec3>("f:center");

    // For each face, compute its centroid: the average of its corner positions.
    for (auto f : mesh->faces()) {
        vec3 c(0, 0, 0);
        // vertices(f) yields the corner vertices of face f; position(v) gives each corner's xyz.
        for (auto v : mesh->vertices(f))
            c += mesh->position(v);
        center[f] = c / mesh->vertices(f).size(); // divide by the number of corners to get the mean
    }

    // Read the property back: print the centroid stored for each face.
    for (auto f : mesh->faces()) {
        std::cout << "center of " << f << ": " << center[f] << std::endl;
    }

    // Delete the mesh (i.e., release memory).
    delete mesh;

    return EXIT_SUCCESS;
}
