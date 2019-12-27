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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/setting.h>


using namespace easy3d;


// This example shows how to render imposters, e.g.,
//		- points as spheres;
//		- lines as cylinders;
//      - lines as cones.


int main(int /*argc*/, char** /*argv*/) {

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_404_Imposters");

        // Load point cloud data from a file
        const std::string file_name = setting::resource_directory() + "/data/sphere.obj";
        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer.open(file_name, false));
        if (!mesh) {
            std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
            return EXIT_FAILURE;
        }

        auto points = mesh->get_vertex_property<vec3>("v:point");

        //--------------------- render vertices as spheres -----------------------

        PointsDrawable* points_drawable = mesh->add_points_drawable("vertices");
        points_drawable->update_vertex_buffer(points.vector());
        points_drawable->set_per_vertex_color(false);
        points_drawable->set_default_color(vec3(1.0f, 0.0f, 0.0f));
        points_drawable->set_point_size(26.0f);
        points_drawable->set_impostors(true);

        //--------------------- render edges as cylinders -----------------------

        LinesDrawable* edges_drawable = mesh->add_lines_drawable("edges");
        std::vector<vec3> edge_points; // each consecutive point pair represents an edge
        for (auto e : mesh->edges()) {
            SurfaceMesh::Vertex s = mesh->vertex(e, 0);
            edge_points.push_back(points[s]);
            SurfaceMesh::Vertex t = mesh->vertex(e, 1);
            edge_points.push_back(points[t]);
        }
        edges_drawable->update_vertex_buffer(edge_points);
        edges_drawable->set_per_vertex_color(false);
        edges_drawable->set_default_color(vec3(1.0f, 0.67f, 0.5f));
        edges_drawable->set_impostor_type(IT_CYLINDERS);
        edges_drawable->set_line_width(8);

        //--------------------- render normals as cones -----------------------

        // Get the bounding box of the model. Then we defined the length of the
        // normal vectors to be 10% of the bounding box diagonal.
        const Box3& box = mesh->bounding_box();
        float length = norm(box.max() - box.min()) * 0.1f;

        // Now let collects the two end points of each normal vector. So from
        // these points we can create a drawable to visualize the normal vectors.

        // Every consecutive two points represent a normal vector.
        std::vector<vec3> normal_points;
        for (auto v : mesh->vertices()) {
            const vec3& s = points[v];
            const vec3& n = mesh->compute_vertex_normal(v);
            const vec3& t = points[v] + n * length;
            normal_points.push_back(s);
            normal_points.push_back(t);
        }

        LinesDrawable* normals_drawable = mesh->add_lines_drawable("normals");
        normals_drawable->update_vertex_buffer(normal_points);
        normals_drawable->set_per_vertex_color(false);
        normals_drawable->set_default_color(vec3(0.0f, 1.0f, 0.0f));
        normals_drawable->set_impostor_type(IT_CONES);
        normals_drawable->set_line_width(15);

        // -----------------------------------------------------------------

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        const std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

