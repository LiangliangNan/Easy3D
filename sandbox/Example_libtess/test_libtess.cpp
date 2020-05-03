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
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/stop_watch.h>

#include <3rd_party/libtess/tesselator.h>

using namespace easy3d;


// This example shows how to triangulate a general polygonal meshes (i.e., meshes
// with concave faces, self−intersecting faces, and faces with holes) using the
// tessellator of Easy3D.
//
// Note: For general rendering purposes, you can use the tessellator to create a
//       trianglesDrawable without modifying the mesh.

typedef std::vector<vec3> Hole;

// convert the mesh into triangular mesh using the tessellator
void triangulate(SurfaceMesh* mesh) {
    if (!mesh)
        return;

    mesh->update_face_normals();
    auto normals = mesh->face_property<vec3>("f:normal");
    auto holes = mesh->get_face_property<Hole>("f:holes");

    struct Triangle : public std::vector<int> {
        Triangle() { resize(3); }
        Triangle(int a, int b, int c) {
            push_back(a);
            push_back(b);
            push_back(c);
        }
    };
    struct Point {
        Point(const vec3& p, bool status = false) : pt(p), is_new(status) {}
        vec3 pt;
        bool is_new;
    };

    std::vector<Point> points;
    std::vector<Triangle> triangles;

    gluTesselator *tessellator = tessNewTess(nullptr);

    int offset = 0;
    for (auto f : mesh->faces()) {
//        tessellator.begin_polygon(normals[f]);
//
//        tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
//        tessellator.begin_contour();
        std::vector<vec3> pts;
        for (auto h : mesh->halfedges(f)) {
            SurfaceMesh::Vertex v = mesh->to_vertex(h);
            pts.push_back(mesh->position(v));
        }

        tessSetOption(tessellator, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);  // use CDT
        tessAddContour(tessellator, 3, pts[0], sizeof(vec3), pts.size());
//        tessellator.end_contour();

        int rule = TESS_WINDING_NONZERO;
        if (holes && holes[f].size() > 3) { // has a valid hole
            const Hole& hole = holes[f];
            if (!hole.empty()) {
                tessAddContour(tessellator, 3, hole[0], sizeof(vec3), hole.size());
                rule = TESS_WINDING_ODD;
            }
        }

//        tessellator.end_polygon();

        if (tessTesselate(tessellator, rule, TESS_POLYGONS, 3, 3, normals[f])) {
            const float *verts = tessGetVertices(tessellator);
            const int *vinds = tessGetVertexIndices(tessellator);
            const int *elems = tessGetElements(tessellator);
            const int nverts = tessGetVertexCount(tessellator);
            const int nelems = tessGetElementCount(tessellator);

            for (int i = 0; i < nverts; ++i) {
                points.emplace_back(Point(vec3(verts + i * 3), vinds[i] == TESS_UNDEF));

//                if (vinds[i] == TESS_UNDEF) {
//                    // this is a new point due to self intersection
//                }
            }

            for (int i = 0; i < nelems; ++i) {
                const int *poly = &elems[i * 3];
                int a = offset + poly[0];
                int b = offset + poly[1];
                int c = offset + poly[2];
                triangles.emplace_back(Triangle(a, b, c));
            }

            offset = points.size();
        }
    }

    tessDeleteTess(tessellator);


    // now the tessellation is done. We can clear the old mesh and
    // fill it will the new set of triangles

    mesh->clear();

    if (!triangles.empty()) { // in degenerate cases num can be zero

        auto lock = mesh->vertex_property<bool>("v:lock");
        for (const auto &p : points) {
            auto v = mesh->add_vertex(p.pt);
            lock[v] = p.is_new;
//            if (p.is_new)
//                std::cout << "vertex " << v << " is new" << std::endl;
        }

        for (const auto &t : triangles) {
            mesh->add_triangle(SurfaceMesh::Vertex(t[0]), SurfaceMesh::Vertex(t[1]), SurfaceMesh::Vertex(t[2]));
            LOG(INFO) << "triangle: " << t[0] << " " << t[1] << " " << t[2] << std::endl;
        }
    }
}


int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Test_New_Tessellator");
        viewer.camera()->setUpVector(vec3(0, 1, 0));
        viewer.camera()->setViewDirection(vec3(0, 0, -1));

        //---------------------- create model -----------------------

#if 1
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
#else
        // Read a mesh specified by its file name
        const std::string file_name = resource::directory() + "/data/other/tree.obj";
        SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
        if (!mesh) {
            LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
            return EXIT_FAILURE;
        }
#endif

        //-------- Triangulate the mesh using the tessellator ---------

        StopWatch w;
        triangulate(mesh);
        std::cout << "tessellation took " << w.time_string() << std::endl;

        // ------------------------------------------------------------

        viewer.add_model(mesh, true);

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << "caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

