/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/stop_watch.h>


using namespace easy3d;


// This example shows how to triangulate a general polygonal meshes (e.g., meshes
// with concave faces, self−intersecting faces, and faces with holes) using the
// tessellator of Easy3D.
//
// Note: For general rendering purposes, you can use the tessellator to create a
//       TrianglesDrawable without modifying the mesh.

typedef std::vector<vec3> Hole;

// convert the mesh into triangular mesh using the tessellator
void triangulate(SurfaceMesh *mesh) {
    if (!mesh)
        return;

    mesh->update_face_normals();
    auto normals = mesh->face_property<vec3>("f:normal");
    auto holes = mesh->get_face_property<Hole>("f:holes");

    Tessellator tessellator;
    for (auto f : mesh->faces()) {
        tessellator.begin_polygon(normals[f]);

        tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);
        tessellator.begin_contour();
        for (auto h : mesh->halfedges(f)) {
            SurfaceMesh::Vertex v = mesh->target(h);
            tessellator.add_vertex(mesh->position(v), v.idx());
        }
        tessellator.end_contour();

        if (holes && holes[f].size() >= 3) { // has a valid hole
            tessellator.set_winding_rule(Tessellator::WINDING_ODD);
            tessellator.begin_contour();
            for (auto p : holes[f])
                tessellator.add_vertex(p);
            tessellator.end_contour();
        }

        tessellator.end_polygon();
    }

    // now the tessellation is done. We can clear the old mesh and
    // fill it will the new set of triangles

    mesh->clear();

    const auto& triangles = tessellator.elements();
    if (!triangles.empty()) { // in degenerate cases num can be zero
        const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
        for (auto v : vts) {
            mesh->add_vertex(vec3(v->data()));
        }

        for (const auto& t : triangles) {
            mesh->add_triangle(SurfaceMesh::Vertex(t[0]), SurfaceMesh::Vertex(t[1]), SurfaceMesh::Vertex(t[2]));
        }
    }
}


int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer("Tutorial_307_Tessellator");
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, -1));

    //---------------------- create model -----------------------

#if 1
    SurfaceMesh *mesh = new SurfaceMesh;

    // Face 1: a concave quad
    if (1)
    {
        SurfaceMesh::Vertex v0 = mesh->add_vertex(vec3(0, 0, 0));
        SurfaceMesh::Vertex v1 = mesh->add_vertex(vec3(800, 0, 0));
        SurfaceMesh::Vertex v2 = mesh->add_vertex(vec3(800, 800, 0));
        SurfaceMesh::Vertex v3 = mesh->add_vertex(vec3(600, 300, 0));
        mesh->add_quad(v0, v1, v2, v3);
    }

    // Face 2: a self-intersecting face (a star)
    if (1)
    {
        std::vector<SurfaceMesh::Vertex> vertices = {
                mesh->add_vertex(vec3(1500, 0, 0)),
                mesh->add_vertex(vec3(1300, 800, 0)),
                mesh->add_vertex(vec3(1100, 0, 0)),
                mesh->add_vertex(vec3(1700, 500, 0)),
                mesh->add_vertex(vec3(900, 500, 0))
        };
        mesh->add_face(vertices);
    }

    // Face 3: a quad face with a hole
    if (1)
    {
        std::vector<SurfaceMesh::Vertex> vertices = {
                mesh->add_vertex(vec3(1800, 0, 0)),
                mesh->add_vertex(vec3(2200, 0, 0)),
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

#else
    // Read a mesh and then tessellate it into a triangular mesh.
    const std::string file_name = resource::directory() + "/data/quad_mesh/P.off";
    SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
    if (!mesh) {
        LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }
#endif

    //-------- Triangulate the mesh using the tessellator ---------

    StopWatch w;
    std::cout << "tessellating ..."<< std::endl;
    triangulate(mesh);
    std::cout << "done. time: " << w.time_string() << std::endl;

    // ------------------------------------------------------------

    // ------------------------------------------------------------

    // add the model to the viewer
    viewer.add_model(mesh, true);

    // show the edges
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);
    // also show the borders
    mesh->renderer()->get_lines_drawable("borders")->set_visible(true);

    // Run the viewer
    return viewer.run();
}

