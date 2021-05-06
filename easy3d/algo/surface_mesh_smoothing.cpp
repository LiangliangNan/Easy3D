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


#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/algo/surface_mesh_geometry.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>


namespace easy3d {

    // \cond
    using SparseMatrix = Eigen::SparseMatrix<double>;
    using Triplet = Eigen::Triplet<double>;
    // \endcond

    //-----------------------------------------------------------------------------

    SurfaceMeshSmoothing::SurfaceMeshSmoothing(SurfaceMesh *mesh) : mesh_(mesh) {
        how_many_edge_weights_ = 0;
        how_many_vertex_weights_ = 0;
    }

    //-----------------------------------------------------------------------------

    SurfaceMeshSmoothing::~SurfaceMeshSmoothing() {
        auto vweight = mesh_->get_vertex_property<float>("v:area");
        if (vweight)
            mesh_->remove_vertex_property(vweight);

        auto eweight = mesh_->get_edge_property<float>("e:cotan");
        if (eweight)
            mesh_->remove_edge_property(eweight);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSmoothing::compute_edge_weights(bool use_uniform_laplace) {
        auto eweight = mesh_->edge_property<float>("e:cotan");

        if (use_uniform_laplace) {
            for (auto e : mesh_->edges())
                eweight[e] = 1.0;
        } else {
            for (auto e : mesh_->edges())
                eweight[e] = std::max(0.0, geom::cotan_weight(mesh_, e));
        }

        how_many_edge_weights_ = mesh_->n_edges();
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSmoothing::compute_vertex_weights(bool use_uniform_laplace) {
        auto vweight = mesh_->vertex_property<float>("v:area");

        if (use_uniform_laplace) {
            for (auto v : mesh_->vertices())
                vweight[v] = 1.0 / mesh_->valence(v);
        } else {
            for (auto v : mesh_->vertices())
                vweight[v] = 0.5 / geom::voronoi_area(mesh_, v);
        }

        how_many_vertex_weights_ = mesh_->n_vertices();
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSmoothing::explicit_smoothing(unsigned int iters,
                                                  bool use_uniform_laplace) {
        if (!mesh_->n_vertices())
            return;

        // compute Laplace weight per edge: cotan or uniform
        auto eweight = mesh_->get_edge_property<float>("e:cotan");
        if (!eweight || how_many_edge_weights_ != mesh_->n_edges())
            compute_edge_weights(use_uniform_laplace);
        eweight = mesh_->get_edge_property<float>("e:cotan");

        auto points = mesh_->get_vertex_property<vec3>("v:point");
        auto laplace = mesh_->add_vertex_property<vec3>("v:laplace");

        // smoothing iterations
        SurfaceMesh::Vertex vv;
        SurfaceMesh::Edge e;
        for (unsigned int i = 0; i < iters; ++i) {
            // step 1: compute Laplace for each vertex
            for (auto v : mesh_->vertices()) {
                vec3 l(0, 0, 0);

                if (!mesh_->is_border(v)) {
                    float w(0);

                    for (auto h : mesh_->halfedges(v)) {
                        vv = mesh_->target(h);
                        e = mesh_->edge(h);
                        l += eweight[e] * (points[vv] - points[v]);
                        w += eweight[e];
                    }

                    l /= w;
                }

                laplace[v] = l;
            }

            // step 2: move each vertex by its (damped) Laplacian
            for (auto v : mesh_->vertices()) {
                points[v] += 0.5f * laplace[v];
            }
        }

        // clean-up custom properties
        mesh_->remove_vertex_property(laplace);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSmoothing::implicit_smoothing(float timestep,
                                                  bool use_uniform_laplace,
                                                  bool rescale) {
        if (!mesh_->n_vertices())
            return;

        // compute edge weights if they don't exist or if the mesh changed
        auto eweight = mesh_->get_edge_property<float>("e:cotan");
        if (!eweight || how_many_edge_weights_ != mesh_->n_edges())
            compute_edge_weights(use_uniform_laplace);
        eweight = mesh_->get_edge_property<float>("e:cotan");

        // compute vertex weights
        compute_vertex_weights(use_uniform_laplace);

        // store center and area
        vec3 center_before = geom::centroid(mesh_);
        float area_before = geom::surface_area(mesh_);

        // properties
        auto points = mesh_->get_vertex_property<vec3>("v:point");
        auto vweight = mesh_->get_vertex_property<float>("v:area");
        auto idx = mesh_->add_vertex_property<int>("v:idx", -1);

        // collect free (non-boundary) vertices in array free_vertices[]
        // assign indices such that idx[ free_vertices[i] ] == i
        unsigned i = 0;
        std::vector<SurfaceMesh::Vertex> free_vertices;
        free_vertices.reserve(mesh_->n_vertices());
        for (auto v : mesh_->vertices()) {
            if (!mesh_->is_border(v)) {
                idx[v] = i++;
                free_vertices.push_back(v);
            }
        }
        const unsigned int n = free_vertices.size();

        // A*X = B
        SparseMatrix A(n, n);
        Eigen::MatrixXd B(n, 3);

        // nonzero elements of A as triplets: (row, column, value)
        std::vector<Triplet> triplets;

        // setup matrix A and rhs B
        dvec3 b;
        double ww;
        SurfaceMesh::Vertex v, vv;
        SurfaceMesh::Edge e;
        for (unsigned int i = 0; i < n; ++i) {
            v = free_vertices[i];

            // rhs row
            b = static_cast<dvec3>(points[v]) / vweight[v];

            // lhs row
            ww = 0.0;
            for (auto h : mesh_->halfedges(v)) {
                vv = mesh_->target(h);
                e = mesh_->edge(h);
                ww += eweight[e];

                // fixed boundary vertex -> right hand side
                if (mesh_->is_border(vv)) {
                    b -= -timestep * eweight[e] * static_cast<dvec3>(points[vv]);
                }
                    // free interior vertex -> matrix
                else {
                    triplets.emplace_back(i, idx[vv], -timestep * eweight[e]);
                }

                B.row(i) = (Eigen::Vector3d) b;
            }

            // center vertex -> matrix
            triplets.emplace_back(i, i, 1.0 / vweight[v] + timestep * ww);
        }

        // build sparse matrix from triplets
        A.setFromTriplets(triplets.begin(), triplets.end());

        // solve A*X = B
        Eigen::SimplicialLDLT<SparseMatrix> solver(A);
        Eigen::MatrixXd X = solver.solve(B);
        if (solver.info() != Eigen::Success) {
            std::cerr << "SurfaceMeshSmoothing: Could not solve linear system\n";
        } else {
            // copy solution
            for (unsigned int i = 0; i < n; ++i) {
                const auto &tmp = X.row(i);
                points[free_vertices[i]] = vec3(tmp(0), tmp(1), tmp(2));
            }
        }

        if (rescale) {
            // restore original surface area
            float area_after = geom::surface_area(mesh_);
            float scale = sqrt(area_before / area_after);
            for (auto v : mesh_->vertices())
                mesh_->position(v) *= scale;

            // restore original center
            vec3 center_after = geom::centroid(mesh_);
            vec3 trans = center_before - center_after;
            for (auto v : mesh_->vertices())
                mesh_->position(v) += trans;
        }

        // clean-up
        mesh_->remove_vertex_property(idx);
    }

} // namespace easy3d
