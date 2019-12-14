/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/tessellator_gen.h>

using namespace easy3d;


// This example shows how to tessellate and rendering general
// polygonal meshes.
// From this example, you can easily (with minor modifications)
// render mesh models with concave polygons, self−intersecting
// polygons, and polygons with .

void create_drawable(SurfaceMesh* mesh, TessellatorGen::WindingRule rule) {
    if (!mesh)
        return;

    std::vector<vec3> points;
    std::vector<vec3> normals;
    std::vector<vec3> colors;    // when color attributes exist
    std::vector<unsigned int> indices;

    bool smooth_shading = false;
    auto v_normals = mesh->get_vertex_property<vec3>("v:normal");
    if (smooth_shading && !v_normals) {
        v_normals = mesh->add_vertex_property<vec3>("v:normal");
        for (auto v : mesh->vertices())
            v_normals[v] = mesh->compute_vertex_normal(v);
    }
    auto f_normals = mesh->get_face_property<vec3>("f:normal");
    if (!smooth_shading && !f_normals) {
        f_normals = mesh->add_face_property<vec3>("f:normal");
        for (auto f : mesh->faces())
            f_normals[f] = mesh->compute_face_normal(f);
    }

    auto v_colors = mesh->get_vertex_property<vec3>("v:color");
    auto f_colors = mesh->get_face_property<vec3>("f:color");
    const vec3 c(1.0f, 0.67f, 0.5f);

    //std::size_t face_idx = 0;
    //std::size_t tri_idx = 0;
    std::size_t index_offset = 0;
    //std::vector< std::vector<unsigned int> >	triangle_indices(mesh->n_faces());

    TessellatorGen gen;
    gen.set_winding_rule(rule);
    for (auto f : mesh->faces()) {
        gen.reset();
        gen.begin_polygon();
        gen.begin_contour();

        for (auto h : mesh->halfedges(f)) {
            SurfaceMesh::Vertex v = mesh->to_vertex(h);
            std::vector<vec3> data(3); // 3 (point) + 3 (normal) + 3 (color)
            data[0] = mesh->position(v);

            if (smooth_shading)
                data[1] = v_normals[v];
            else
                data[1] = f_normals[f];

            if (v_colors)
                data[2] = v_colors[v];
            else if (f_colors)
                data[2] = f_colors[f];
            else
                data[2] = c;
            gen.add_vertex_data(data[0], 9); // 3 (point) + 3 (normal) + 3 (color)
        }
        gen.end_contour();
        gen.end_polygon();

        std::size_t num = gen.num_triangles();
        if (num > 0) { // in degenerate cases num can be zero
            const std::vector<const double*>& vts = gen.get_vertices();
            for (std::size_t i = 0; i < vts.size(); ++i) {
                points.push_back(vec3(vts[i]));
                normals.push_back(vec3(vts[i] + 3));
                colors.push_back(vec3(vts[i] + 6));
            }

            for (std::size_t i = 0; i < num; ++i) {
                std::size_t a, b, c;
                gen.get_triangle(i, a, b, c);
                indices.push_back(a + index_offset);
                indices.push_back(b + index_offset);
                indices.push_back(c + index_offset);
                //triangle_indices[face_idx].push_back(tri_idx);
                //++tri_idx;
            }

            index_offset += vts.size();
        }
        //++face_idx;
    }

    TrianglesDrawable* drawable = mesh->add_triangles_drawable("surface");
    drawable->update_vertex_buffer(points);
    drawable->update_normal_buffer(normals);
    drawable->update_color_buffer(colors);
    drawable->update_index_buffer(indices);
    drawable->set_default_color(c);
    //drawable->set_triangle_indices(triangle_indices);
}


int main(int /*argc*/, char** /*argv*/) {
    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_405_Texture");
        viewer.camera()->setUpVector(vec3(0, 1, 0));
        viewer.camera()->setViewDirection(vec3(0, 0, -1));

        //---------------------- create models -----------------------

        // Mesh 1: a single concave quad face
        {
            SurfaceMesh* mesh = new SurfaceMesh;
            SurfaceMesh::Vertex v0 = mesh->add_vertex(vec3( 200,    0, 0));
            SurfaceMesh::Vertex v1 = mesh->add_vertex(vec3(1000,    0, 0));
            SurfaceMesh::Vertex v2 = mesh->add_vertex(vec3(1000,  800, 0));
            SurfaceMesh::Vertex v3 = mesh->add_vertex(vec3( 800,  300, 0));
            mesh->add_quad(v0, v1, v2, v3);
            viewer.add_model(mesh, false);  // I will create my own drawables
            create_drawable(mesh, TessellatorGen::NONZERO); // or POSITIVE
        }

        // Mesh 2: a self-intersecting face (a star)
        {
            SurfaceMesh* mesh = new SurfaceMesh;
            std::vector<SurfaceMesh::Vertex> vertices = {
                mesh->add_vertex(vec3( 400, 200, 0)),
                mesh->add_vertex(vec3( 200, 900, 0)),
                mesh->add_vertex(vec3(   0, 200, 0)),
                mesh->add_vertex(vec3( 600, 600, 0)),
                mesh->add_vertex(vec3(-200, 600, 0))
            };
            mesh->add_face(vertices);
            viewer.add_model(mesh, false);  // I will create my own drawables
            create_drawable(mesh, TessellatorGen::NONZERO); // or POSITIVE
        }

        //--- Create triangles drawable using the tessellator ---



        // -----------------------------------------------------------------

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

