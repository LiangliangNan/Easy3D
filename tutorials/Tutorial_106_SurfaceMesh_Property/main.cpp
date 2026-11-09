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
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_106_SurfaceMesh_Property/main.cpp
 *
 * \brief Storing a per-face attribute (the face normal) on a SurfaceMesh through a named property.
 *
 * <b>Overview</b>
 * This console tutorial applies the property system from Tutorial_102_PointCloud_Property to a
 * surface mesh, using faces instead of points. On the tetrahedron from Tutorial_104 we create a
 * per-face property "f:normal", fill it with each face's normal, and print the results. The same
 * pattern works for per-vertex ("v:") and per-edge ("e:") properties — only the add_*_property /
 * handle type changes.
 *
 * <b>The technique</b>
 * A property is a parallel array with one slot per element, indexed by the element handle, so a
 * FaceProperty<vec3> stores one normal per face. The normal of a planar face is the unit vector
 * perpendicular to its plane. For a triangle with corners p0, p1, p2 it is
 * normalize((p1 - p0) x (p2 - p0)): the cross product of two edge vectors gives a perpendicular
 * whose direction follows the face winding (right-hand rule), and normalizing makes it unit
 * length. SurfaceMesh::compute_face_normal() does this for you and also handles general
 * (non-triangular) polygons robustly. Normals are the basis of lighting, shading, offsetting and
 * many geometry-processing operations, which is why meshes routinely cache them as a property.
 *
 * <b>Easy3D API</b>
 * - SurfaceMesh::add_face_property<T>(name): creates the property and returns a FaceProperty<T>.
 * - SurfaceMesh::faces(): the range of face handles to iterate.
 * - SurfaceMesh::compute_face_normal(f): the built-in unit normal of face f.
 * - vec3: the 3D vector type used to store each normal.
 *
 * <b>Running it</b>
 * Console-only. It prints one line per face, e.g. "normal of face f0: ...", for the four faces of
 * the tetrahedron. The mesh is heap-allocated and deleted at the end to release its memory.
 *
 * \see SurfaceMesh, Tutorial_102_PointCloud_Property, Tutorial_104_SurfaceMesh, Tutorial_114_PolyMesh_Property
 */

using namespace easy3d;

// Rebuilds the tetrahedron from Tutorial_104 so this example is self-contained.
SurfaceMesh *old_mesh_from_previous_example() {
    // Create a surface mesh
    auto mesh = new SurfaceMesh;

    // Add 4 vertices
    SurfaceMesh::Vertex v0 = mesh->add_vertex(vec3(0, 0, 0));
    SurfaceMesh::Vertex v1 = mesh->add_vertex(vec3(1, 0, 0));
    SurfaceMesh::Vertex v2 = mesh->add_vertex(vec3(0, 1, 0));
    SurfaceMesh::Vertex v3 = mesh->add_vertex(vec3(0, 0, 1));

    // Add 4 triangular faces
    mesh->add_triangle(v0, v1, v3);
    mesh->add_triangle(v1, v2, v3);
    mesh->add_triangle(v2, v0, v3);
    mesh->add_triangle(v0, v2, v1);

    return mesh;
}

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create a mesh
    SurfaceMesh *mesh = old_mesh_from_previous_example();

    // We add a per-face property "f:normal" storing the normal of each face
    SurfaceMesh::FaceProperty<vec3> normals = mesh->add_face_property<vec3>("f:normal");

    // for each face, we access the face normal and print it.
    for (auto f : mesh->faces()) {
        // We use the built-in function of SurfaceMesh compute_face_normal().
        // Of course, you can write your own function to compute the normal of
        // a face (the normalized cross product of two consecutive edge vectors).
        normals[f] = mesh->compute_face_normal(f);
        std::cout << "normal of face " << f << ": " << normals[f] << std::endl;
    }

    // Delete the mesh (i.e., release memory)
    delete mesh;

    return EXIT_SUCCESS;
}
