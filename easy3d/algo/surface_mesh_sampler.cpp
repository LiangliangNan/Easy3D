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

#include <easy3d/algo/surface_mesh_sampler.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/progress.h>
#include <easy3d/algo/surface_mesh_triangulation.h>


namespace easy3d {


    PointCloud *SurfaceMeshSampler::apply(const SurfaceMesh *input_mesh, int expected_num /* = 1000000 */) {
        auto func = [](const SurfaceMesh *mesh, int num) -> PointCloud * {
            auto cloud = new PointCloud;
            const std::string &name = file_system::name_less_extension(mesh->name()) + "_sampled.ply";
            cloud->set_name(name);

            auto normals = cloud->add_vertex_property<vec3>("v:normal");

            LOG(INFO) << "sampling surface...";

            auto mesh_points = mesh->get_vertex_property<vec3>("v:point");

            // we need the vertex normals and face normals.
            const_cast<SurfaceMesh *>(mesh)->update_vertex_normals();
            const_cast<SurfaceMesh *>(mesh)->update_face_normals();

            // add all mesh vertices (even the requested number is smaller than the
            // number of vertices in the mesh).
            auto mesh_vertex_normals = mesh->get_vertex_property<vec3>("v:normal");
            for (auto p : mesh->vertices()) {
                PointCloud::Vertex v = cloud->add_vertex(mesh_points[p]);
                if (mesh_vertex_normals)
                    normals[v] = mesh_vertex_normals[p];
                else
                    normals[v] = mesh->compute_vertex_normal(p);
            }

            // now we may still need some points
            int num_needed = num - static_cast<int>(cloud->n_vertices());
            if (num_needed <= 0)
                return cloud;   // we got enough points already

            // collect triangles and compute their areas
            struct Triangle : std::vector<SurfaceMesh::Vertex> {};

            std::vector<Triangle> triangles;
            std::vector<float> triangle_areas;
            std::vector<vec3> triangle_normals;

            auto mesh_face_normals = mesh->get_face_property<vec3>("f:normal");
            float surface_area = 0.0f;
            for (auto f : mesh->faces()) {
                const vec3 &n = mesh_face_normals[f];
                SurfaceMesh::Halfedge start = mesh->halfedge(f);
                SurfaceMesh::Halfedge cur = mesh->next(mesh->next(start));
                SurfaceMesh::Vertex va = mesh->target(start);
                while (cur != start) {
                    SurfaceMesh::Vertex vb = mesh->source(cur);
                    SurfaceMesh::Vertex vc = mesh->target(cur);

                    Triangle tri;
                    tri.push_back(va);
                    tri.push_back(vb);
                    tri.push_back(vc);
                    triangles.push_back(tri);
                    float area = geom::triangle_area(mesh_points[va], mesh_points[vb], mesh_points[vc]);
                    triangle_areas.push_back(area);
                    triangle_normals.push_back(n);
                    surface_area += area;

                    cur = mesh->next(cur);
                }
            }

            float density = static_cast<float>(num_needed) / surface_area;
            float samples_error = 0.0f;
            std::size_t triangle_num = triangles.size();
            std::size_t num_generated = 0;
            std::size_t triangles_done = 0;
            ProgressLogger progress(triangle_num, false, false);
            for (std::size_t idx = 0; idx < triangle_num; ++idx) {
                if (progress.is_canceled()) {
                    LOG(WARNING) << "sampling surface mesh cancelled";
                    delete cloud;
                    return nullptr;
                }

                const Triangle &tri = triangles[idx];
                const vec3 &n = triangle_normals[idx];

                // samples number considering the facet size (i.e., area)
                float samples_num = triangle_areas[idx] * density;
                int quant_samples_num = static_cast<int>(samples_num);

                // adjust w.r.t. accumulated error
                samples_error += samples_num - static_cast<float>(quant_samples_num);
                if (samples_error > 1.0) {
                    samples_error -= 1.0;
                    quant_samples_num++;
                }

                if (triangles_done ==
                    triangle_num - 1)        // override number to gather all remaining points if last facet
                    quant_samples_num = num_needed - static_cast<int>(num_generated);

                // generate points
                for (int j = 0; j < quant_samples_num; j++) {
                    // compute barycentric coords
                    double s = std::sqrt(static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
                    double t = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
                    double c[3];

                    c[0] = 1.0 - s;
                    c[1] = s * (1.0 - t);
                    c[2] = s * t;

                    vec3 p;
                    for (std::size_t i = 0; i < 3; i++)
                        p = p + c[i] * mesh_points[tri[i]];

                    PointCloud::Vertex v = cloud->add_vertex(p);
                    normals[v] = n;
                }

                num_generated += quant_samples_num;
                triangles_done++;
                progress.next();
            }

            LOG(INFO) << "done. resulted point cloud has " << cloud->n_vertices() << " points";
            return cloud;
        };

        if (input_mesh->is_triangle_mesh())
            return func(input_mesh, expected_num);
        else {
            LOG(WARNING)
                    << "this is not a triangle mesh (creating a temporary triangle mesh by triangulating the input...)";
            SurfaceMesh mesh = *input_mesh;
            SurfaceMeshTriangulation triangulator(&mesh);
            triangulator.triangulate(SurfaceMeshTriangulation::MIN_AREA);
            return func(&mesh, expected_num);
        }
    }

}
