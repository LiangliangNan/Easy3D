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
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>


using namespace easy3d;


int test_surface_mesh() {

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

    // construct a mesh from its vertices and known connectivity
    {
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
        } else if (option == 2) { // Option 2: use SurfaceMeshBuilder.
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
        } else
            LOG(ERROR) << "option must be 1 or 2";

        std::cout << "#face:   " << mesh.n_faces() << std::endl;
        std::cout << "#vertex: " << mesh.n_vertices() << std::endl;
        std::cout << "#edge:   " << mesh.n_edges() << std::endl;
    }


    // This example shows how to access the adjacency information of a surface mesh, i.e.,
    //		- the incident vertices of each vertex
    //		- the incident outgoing/ingoing edges of each vertex
    //		- the incident faces of each vertex
    //		- the incident vertices of each face
    //		- the incident half-edges of each face
    //		- the two end points of each edge;
    //		- the two faces connected by each edge

    // There are two ways to traverse the incident entities of an element.
    //   - use a "for" loop (cleaner code);
    //   - use a circulator.
#define USE_FOR_LOOP
    {
        std::cout << "----------------------------------------\n";
        std::cout << "The incident vertices of each vertex" << std::endl;
        std::cout << "----------------------------------------\n";

        // loop over all vertices
        for (auto v : mesh.vertices()) {
            std::cout << "incident vertices of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
            // loop over all incident vertices
            for (auto vv : mesh.vertices(v))
                std::cout << vv << " ";
#else   // use circulator
            SurfaceMesh::VertexAroundVertexCirculator cir = mesh.vertices(v);
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
        for (auto v : mesh.vertices()) {
            std::cout << "incident outgoing/ingoing edges of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
            // loop over all incident outgoing edges
            for (auto h : mesh.halfedges(v))
                std::cout << h << "/" << mesh.opposite(h) << " ";
#else   // use circulator
            SurfaceMesh::HalfedgeAroundVertexCirculator cir = mesh.halfedges(v);
            SurfaceMesh::HalfedgeAroundVertexCirculator end = cir;
            do {
                SurfaceMesh::Halfedge h = *cir;
                std::cout << h << "/" << mesh.opposite(h) << " ";
                ++cir;
            } while (cir != end);
#endif
            std::cout << std::endl;
        }

        std::cout << "\n--------------------------------------\n";
        std::cout << "The incident faces of each vertex" << std::endl;
        std::cout << "----------------------------------------\n";

        // loop over all vertices
        for (auto v : mesh.vertices()) {
            std::cout << "incident faces of vertex " << v << ": ";
#ifdef USE_FOR_LOOP
            // loop over all incident faces
            for (auto f : mesh.faces(v))
                std::cout << f << " ";
#else   // use circulator
            SurfaceMesh::FaceAroundVertexCirculator cir = mesh.faces(v);
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
        for (auto f : mesh.faces()) {
            std::cout << "incident vertices of face " << f << ": ";
#ifdef USE_FOR_LOOP
            // loop over all incident vertices
            for (auto v : mesh.vertices(f))
                std::cout << v << " ";
#else   // use circulator
            SurfaceMesh::VertexAroundFaceCirculator cir = mesh.vertices(f);
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
        for (auto f : mesh.faces()) {
            std::cout << "half-edges around face " << f << ": ";
#ifdef USE_FOR_LOOP
            // loop over all half-edges around the face
            for (auto h : mesh.halfedges(f))
                std::cout << h << " ";
#else
            SurfaceMesh::HalfedgeAroundFaceCirculator cir = mesh.halfedges(f);
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
        for (auto e : mesh.edges()) {
            std::cout << "the two end points of edge " << e << ": ";
            SurfaceMesh::Vertex vs = mesh.vertex(e, 0);
            std::cout << vs << " ";
            SurfaceMesh::Vertex vt = mesh.vertex(e, 1);
            std::cout << vt << " " << std::endl;
        }

        std::cout << "\n--------------------------------------\n";
        std::cout << "The two faces connected by each edge" << std::endl;
        std::cout << "----------------------------------------\n";

        // loop over all edges
        for (auto e : mesh.edges()) {
            std::cout << "the two faces connected by edge " << e << ": ";
            SurfaceMesh::Halfedge h0 = mesh.halfedge(e, 0);
            if (mesh.is_border(h0))
                std::cout << "NULL" << " ";
            else
                std::cout << mesh.face(h0) << " ";

            SurfaceMesh::Halfedge h1 = mesh.halfedge(e, 1);
            if (mesh.is_border(h1))
                std::cout << "NULL" << " ";
            else
                std::cout << mesh.face(h1) << " ";

            std::cout << std::endl;
        }
    }


    // This example shows how to create and access properties defined on a surface mesh.
    // We use per-face properties as example, you should be able to do similarly for per-edge/vertex properties also.
    {
        // We add a per-face property "f:normal" storing the normal of each face
        SurfaceMesh::FaceProperty<vec3> normals = mesh.add_face_property<vec3>("f:normal");

        // for each face, we access the face normal and print it.
        for (auto f : mesh.faces()) {
            // We use the built-in function of SurfaceMesh compute_face_normal().
            // Of course you can write your own function to compute the normal of
            // a face (the normalized cross product of two consecutive edge vectors).
            normals[f] = mesh.compute_face_normal(f);
            std::cout << "normal of face " << f << ": " << normals[f] << std::endl;
        }
    }

    //		- load a surface mesh from a file;
    //		- save a surface mesh into a file.
    {
        // Read a mesh specified by its file name
        const std::string file_name = resource::directory() + "/data/sphere.obj";
        SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
        if (!mesh) {
            LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
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
        const std::string save_file_name = "./sphere-copy.obj";
        if (SurfaceMeshIO::save(save_file_name, mesh))
            std::cout << "mesh saved to \'" << save_file_name << "\'"  << std::endl;
        else
            std::cerr << "failed create the new file" << std::endl;

        if (file_system::delete_file(save_file_name))
            std::cout << "the saved file has been deleted"  << std::endl;
        else
            std::cerr << "failed to delete the saved file" << std::endl;
    }

    return EXIT_SUCCESS;
}

