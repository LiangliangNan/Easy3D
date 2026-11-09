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
 * \example{lineno} Tutorial_105_SurfaceMesh_Connectivity/main.cpp
 *
 * \brief Walking the adjacency (connectivity) of a SurfaceMesh through its halfedge links.
 *
 * <b>Overview</b>
 * Using the tetrahedron from Tutorial_104_SurfaceMesh, this console tutorial prints every common
 * adjacency relationship a geometry algorithm needs:
 *      - the incident vertices of each vertex
 *      - the incident outgoing/ingoing edges (halfedges) of each vertex
 *      - the incident faces of each vertex
 *      - the incident vertices of each face
 *      - the incident half-edges of each face
 *      - the two end points of each edge
 *      - the two faces connected by each edge
 * Each query is shown twice — once with a clean range-based for loop and once with an explicit
 * circulator — selected by the USE_FOR_LOOP macro, so you can see both traversal styles.
 *
 * <b>The technique</b>
 * In a halfedge data structure every edge is two directed halfedges. A halfedge knows its target
 * vertex, its opposite twin, the next halfedge around its face, and the face it bounds; a vertex
 * and a face each remember one incident halfedge. From those few links you can "circulate"
 * around an element: stepping next() walks the boundary of a face, while opposite() then next()
 * rotates around a vertex. That is how the library answers neighbor queries in time proportional
 * to the number of neighbors, without scanning the whole mesh. A circulator is an iterator that
 * wraps around a closed fan (it has no natural end, so you stop when it returns to the start);
 * the range-based for wrappers hide that bookkeeping. Note the boundary case: an edge on the
 * border has a halfedge whose face is null, which we detect with is_border() and print as NULL.
 *
 * <b>Easy3D API</b>
 * - SurfaceMesh::vertices(v) / halfedges(v) / faces(v): the neighbors of a vertex.
 * - SurfaceMesh::vertices(f) / halfedges(f): the corners and boundary halfedges of a face.
 * - SurfaceMesh::vertex(e, i) and halfedge(e, i): the i-th (0 or 1) endpoint / halfedge of an edge.
 * - SurfaceMesh::opposite(h), face(h), is_border(h): cross to the twin halfedge, its face, or test
 *   whether that face is the null boundary face.
 * - The *Circulator types (e.g. VertexAroundVertexCirculator) are the explicit traversal form.
 *
 * <b>Running it</b>
 * Console-only. It prints a titled section for each of the seven queries above, listing the
 * incident element handles (e.g. "v0", "h1", "f2") for every vertex, face and edge of the
 * tetrahedron. Border faces print "NULL". The mesh is heap-allocated and deleted at the end.
 *
 * \see SurfaceMesh, Tutorial_104_SurfaceMesh, Tutorial_109_Graph_Connectivity, Tutorial_113_PolyMesh_Connectivity
 */

using namespace easy3d;

// There are two ways to traverse the incident entities of an element.
//   - use a "for" loop (cleaner code);
//   - use a circulator.
#define USE_FOR_LOOP

// the mesh created in the previous tutorial (so you can skip it)
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

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident vertices of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
        // loop over all incident vertices
        for (auto vv : mesh->vertices(v))
            std::cout << vv << " ";
#else // use circulator
        // A circulator wraps around a closed fan and has no natural end, so we save the start
        // position and stop once advancing brings us back to it.
        SurfaceMesh::VertexAroundVertexCirculator cir = mesh->vertices(v);
        SurfaceMesh::VertexAroundVertexCirculator end = cir;
        do {
            SurfaceMesh::Vertex vv = *cir;
            std::cout << vv << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The incident outgoing/ingoing edges of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident outgoing/ingoing edges of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
        // loop over all incident outgoing edges
        for (auto h : mesh->halfedges(v))
            std::cout << h << "/" << mesh->opposite(h) << " ";
#else // use circulator
        SurfaceMesh::HalfedgeAroundVertexCirculator cir = mesh->halfedges(v);
        SurfaceMesh::HalfedgeAroundVertexCirculator end = cir;
        do {
            SurfaceMesh::Halfedge h = *cir;
            std::cout << h << "/" << mesh->opposite(h) << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The incident faces of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all vertices
    for (auto v : mesh->vertices()) {
        std::cout << "incident faces of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
        // loop over all incident faces
        for (auto f : mesh->faces(v))
            std::cout << f << " ";
#else // use circulator
        SurfaceMesh::FaceAroundVertexCirculator cir = mesh->faces(v);
        SurfaceMesh::FaceAroundVertexCirculator end = cir;
        do {
            SurfaceMesh::Face f = *cir;
            std::cout << f << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The incident vertices of each face" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all faces
    for (auto f : mesh->faces()) {
        std::cout << "incident vertices of face " << f << ": ";
#ifdef USE_FOR_LOOP
        // loop over all incident vertices
        for (auto v : mesh->vertices(f))
            std::cout << v << " ";
#else // use circulator
        SurfaceMesh::VertexAroundFaceCirculator cir = mesh->vertices(f);
        SurfaceMesh::VertexAroundFaceCirculator end = cir;
        do {
            SurfaceMesh::Vertex v = *cir;
            std::cout << v << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The incident half-edges of each face" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all faces
    for (auto f : mesh->faces()) {
        std::cout << "half-edges around face " << f << ": ";
#ifdef USE_FOR_LOOP
        // loop over all half-edges around the face
        for (auto h : mesh->halfedges(f))
            std::cout << h << " ";
#else
        SurfaceMesh::HalfedgeAroundFaceCirculator cir = mesh->halfedges(f);
        SurfaceMesh::HalfedgeAroundFaceCirculator end = cir;
        do {
            SurfaceMesh::Halfedge h = *cir;
            std::cout << h << " ";
            ++cir;
        } while (cir != end);
#endif
        std::cout << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The two end points of each edge" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all edges
    for (auto e : mesh->edges()) {
        std::cout << "the two end points of edge " << e << ": ";
        SurfaceMesh::Vertex vs = mesh->vertex(e, 0);
        std::cout << vs << " ";
        SurfaceMesh::Vertex vt = mesh->vertex(e, 1);
        std::cout << vt << " " << std::endl;
    }

    std::cout << "\n--------------------------------------\n";
    std::cout << "The two faces connected by each edge" << std::endl;
    std::cout << "----------------------------------------\n";

    // loop over all edges
    for (auto e : mesh->edges()) {
        std::cout << "the two faces connected by edge " << e << ": ";
        // An interior edge borders two real faces; a boundary edge has a null face on one side,
        // which is_border() detects so we can print NULL instead of dereferencing it.
        SurfaceMesh::Halfedge h0 = mesh->halfedge(e, 0);
        if (mesh->is_border(h0))
            std::cout << "NULL" << " ";
        else
            std::cout << mesh->face(h0) << " ";

        SurfaceMesh::Halfedge h1 = mesh->halfedge(e, 1);
        if (mesh->is_border(h1))
            std::cout << "NULL" << " ";
        else
            std::cout << mesh->face(h1) << " ";

        std::cout << std::endl;
    }

    // Delete the mesh (i.e., release memory)
    delete mesh;

    return EXIT_SUCCESS;
}
