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
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>


using namespace easy3d;


int test_polyhedral_mesh() {
    // Create mesh object
    PolyMesh mesh;

    // construct a polyhedral mesh (consisting of a single tetrahedron) from its vertices and known connectivity
    {
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
    }

    // the adjacency information of a polyhedral mesh (consisting of a single tetrahedron),
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
    {
        std::cout << "----------------------------------------\n";
        std::cout << "The incident vertices of each vertex" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all vertices
        for (auto v : mesh.vertices()) {
            std::cout << "incident vertices of vertex " << v << ": ";
            // loop over all incident vertices
            for (auto vv : mesh.vertices(v))
                std::cout << vv << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident edges of each vertex" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all vertices
        for (auto v : mesh.vertices()) {
            std::cout << "incident edges of vertex " << v << ": ";
            // loop over all incident edges
            for (auto e : mesh.edges(v))
                std::cout << e << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident halffaces of each vertex" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all vertices
        for (auto v : mesh.vertices()) {
            std::cout << "incident halffaces of vertex " << v << ": ";
            // loop over all incident edges
            for (auto h : mesh.halffaces(v))
                std::cout << h << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident cells of each vertex" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all vertices
        for (auto v : mesh.vertices()) {
            std::cout << "incident cells of vertex " << v << ": ";
            // loop over all incident cells
            for (auto c : mesh.cells(v))
                std::cout << c << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident vertices of each edge" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all edges
        for (auto e : mesh.edges()) {
            std::cout << "incident vertices of edge " << e << ": " << mesh.vertex(e, 0) << " " << mesh.vertex(e, 1)
                      << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident halffaces of each edge" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all edges
        for (auto e : mesh.edges()) {
            std::cout << "incident halffaces of edge " << e << ": ";
            // loop over all incident halffaces
            for (auto h : mesh.halffaces(e))
                std::cout << h << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident cells of each edge" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all edges
        for (auto e : mesh.edges()) {
            std::cout << "incident cells of edge " << e << ": ";
            // loop over all incident cells
            for (auto c : mesh.cells(e))
                std::cout << c << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident vertices of each halfface" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all halffaces
        for (auto h : mesh.halffaces()) {
            std::cout << "incident vertices of halfface " << h << ": ";
            // loop over all incident cells
            for (auto v : mesh.vertices(h))
                std::cout << v << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident edges of each halfface" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all halffaces
        for (auto h : mesh.halffaces()) {
            std::cout << "incident edges of halfface " << h << ": ";
            // loop over all incident edges
            for (auto e : mesh.edges(h))
                std::cout << e << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The associated cell of each halfface" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all halffaces
        for (auto h : mesh.halffaces()) {
            std::cout << "incident associated cell of halfface " << h << ": " << mesh.cell(h) << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The opposite halfface and cell of each halfface" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all halffaces
        for (auto h : mesh.halffaces()) {
            std::cout << "opposite halfface of halfface " << h << ": " << mesh.opposite(h) << ". ";
            std::cout << "opposite cell of halfface " << h << ": " << mesh.cell(mesh.opposite(h)) << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident vertices of each cell" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all cells
        for (auto c : mesh.cells()) {
            std::cout << "incident vertices of cell " << c << ": ";
            // loop over all incident vertices
            for (auto v : mesh.vertices(c))
                std::cout << v << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident edges of each cell" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all cells
        for (auto c : mesh.cells()) {
            std::cout << "incident edges of cell " << c << ": ";
            // loop over all incident edges
            for (auto e : mesh.edges(c))
                std::cout << e << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The incident halffaces of each cell" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all cells
        for (auto c : mesh.cells()) {
            std::cout << "incident halffaces of cell " << c << ": ";
            // loop over all incident halffaces
            for (auto h : mesh.halffaces(c))
                std::cout << h << " ";
            std::cout << std::endl;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "The two halffaces of each face" << std::endl;
        std::cout << "----------------------------------------\n";
        // loop over all faces
        for (auto f : mesh.faces()) {
            std::cout << "incident halffaces of face " << f << ": " << mesh.halfface(f, 0) << " "
                      << mesh.halfface(f, 1)
                      << std::endl;
        }
    }


    // This example shows how to create and access properties defined on a polyhedral mesh (consisting of a single tetrahedron).
    // We use per-face properties as example, you should be able to do similarly for per-edge/vertex/cell properties as well.
    {
        // We add a per-face property "f:normal" storing the normal of each face
        PolyMesh::FaceProperty<vec3> center = mesh.add_face_property<vec3>("f:center");

        // for each face, we compute the face center.
        for (auto f : mesh.faces()) {
            vec3 c(0, 0, 0);
            for (auto v : mesh.vertices(f))
                c += mesh.position(v);
            center[f] = c / mesh.vertices(f).size();
        }

        // now we print the face center for each face.
        for (auto f : mesh.faces()) {
            std::cout << "center of " << f << ": " << center[f] << std::endl;
        }
    }


    //		- load a polyhedral mesh from a file;
    //		- save a polyhedral mesh into a file;
    {
        // Read a mesh specified by its file name
        const std::string file_name = resource::directory() + "/data/sphere.plm";
        PolyMesh* mesh = PolyMeshIO::load(file_name);
        if (!mesh) {
            LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
            return EXIT_FAILURE;
        }
        std::cout << "mesh loaded. " << std::endl;
        std::cout << "\tvertices: " << mesh->n_vertices() << std::endl;
        std::cout << "\tedges: " << mesh->n_edges() << std::endl;
        std::cout << "\tfaces: " << mesh->n_faces() << std::endl;
        std::cout << "\tcells: " << mesh->n_cells() << std::endl;

        // ...
        // Do fancy stuff with the mesh
        // ...

        // Write the mesh to a new file.
        const std::string save_file_name = "./sphere-copy.plm";
        if (PolyMeshIO::save(save_file_name, mesh))
            std::cout << "mesh saved to \'" << save_file_name << "\'"  << std::endl;
        else
            std::cerr << "failed create the new file" << std::endl;

        if (file_system::delete_file(save_file_name))
            std::cout << "the saved file has been deleted"  << std::endl;
        else
            std::cerr << "failed to delete the saved file" << std::endl;

        // delete the mesh (i.e., release memory)
        delete mesh;
    }

    return EXIT_SUCCESS;
}

