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

#include <easy3d/algo/surface_mesh_geometry.h>

#include <limits>
#include <cmath>
#include <cfloat>


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
            vec3 laplace(0.0, 0.0, 0.0);

            if (!mesh->is_isolated(v)) {
                float weight, sumWeights(0.0);

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

        float angle_sum(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            float angles(0.0);

            if (!mesh->is_border(v)) {
                const vec3 &p0 = mesh->position(v);

                for (auto h : mesh->halfedges(v)) {
                    const vec3 &p1 = mesh->position(mesh->target(h));
                    const vec3 &p2 =
                            mesh->position(mesh->target(mesh->prev_around_source(h)));

                    const vec3 p01 = normalize(p1 - p0);
                    const vec3 p02 = normalize(p2 - p0);

                    float cos_angle = clamp_cos(dot(p01, p02));

                    angles += acos(cos_angle);
                }
            }

            return angles;
        }

        //-----------------------------------------------------------------------------

        VertexCurvature vertex_curvature(const SurfaceMesh *mesh, SurfaceMesh::Vertex v) {
            VertexCurvature c;

            const float area = voronoi_area(mesh, v);
            if (area > std::numeric_limits<float>::min()) {
                c.mean = float(0.5) * norm(laplace(mesh, v));
                c.gauss = (2.0 * M_PI - angle_sum(mesh, v)) / area;

                const float s = sqrt(std::max(float(0.0), c.mean * c.mean - c.gauss));
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
