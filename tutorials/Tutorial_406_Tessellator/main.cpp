/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/tessellator_gen.h>
#include <easy3d/util/logging.h>



using namespace easy3d;


// This example shows how to triangulate a general polygonal meshes (i.e., meshes
// with concave faces, self−intersecting faces, and faces with holes) using the
// tessellator of Easy3D.
//
// Note: For general rendering puruposes, you can use the tessellator to create a
//       trianglesDrawable without modifying the mesh.

typedef std::vector<vec3> Hole;

// convert the mesh into triangular mesh using the tessellator
void triangulate(SurfaceMesh* mesh) {
    if (!mesh)
        return;

    mesh->update_face_normals();
    auto normals = mesh->face_property<vec3>("f:normal");
    auto holes = mesh->get_face_property<Hole>("f:holes");

    TessellatorGen gen;
    for (auto f : mesh->faces()) {
        gen.begin_polygon(normals[f]);

        gen.set_winding_rule(TessellatorGen::NONZERO);  // or POSITIVE
        gen.begin_contour();
        for (auto h : mesh->halfedges(f)) {
            SurfaceMesh::Vertex v = mesh->to_vertex(h);
            gen.add_vertex(mesh->position(v));
        }
        gen.end_contour();

        if (holes && holes[f].size() > 3) { // has a valid hole
            gen.set_winding_rule(TessellatorGen::ODD);
            gen.begin_contour();
            for (auto p : holes[f])
                gen.add_vertex(p);
            gen.end_contour();
        }

        gen.end_polygon();
    }

    // now the tessellation is done. We can clear the old mesh and
    // fill it will the new set of triangles

    mesh->clear();

    std::size_t num = gen.num_triangles();
    if (num > 0) { // in degenerate cases num can be zero
        const std::vector<const double*>& vts = gen.get_vertices();
        for (std::size_t i = 0; i < vts.size(); ++i)
            mesh->add_vertex(vec3(vts[i]));

        for (std::size_t i = 0; i < num; ++i) {
            std::size_t a, b, c;
            gen.get_triangle(i, a, b, c);
            mesh->add_triangle(SurfaceMesh::Vertex(a), SurfaceMesh::Vertex(b), SurfaceMesh::Vertex(c));
        }
    }
}


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize(argv[0]);

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_405_Texture");
        viewer.camera()->setUpVector(vec3(0, 1, 0));
        viewer.camera()->setViewDirection(vec3(0, 0, -1));

        //---------------------- create model -----------------------

        SurfaceMesh* mesh = new SurfaceMesh;

        // Face 1: a concave quad
        {
            SurfaceMesh::Vertex v0 = mesh->add_vertex(vec3(  0,    0, 0));
            SurfaceMesh::Vertex v1 = mesh->add_vertex(vec3(800,    0, 0));
            SurfaceMesh::Vertex v2 = mesh->add_vertex(vec3(800,  800, 0));
            SurfaceMesh::Vertex v3 = mesh->add_vertex(vec3(600,  300, 0));
            mesh->add_quad(v0, v1, v2, v3);
        }

        // Face 2: a self-intersecting face (a star)
        {
            std::vector<SurfaceMesh::Vertex> vertices = {
                mesh->add_vertex(vec3(1500,   0, 0)),
                mesh->add_vertex(vec3(1300, 800, 0)),
                mesh->add_vertex(vec3(1100,   0, 0)),
                mesh->add_vertex(vec3(1700, 500, 0)),
                mesh->add_vertex(vec3( 900, 500, 0))
            };
            mesh->add_face(vertices);
        }

        // Face 3: a quad face with a hole
        {
            std::vector<SurfaceMesh::Vertex> vertices = {
                mesh->add_vertex(vec3(1800,   0, 0)),
                mesh->add_vertex(vec3(2200,   0, 0)),
                mesh->add_vertex(vec3(2200, 700, 0)),
                mesh->add_vertex(vec3(1800, 700, 0))
            };
            SurfaceMesh::Face f = mesh->add_face(vertices);

            auto holes = mesh->add_face_property<Hole>("f:holes");
            holes[f] = {
                vec3(1900, 100, 0),
                vec3(2100, 100, 0),
                vec3(2100, 600, 0),
                vec3(1900, 600, 0)
            };
        }

        //-------- Triangulate the mesh using the tessellator ---------

        triangulate(mesh);

        // ------------------------------------------------------------

        viewer.add_model(mesh, true, false);

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << "Caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

