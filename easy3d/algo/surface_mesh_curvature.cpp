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


#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/core/eigen_solver.h>

namespace easy3d {

    SurfaceMeshCurvature::SurfaceMeshCurvature(SurfaceMesh *mesh) : mesh_(mesh) {
        min_curvature_ = mesh_->vertex_property<float>("v:curv-min");
        max_curvature_ = mesh_->vertex_property<float>("v:curv-max");
    }

    //-----------------------------------------------------------------------------

    SurfaceMeshCurvature::~SurfaceMeshCurvature() {
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshCurvature::analyze(unsigned int post_smoothing_steps) {
        float kmin, kmax, mean, gauss;
        float area, sum_angles;
        float weight, sum_weights;
        vec3 p0, p1, p2, laplace;

        // cotan weight per edge
        auto cotan = mesh_->add_edge_property<double>("curv:cotan");
        for (auto e : mesh_->edges())
            cotan[e] = geom::cotan_weight(mesh_, e);

        // Voronoi area per vertex
        // Laplace per vertex
        // angle sum per vertex
        // -> mean, Gauss -> min, max curvature
        for (auto v : mesh_->vertices()) {
            kmin = kmax = 0.0;

            if (!mesh_->is_isolated(v) && !mesh_->is_boundary(v)) {
                laplace = vec3(0.0);
                sum_weights = 0.0;
                sum_angles = 0.0;
                p0 = mesh_->position(v);

                // Voronoi area
                area = geom::voronoi_area(mesh_, v);

                // Laplace & angle sum
                for (auto vh : mesh_->halfedges(v)) {
                    p1 = mesh_->position(mesh_->to_vertex(vh));
                    p2 = mesh_->position(
                            mesh_->to_vertex(mesh_->ccw_rotated_halfedge(vh)));

                    weight = cotan[mesh_->edge(vh)];
                    sum_weights += weight;
                    laplace += weight * p1;

                    p1 -= p0;
                    p1.normalize();
                    p2 -= p0;
                    p2.normalize();
                    sum_angles += acos(geom::clamp_cos(dot(p1, p2)));
                }
                laplace -= sum_weights * mesh_->position(v);
                laplace /= float(2.0) * area;

                mean = float(0.5) * norm(laplace);
                gauss = (2.0 * M_PI - sum_angles) / area;

                const float s = sqrt(std::max(float(0.0), mean * mean - gauss));
                kmin = mean - s;
                kmax = mean + s;
            }

            min_curvature_[v] = kmin;
            max_curvature_[v] = kmax;
        }

        // boundary vertices: interpolate from interior neighbors
        for (auto v : mesh_->vertices()) {
            if (mesh_->is_boundary(v)) {
                kmin = kmax = sum_weights = 0.0;

                for (auto vh : mesh_->halfedges(v)) {
                    v = mesh_->to_vertex(vh);
                    if (!mesh_->is_boundary(v)) {
                        weight = cotan[mesh_->edge(vh)];
                        sum_weights += weight;
                        kmin += weight * min_curvature_[v];
                        kmax += weight * max_curvature_[v];
                    }
                }

                if (sum_weights) {
                    kmin /= sum_weights;
                    kmax /= sum_weights;
                }

                min_curvature_[v] = kmin;
                max_curvature_[v] = kmax;
            }
        }

        // clean-up properties
        mesh_->remove_edge_property(cotan);

        // smooth curvature values
        smooth_curvatures(post_smoothing_steps);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshCurvature::analyze_tensor(unsigned int post_smoothing_steps,
                                              bool two_ring_neighborhood) {
        auto area = mesh_->add_vertex_property<double>("curv:area", 0.0);
        auto normal = mesh_->add_face_property<dvec3>("curv:normal");
        auto evec = mesh_->add_edge_property<dvec3>("curv:evec", dvec3(0, 0, 0));
        auto angle = mesh_->add_edge_property<double>("curv:angle", 0.0);

        dvec3 p0, p1, n0, n1, ev;
        double l, A, beta, a1, a2, a3;
        dmat3 tensor;

        double eval1, eval2, eval3, kmin, kmax;
        dvec3 evec1, evec2, evec3;

        std::vector<SurfaceMesh::Vertex> neighborhood;
        neighborhood.reserve(15);

        // precompute Voronoi area per vertex
        for (auto v : mesh_->vertices()) {
            area[v] = geom::voronoi_area(mesh_, v);
        }

        // precompute face normals
        for (auto f : mesh_->faces()) {
            normal[f] = (dvec3) mesh_->compute_face_normal(f);
        }

        // precompute dihedralAngle*edge_length*edge per edge
        for (auto e : mesh_->edges()) {
            auto h0 = mesh_->halfedge(e, 0);
            auto h1 = mesh_->halfedge(e, 1);
            auto f0 = mesh_->face(h0);
            auto f1 = mesh_->face(h1);
            if (f0.is_valid() && f1.is_valid()) {
                n0 = normal[f0];
                n1 = normal[f1];
                ev = (dvec3) mesh_->position(mesh_->to_vertex(h0));
                ev -= (dvec3) mesh_->position(mesh_->to_vertex(h1));
                l = norm(ev);
                if (l != 0) {   // avoid overflow in case of 0-length edges
                    ev /= l;
                    l *= 0.5; // only consider half of the edge (matchig Voronoi area)
                    angle[e] = atan2(dot(cross(n0, n1), ev), dot(n0, n1));
                    evec[e] = sqrt(l) * ev;
                }
            }
        }

        // compute curvature tensor for each vertex
        for (auto v : mesh_->vertices()) {
            kmin = 0.0;
            kmax = 0.0;

            if (!mesh_->is_isolated(v)) {
                // one-ring or two-ring neighborhood?
                neighborhood.clear();
                neighborhood.push_back(v);
                if (two_ring_neighborhood) {
                    for (auto vv : mesh_->vertices(v))
                        neighborhood.push_back(vv);
                }

                A = 0.0;
                tensor = dmat3(0.0);

                // compute tensor over vertex neighborhood stored in vertices
                for (auto nit : neighborhood) {
                    // accumulate tensor from dihedral angles around vertices
                    for (auto hv : mesh_->halfedges(nit)) {
                        auto ee = mesh_->edge(hv);
                        ev = evec[ee];
                        beta = angle[ee];
                        for (int i = 0; i < 3; ++i)
                            for (int j = 0; j < 3; ++j)
                                tensor(i, j) += beta * ev[i] * ev[j];
                    }

                    // accumulate area
                    A += area[nit];
                }

                // normalize tensor by accumulated
                if (A != 0)     // avoid overflow in case of 0-area
                    tensor /= A;

                // Liangliang: eigen solver requires FT** as input matrix :-(
                double **matrix = new double *[3];
                for (int i = 0; i < 3; ++i) {
                    matrix[i] = new double[3];
                    for (int j = 0; j < 3; ++j)
                        matrix[i][j] = tensor(i, j);
                }
                // Eigen-decomposition
                EigenSolver<double> solver(3);
                solver.solve(matrix, EigenSolver<double>::DECREASING);
                eval1 = solver.eigen_value(0);
                eval2 = solver.eigen_value(1);
                eval3 = solver.eigen_value(2);
                for (int i = 0; i < 3; ++i) {
                    evec1[i] = solver.eigen_vector(i, 0);
                    evec2[i] = solver.eigen_vector(i, 1);
                    evec3[i] = solver.eigen_vector(i, 2);
                }
                for (int i = 0; i < 3; ++i)
                    delete[] matrix[i];
                delete[] matrix;

                // curvature values:
                //   normal vector -> eval with smallest absolute value
                //   evals are sorted in decreasing order
                a1 = fabs(eval1);
                a2 = fabs(eval2);
                a3 = fabs(eval3);
                if (a1 < a2) {
                    if (a1 < a3) {
                        // e1 is normal
                        kmax = eval2;
                        kmin = eval3;
                    } else {
                        // e3 is normal
                        kmax = eval1;
                        kmin = eval2;
                    }
                } else {
                    if (a2 < a3) {
                        // e2 is normal
                        kmax = eval1;
                        kmin = eval3;
                    } else {
                        // e3 is normal
                        kmax = eval1;
                        kmin = eval2;
                    }
                }
            }

            assert(kmin <= kmax);

            min_curvature_[v] = kmin;
            max_curvature_[v] = kmax;
        }

        // clean-up properties
        mesh_->remove_vertex_property(area);
        mesh_->remove_edge_property(evec);
        mesh_->remove_edge_property(angle);
        mesh_->remove_face_property(normal);

        // smooth curvature values
        smooth_curvatures(post_smoothing_steps);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshCurvature::smooth_curvatures(unsigned int iterations) {
        float kmin, kmax;
        float weight, sum_weights;

        // properties
        auto vfeature = mesh_->get_vertex_property<bool>("v:feature");
        auto cotan = mesh_->add_edge_property<double>("curv:cotan");

        // cotan weight per edge
        for (auto e : mesh_->edges()) {
            cotan[e] = geom::cotan_weight(mesh_, e);
        }

        for (unsigned int i = 0; i < iterations; ++i) {
            for (auto v : mesh_->vertices()) {
                // don't smooth feature vertices
                if (vfeature && vfeature[v])
                    continue;

                kmin = kmax = sum_weights = 0.0;

                for (auto vh : mesh_->halfedges(v)) {
                    auto tv = mesh_->to_vertex(vh);

                    // don't consider feature vertices (high curvature)
                    if (vfeature && vfeature[tv])
                        continue;

                    weight = std::max(0.0, geom::cotan_weight(mesh_, mesh_->edge(vh)));
                    sum_weights += weight;
                    kmin += weight * min_curvature_[tv];
                    kmax += weight * max_curvature_[tv];
                }

                if (sum_weights) {
                    min_curvature_[v] = kmin / sum_weights;
                    max_curvature_[v] = kmax / sum_weights;
                }
            }
        }

        // remove property
        mesh_->remove_edge_property(cotan);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshCurvature::compute_mean_curvature() {
        auto curvatures = mesh_->vertex_property<float>("v:curv-mean");
        for (auto v : mesh_->vertices()) {
//            curvatures[v] = fabs(mean_curvature(v));
            curvatures[v] = mean_curvature(v);
        }
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshCurvature::compute_gauss_curvature() {
        auto curvatures = mesh_->vertex_property<float>("v:curv-gauss");
        for (auto v : mesh_->vertices())
            curvatures[v] = gauss_curvature(v);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshCurvature::compute_max_abs_curvature() {
        auto curvatures = mesh_->vertex_property<float>("v:curv-max_abs");
        for (auto v : mesh_->vertices())
            curvatures[v] = max_abs_curvature(v);
    }

} // namespace easy3d