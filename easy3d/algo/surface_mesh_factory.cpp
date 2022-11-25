/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
 ********************************************************************/


#include <easy3d/algo/surface_mesh_factory.h>

#include <cmath>
#include <algorithm>

#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/algo/surface_mesh_geometry.h>


namespace easy3d {

    namespace internal {
        void project_to_unit_sphere(SurfaceMesh &mesh) {
            for (auto v: mesh.vertices()) {
                auto p = mesh.position(v);
                auto n = norm(p);
                mesh.position(v) = (1.0 / n) * p;
            }
        }
    }

    SurfaceMesh SurfaceMeshFactory::tetrahedron() {
        SurfaceMesh mesh;
        float a = 1.0f / 3.0f;
        float b = std::sqrt(8.0f / 9.0f);
        float c = std::sqrt(2.0f / 9.0f);
        float d = std::sqrt(2.0f / 3.0f);

        auto v0 = mesh.add_vertex(vec3(0, 0, 1));
        auto v1 = mesh.add_vertex(vec3(-c, d, -a));
        auto v2 = mesh.add_vertex(vec3(-c, -d, -a));
        auto v3 = mesh.add_vertex(vec3(b, 0, -a));

        mesh.add_triangle(v0, v1, v2);
        mesh.add_triangle(v0, v2, v3);
        mesh.add_triangle(v0, v3, v1);
        mesh.add_triangle(v3, v2, v1);

        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::hexahedron() {
        SurfaceMesh mesh;

        float a = 1.0f / std::sqrt(3.0f);
        auto v0 = mesh.add_vertex(vec3(-a, -a, -a));
        auto v1 = mesh.add_vertex(vec3(a, -a, -a));
        auto v2 = mesh.add_vertex(vec3(a, a, -a));
        auto v3 = mesh.add_vertex(vec3(-a, a, -a));
        auto v4 = mesh.add_vertex(vec3(-a, -a, a));
        auto v5 = mesh.add_vertex(vec3(a, -a, a));
        auto v6 = mesh.add_vertex(vec3(a, a, a));
        auto v7 = mesh.add_vertex(vec3(-a, a, a));

        mesh.add_quad(v3, v2, v1, v0);
        mesh.add_quad(v2, v6, v5, v1);
        mesh.add_quad(v5, v6, v7, v4);
        mesh.add_quad(v0, v4, v7, v3);
        mesh.add_quad(v3, v7, v6, v2);
        mesh.add_quad(v1, v5, v4, v0);

        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::octahedron() {
        auto mesh = hexahedron();
        geom::dual(&mesh);
        internal::project_to_unit_sphere(mesh);
        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::dodecahedron() {
        auto mesh = icosahedron();
        geom::dual(&mesh);
        internal::project_to_unit_sphere(mesh);
        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::icosahedron() {
        SurfaceMesh mesh;

        float phi = (1.0f + std::sqrt(5.0f)) * 0.5f; // golden ratio
        float a = 1.0f;
        float b = 1.0f / phi;

        auto v1 = mesh.add_vertex(vec3(0, b, -a));
        auto v2 = mesh.add_vertex(vec3(b, a, 0));
        auto v3 = mesh.add_vertex(vec3(-b, a, 0));
        auto v4 = mesh.add_vertex(vec3(0, b, a));
        auto v5 = mesh.add_vertex(vec3(0, -b, a));
        auto v6 = mesh.add_vertex(vec3(-a, 0, b));
        auto v7 = mesh.add_vertex(vec3(0, -b, -a));
        auto v8 = mesh.add_vertex(vec3(a, 0, -b));
        auto v9 = mesh.add_vertex(vec3(a, 0, b));
        auto v10 = mesh.add_vertex(vec3(-a, 0, -b));
        auto v11 = mesh.add_vertex(vec3(b, -a, 0));
        auto v12 = mesh.add_vertex(vec3(-b, -a, 0));

        internal::project_to_unit_sphere(mesh);

        mesh.add_triangle(v3, v2, v1);
        mesh.add_triangle(v2, v3, v4);
        mesh.add_triangle(v6, v5, v4);
        mesh.add_triangle(v5, v9, v4);
        mesh.add_triangle(v8, v7, v1);
        mesh.add_triangle(v7, v10, v1);
        mesh.add_triangle(v12, v11, v5);
        mesh.add_triangle(v11, v12, v7);
        mesh.add_triangle(v10, v6, v3);
        mesh.add_triangle(v6, v10, v12);
        mesh.add_triangle(v9, v8, v2);
        mesh.add_triangle(v8, v9, v11);
        mesh.add_triangle(v3, v6, v4);
        mesh.add_triangle(v9, v2, v4);
        mesh.add_triangle(v10, v3, v1);
        mesh.add_triangle(v2, v8, v1);
        mesh.add_triangle(v12, v10, v7);
        mesh.add_triangle(v8, v11, v7);
        mesh.add_triangle(v6, v12, v5);
        mesh.add_triangle(v11, v9, v5);

        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::icosphere(size_t n_subdivisions) {
        auto mesh = icosahedron();
        for (size_t i = 0; i < n_subdivisions; i++) {
            SurfaceMeshSubdivision::loop(&mesh);
            internal::project_to_unit_sphere(mesh);
        }
        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::quad_sphere(size_t n_subdivisions) {
        auto mesh = hexahedron();
        for (size_t i = 0; i < n_subdivisions; i++) {
            SurfaceMeshSubdivision::catmull_clark(&mesh);
            internal::project_to_unit_sphere(mesh);
        }
        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::uv_sphere(const vec3 &center, float radius,
                                              size_t n_slices, size_t n_stacks) {
        SurfaceMesh mesh;

        // add top vertex
        auto v0 = mesh.add_vertex(vec3(center[0], center[1] + radius, center[2]));

        // generate vertices per stack / slice
        for (size_t i = 0; i < n_stacks - 1; i++) {
            auto phi = M_PI * double(i + 1) / double(n_stacks);
            for (size_t j = 0; j < n_slices; ++j) {
                auto theta = 2.0 * M_PI * double(j) / double(n_slices);
                auto x = center[0] + radius * std::sin(phi) * std::cos(theta);
                auto y = center[1] + radius * std::cos(phi);
                auto z = center[2] + radius * std::sin(phi) * std::sin(theta);
                mesh.add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
            }
        }

        // add bottom vertex
        auto v1 = mesh.add_vertex(vec3(center[0], center[1] - radius, center[2]));

        // add top / bottom triangles
        size_t i0, i1;
        for (size_t i = 0; i < n_slices; ++i) {
            i0 = i + 1;
            i1 = (i + 1) % n_slices + 1;
            mesh.add_triangle(v0, SurfaceMesh::Vertex(static_cast<int>(i1)), SurfaceMesh::Vertex(static_cast<int>(i0)));

            i0 = i + n_slices * (n_stacks - 2) + 1;
            i1 = (i + 1) % n_slices + n_slices * (n_stacks - 2) + 1;
            mesh.add_triangle(v1, SurfaceMesh::Vertex(static_cast<int>(i0)), SurfaceMesh::Vertex(static_cast<int>(i1)));
        }

        // add quads per stack / slice
        size_t i2, i3;
        for (size_t j = 0; j < n_stacks - 2; ++j) {
            size_t idx0 = j * n_slices + 1;
            size_t idx1 = (j + 1) * n_slices + 1;
            for (size_t i = 0; i < n_slices; ++i) {
                i0 = idx0 + i;
                i1 = idx0 + (i + 1) % n_slices;
                i2 = idx1 + (i + 1) % n_slices;
                i3 = idx1 + i;
                mesh.add_quad(
                    SurfaceMesh::Vertex(static_cast<int>(i0)), 
                    SurfaceMesh::Vertex(static_cast<int>(i1)), 
                    SurfaceMesh::Vertex(static_cast<int>(i2)),
                    SurfaceMesh::Vertex(static_cast<int>(i3))
                );
            }
        }

        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::plane(size_t resolution) {
        assert(resolution >= 1);

        SurfaceMesh mesh;

        // generate vertices
        vec3 p(0, 0, 0);
        for (size_t i = 0; i < resolution + 1; i++) {
            for (size_t j = 0; j < resolution + 1; j++) {
                mesh.add_vertex(p);
                p[1] += static_cast<float>(1.0 / resolution);
            }
            p[1] = 0;
            p[0] += static_cast<int>(1.0 / resolution);
        }

        // generate faces
        for (size_t i = 0; i < resolution; i++) {
            for (size_t j = 0; j < resolution; j++) {
                auto v0 = SurfaceMesh::Vertex(static_cast<int>(j + i * (resolution + 1)));
                auto v1 = SurfaceMesh::Vertex(static_cast<int>(v0.idx() + resolution + 1));
                auto v2 = SurfaceMesh::Vertex(static_cast<int>(v0.idx() + resolution + 2));
                auto v3 = SurfaceMesh::Vertex(static_cast<int>(v0.idx() + 1));
                mesh.add_quad(v0, v1, v2, v3);
            }
        }

        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::cone(size_t n_subdivisions, float radius, float height) {
        assert(n_subdivisions >= 3);

        SurfaceMesh mesh;

        // add vertices subdividing a circle
        std::vector<SurfaceMesh::Vertex> base_vertices;
        for (size_t i = 0; i < n_subdivisions; i++) {
            double ratio = static_cast<double>(i) / (n_subdivisions);
            double r = ratio * (M_PI * 2.0);
            double x = std::cos(r) * radius;
            double y = std::sin(r) * radius;
            auto v = mesh.add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), 0.0f));
            base_vertices.push_back(v);
        }

        // add the tip of the cone
        auto v0 = mesh.add_vertex(vec3(0.0, 0.0, height));

        // generate triangular faces
        for (size_t i = 0; i < n_subdivisions; i++) {
            auto ii = (i + 1) % n_subdivisions;
            mesh.add_triangle(v0, SurfaceMesh::Vertex(static_cast<int>(i)), SurfaceMesh::Vertex(static_cast<int>(ii)));
        }

        // reverse order for consistent face orientation
        std::reverse(base_vertices.begin(), base_vertices.end());

        // add polygonal base face
        mesh.add_face(base_vertices);

        return mesh;
    }

    SurfaceMesh SurfaceMeshFactory::cylinder(size_t n_subdivisions, float radius, float height) {
        assert(n_subdivisions >= 3);

        SurfaceMesh mesh;

        // generate vertices
        std::vector<SurfaceMesh::Vertex> bottom_vertices;
        std::vector<SurfaceMesh::Vertex> top_vertices;
        for (size_t i = 0; i < n_subdivisions; i++) {
            double ratio = static_cast<double>(i) / (n_subdivisions);
            double r = ratio * (M_PI * 2.0);
            double x = std::cos(r) * radius;
            double y = std::sin(r) * radius;
            auto v = mesh.add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), 0.0f));
            bottom_vertices.push_back(v);
            v = mesh.add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), height));
            top_vertices.push_back(v);
        }

        // add faces around the cylinder
        for (size_t i = 0; i < n_subdivisions; i++) {
            auto ii = i * 2;
            auto jj = (ii + 2) % (n_subdivisions * 2);
            auto kk = (ii + 3) % (n_subdivisions * 2);
            auto ll = ii + 1;
            mesh.add_quad(
                SurfaceMesh::Vertex(static_cast<int>(ii)), 
                SurfaceMesh::Vertex(static_cast<int>(jj)), 
                SurfaceMesh::Vertex(static_cast<int>(kk)),
                SurfaceMesh::Vertex(static_cast<int>(ll))
            );
        }

        // add top polygon
        mesh.add_face(top_vertices);

        // reverse order for consistent face orientation
        std::reverse(bottom_vertices.begin(), bottom_vertices.end());

        // add bottom polygon
        mesh.add_face(bottom_vertices);

        return mesh;
    }

    SurfaceMesh
    SurfaceMeshFactory::torus(size_t radial_resolution, size_t tubular_resolution, float radius, float thickness) {
        assert(radial_resolution >= 3);
        assert(tubular_resolution >= 3);

        SurfaceMesh mesh;

        // generate vertices
        for (size_t i = 0; i < radial_resolution; i++) {
            for (size_t j = 0; j < tubular_resolution; j++) {
                double u = static_cast<double>(j) / tubular_resolution * M_PI * 2.0;
                double v = static_cast<double>(i) / radial_resolution * M_PI * 2.0;
                double x = (radius + thickness * std::cos(v)) * std::cos(u);
                double y = (radius + thickness * std::cos(v)) * std::sin(u);
                double z = thickness * std::sin(v);
                mesh.add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
            }
        }

        // add quad faces
        for (size_t i = 0; i < radial_resolution; i++) {
            auto i_next = (i + 1) % radial_resolution;
            for (size_t j = 0; j < tubular_resolution; j++) {
                auto j_next = (j + 1) % tubular_resolution;
                auto i0 = i * tubular_resolution + j;
                auto i1 = i * tubular_resolution + j_next;
                auto i2 = i_next * tubular_resolution + j_next;
                auto i3 = i_next * tubular_resolution + j;
                mesh.add_quad(
                    SurfaceMesh::Vertex(static_cast<int>(i0)), 
                    SurfaceMesh::Vertex(static_cast<int>(i1)), 
                    SurfaceMesh::Vertex(static_cast<int>(i2)),
                    SurfaceMesh::Vertex(static_cast<int>(i3))
                );
            }
        }

        return mesh;
    }

} // namespace easy3d