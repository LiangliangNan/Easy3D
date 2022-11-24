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

#include <easy3d/algo/surface_mesh_smoothing.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <easy3d/algo/surface_mesh_geometry.h>


namespace easy3d {

    // \cond
    using SparseMatrix = Eigen::SparseMatrix<double>;
    using Triplet = Eigen::Triplet<double>;
    // \endcond

    //-----------------------------------------------------------------------------

    SurfaceMeshSmoothing::SurfaceMeshSmoothing(SurfaceMesh *mesh) : mesh_(mesh) {
        how_many_edge_weights_ = 0;
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
                eweight[e] = static_cast<float>(std::max(0.0, geom::cotan_weight(mesh_, e)));
        }

        how_many_edge_weights_ = mesh_->n_edges();
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSmoothing::compute_vertex_weights(bool use_uniform_laplace) {
        auto vweight = mesh_->vertex_property<float>("v:area");

        if (use_uniform_laplace) {
            for (auto v : mesh_->vertices())
                vweight[v] = static_cast<float>(1.0 / mesh_->valence(v));
        } else {
            for (auto v : mesh_->vertices())
                vweight[v] = static_cast<float>(0.5 / geom::voronoi_area(mesh_, v));
        }
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
        vec3 center_before;
        float area_before;
        if (rescale) {
            center_before = geom::centroid(mesh_);
            area_before = geom::surface_area(mesh_);
        }

        // properties
        auto points = mesh_->get_vertex_property<vec3>("v:point");
        auto vweight = mesh_->get_vertex_property<float>("v:area");
        auto idx = mesh_->add_vertex_property<int>("v:idx", -1);

        // collect free (non-boundary) vertices in array free_vertices[]
        // assign indices such that idx[ free_vertices[i] ] == i
        int i = 0;
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
        for (i = 0; i < n; ++i) {
            SurfaceMesh::Vertex v = free_vertices[i];

            // rhs row
            dvec3 b = static_cast<dvec3>(points[v]) / vweight[v];

            // lhs row
            double ww = 0.0;
            for (auto h : mesh_->halfedges(v)) {
                SurfaceMesh::Vertex vv = mesh_->target(h);
                SurfaceMesh::Edge e = mesh_->edge(h);
                ww += eweight[e];

                // fixed boundary vertex -> right-hand side
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
            for (i = 0; i < n; ++i) {
                const auto &tmp = X.row(i);
                points[free_vertices[i]] = vec3(static_cast<float>(tmp(0)), static_cast<float>(tmp(1)), static_cast<float>(tmp(2)));
            }
        }

        if (rescale) {
            // restore original surface area
            float area_after = geom::surface_area(mesh_);
            float scale = std::sqrt(area_before / area_after);
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
