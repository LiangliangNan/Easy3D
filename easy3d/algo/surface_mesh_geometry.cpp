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

#include <easy3d/algo/surface_mesh_geometry.h>

#include <limits>
#include <cmath>


namespace easy3d {

    namespace geom {

        float triangle_area(const SurfaceMesh *mesh, SurfaceMesh::Face f) {
            assert(mesh->valence(f) == 3);

            auto fv = mesh->vertices(f);
            const auto &p0 = mesh->position(*fv);
            const auto &p1 = mesh->position(*(++fv));
            const auto &p2 = mesh->position(*(++fv));

            return triangle_area(p0, p1, p2);
        }

        //-----------------------------------------------------------------------------

        float surface_area(const SurfaceMesh *mesh) {
            float area(0);
            for (auto f : mesh->faces()) {
                area += triangle_area(mesh, f);
            }
            return area;
        }

        //-----------------------------------------------------------------------------

        float volume(const SurfaceMesh& mesh)
        {
            if (!mesh.is_triangle_mesh()) {
                LOG(ERROR) << "input is not a pure triangle mesh!";
                return 0;
            }

            float volume(0);
            for (const auto f : mesh.faces()) {
                auto fv = mesh.vertices(f);
                const auto& p0 = mesh.position(*fv);
                const auto& p1 = mesh.position(*(++fv));
                const auto& p2 = mesh.position(*(++fv));

                volume += float(1.0) / float(6.0) * dot(cross(p0, p1), p2);
            }

            return std::abs(volume);
        }

        //-----------------------------------------------------------------------------

        vec3 centroid(const SurfaceMesh *mesh, SurfaceMesh::Face f) {
            vec3 c(0, 0, 0);
            float n(0);
            for (auto v : mesh->vertices(f)) {
                c += mesh->position(v);
                ++n;
            }
            c /= n;
            return c;
        }

        //-----------------------------------------------------------------------------

        vec3 centroid(const SurfaceMesh *mesh) {
            vec3 center(0, 0, 0), c;
            float area(0), a;
            for (auto f : mesh->faces()) {
                a = triangle_area(mesh, f);
                c = centroid(mesh, f);
                area += a;
                center += a * c;
            }
            center /= area;
            return center;
        }

        //-----------------------------------------------------------------------------

        void dual(SurfaceMesh* mesh)
        {
            // the new dualized mesh
            SurfaceMesh tmp;

            // remember new vertices per face
            auto fvertex = mesh->add_face_property<SurfaceMesh::Vertex>("f:vertex");

            // add centroid for each face
            for (auto f : mesh->faces())
                fvertex[f] = tmp.add_vertex(centroid(mesh, f));

            // add new face for each vertex
            for (auto v : mesh->vertices())
            {
                std::vector<SurfaceMesh::Vertex> vertices;
                for (auto f : mesh->faces(v))
                    vertices.push_back(fvertex[f]);

                tmp.add_face(vertices);
            }

            // swap old and new meshes, don't copy properties
            mesh->assign(tmp);
        }

        //-----------------------------------------------------------------------------

        double cotan_weight(const SurfaceMesh *mesh, SurfaceMesh::Edge e) {
            double weight = 0.0;

            const SurfaceMesh::Halfedge h0 = mesh->halfedge(e, 0);
            const SurfaceMesh::Halfedge h1 = mesh->halfedge(e, 1);

            const dvec3 p0 = (dvec3) mesh->position(mesh->target(h0));
            const dvec3 p1 = (dvec3) mesh->position(mesh->target(h1));

            if (!mesh->is_border(h0)) {
                const dvec3 p2 =
                        (dvec3) mesh->position(mesh->target(mesh->next(h0)));
                const dvec3 d0 = p0 - p2;
                const dvec3 d1 = p1 - p2;
                const double area = norm(cross(d0, d1));
                if (area > std::numeric_limits<double>::min()) {
                    const double cot = dot(d0, d1) / area;
                    weight += clamp_cot(cot);
                }
            }

            if (!mesh->is_border(h1)) {
                const dvec3 p2 =
                        (dvec3) mesh->position(mesh->target(mesh->next(h1)));
                const dvec3 d0 = p0 - p2;
                const dvec3 d1 = p1 - p2;
                const double area = norm(cross(d0, d1));
                if (area > std::numeric_limits<double>::min()) {
                    const double cot = dot(d0, d1) / area;
                    weight += clamp_cot(cot);
                }
            }

            assert(!std::isnan(weight));
            assert(!std::isinf(weight));

            return weight;
        }

        //-----------------------------------------------------------------------------

        double voronoi_area(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            double area(0.0);

            if (!mesh->is_isolated(v)) {
                SurfaceMesh::Halfedge h0, h1, h2;
                dvec3 p, q, r, pq, qr, pr;
                double dotp, dotq, dotr, triArea;
                double cotq, cotr;

                for (auto h : mesh->halfedges(v)) {
                    h0 = h;
                    h1 = mesh->next(h0);
                    h2 = mesh->next(h1);

                    if (mesh->is_border(h0))
                        continue;

                    // three vertex positions
                    p = (dvec3) mesh->position(mesh->target(h2));
                    q = (dvec3) mesh->position(mesh->target(h0));
                    r = (dvec3) mesh->position(mesh->target(h1));

                    // edge vectors
                    (pq = q) -= p;
                    (qr = r) -= q;
                    (pr = r) -= p;

                    // compute and check triangle area
                    triArea = norm(cross(pq, pr));
                    if (triArea <= std::numeric_limits<double>::min())
                        continue;

                    // dot products for each corner (of its two emanating edge vectors)
                    dotp = dot(pq, pr);
                    dotq = -dot(qr, pq);
                    dotr = dot(qr, pr);

                    // angle at p is obtuse
                    if (dotp < 0.0) {
                        area += 0.25 * triArea;
                    }

                        // angle at q or r obtuse
                    else if (dotq < 0.0 || dotr < 0.0) {
                        area += 0.125 * triArea;
                    }

                        // no obtuse angles
                    else {
                        // cot(angle) = cos(angle)/sin(angle) = dot(A,B)/norm(cross(A,B))
                        cotq = dotq / triArea;
                        cotr = dotr / triArea;

                        // clamp cot(angle) by clamping angle to [1,179]
                        area += 0.125 * (length2(pr) * clamp_cot(cotq) +
                                         length2(pq) * clamp_cot(cotr));
                    }
                }
            }

            assert(!std::isnan(area));
            assert(!std::isinf(area));

            return area;
        }

        //-----------------------------------------------------------------------------

        double voronoi_area_barycentric(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            double area(0.0);

            if (!mesh->is_isolated(v)) {
                const vec3 p = mesh->position(v);
                SurfaceMesh::Halfedge h0, h1;
                vec3 q, r, pq, pr;

                for (auto h : mesh->halfedges(v)) {
                    if (mesh->is_border(h))
                        continue;

                    h0 = h;
                    h1 = mesh->next(h0);

                    pq = mesh->position(mesh->target(h0));
                    pq -= p;

                    pr = mesh->position(mesh->target(h1));
                    pr -= p;

                    area += norm(cross(pq, pr)) / 3.0;
                }
            }

            return area;
        }

        //-----------------------------------------------------------------------------

        vec3 laplace(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            vec3 laplace(0.0f, 0.0f, 0.0f);

            if (!mesh->is_isolated(v)) {
                double weight, sumWeights(0.0f);

                for (auto h : mesh->halfedges(v)) {
                    weight = cotan_weight(mesh, mesh->edge(h));
                    sumWeights += weight;
                    laplace += weight * mesh->position(mesh->target(h));
                }

                laplace -= sumWeights * mesh->position(v);
                laplace /= float(2.0) * voronoi_area(mesh, v);
            }

            return laplace;
        }

        //-----------------------------------------------------------------------------

        double angle_sum(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            double angles(0.0);

            if (!mesh->is_border(v)) {
                const vec3 &p0 = mesh->position(v);

                for (auto h : mesh->halfedges(v)) {
                    const vec3 &p1 = mesh->position(mesh->target(h));
                    const vec3 &p2 =
                            mesh->position(mesh->target(mesh->prev_around_source(h)));

                    const vec3 p01 = normalize(p1 - p0);
                    const vec3 p02 = normalize(p2 - p0);

                    auto cos_angle = clamp_cos(dot(p01, p02));

                    angles += std::acos(cos_angle);
                }
            }

            return angles;
        }

        //-----------------------------------------------------------------------------

        VertexCurvature vertex_curvature(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            VertexCurvature c;

            const double area = voronoi_area(mesh, v);
            if (area > std::numeric_limits<double>::min()) {
                c.mean = 0.5f * norm(laplace(mesh, v));
                c.gauss = static_cast<float>((2.0f * M_PI - angle_sum(mesh, v)) / area);

                const float s = std::sqrt(std::max(0.0f, c.mean * c.mean - c.gauss));
                c.min = c.mean - s;
                c.max = c.mean + s;

                assert(!std::isnan(c.mean));
                assert(!std::isnan(c.gauss));
                assert(!std::isinf(c.mean));
                assert(!std::isinf(c.gauss));

                assert(c.min <= c.mean);
                assert(c.mean <= c.max);
            }

            return c;
        }

    }

} // namespace easy3d
