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


#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/util/logging.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>


namespace easy3d {

    // \cond
    using SparseMatrix = Eigen::SparseMatrix<double>;
    using Triplet = Eigen::Triplet<double>;
    // \endcond

    //=============================================================================

    SurfaceMeshFairing::SurfaceMeshFairing(SurfaceMesh *mesh) : mesh_(mesh) {
        // get & add properties
        points_ = mesh_->get_vertex_property<vec3>("v:point");
        vselected_ = mesh_->get_vertex_property<bool>("v:selected");
        vlocked_ = mesh_->add_vertex_property<bool>("fairing:locked");
        vweight_ = mesh_->add_vertex_property<double>("fairing:vweight");
        eweight_ = mesh_->add_edge_property<double>("fairing:eweight");
        idx_ = mesh_->add_vertex_property<int>("fairing:idx", -1);
    }

    //-----------------------------------------------------------------------------

    SurfaceMeshFairing::~SurfaceMeshFairing() {
        // remove properties
        mesh_->remove_vertex_property(vlocked_);
        mesh_->remove_vertex_property(vweight_);
        mesh_->remove_edge_property(eweight_);
        mesh_->remove_vertex_property(idx_);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshFairing::fair(unsigned int k) {
        // compute cotan weights
        for (auto v : mesh_->vertices()) {
            vweight_[v] = 0.5 / geom::voronoi_area(mesh_, v);
        }
        for (auto e : mesh_->edges()) {
            eweight_[e] = std::max(0.0, geom::cotan_weight(mesh_, e));
        }

        // check whether some vertices are selected
        bool no_selection = true;
        if (vselected_) {
            for (auto v : mesh_->vertices()) {
                if (vselected_[v]) {
                    no_selection = false;
                    break;
                }
            }
        }

        // lock k locked boundary rings
        for (auto v : mesh_->vertices()) {
            // lock boundary
            if (mesh_->is_border(v)) {
                vlocked_[v] = true;

                // lock one-ring of boundary
                if (k > 1) {
                    for (auto vv : mesh_->vertices(v)) {
                        vlocked_[vv] = true;

                        // lock two-ring of boundary
                        if (k > 2) {
                            for (auto vvv : mesh_->vertices(vv)) {
                                vlocked_[vvv] = true;
                            }
                        }
                    }
                }
            }
        }

        // lock un-selected and isolated vertices
        for (auto v : mesh_->vertices()) {
            if (!no_selection && !vselected_[v]) {
                vlocked_[v] = true;
            }

            if (mesh_->is_isolated(v)) {
                vlocked_[v] = true;
            }
        }

        // collect free vertices
        std::vector<SurfaceMesh::Vertex> vertices;
        vertices.reserve(mesh_->n_vertices());
        for (auto v : mesh_->vertices()) {
            if (!vlocked_[v]) {
                idx_[v] = vertices.size();
                vertices.push_back(v);
            }
        }

        // we need locked vertices as boundary constraints
        if (vertices.size() == mesh_->n_vertices()) {
            LOG(WARNING) << "SurfaceMeshFairing requires locked vertices as boundary constraints.";
            return;
        }

        // construct matrix & rhs
        const unsigned int n = vertices.size();
        SparseMatrix A(n, n);
        Eigen::MatrixXd B(n, 3);
        dvec3 b;

        std::map<SurfaceMesh::Vertex, double> row;
        std::vector<Triplet> triplets;

        for (unsigned int i = 0; i < n; ++i) {
            b = dvec3(0.0);

            setup_matrix_row(vertices[i], vweight_, eweight_, k, row);

            for (auto r : row) {
                auto v = r.first;
                auto w = r.second;

                if (idx_[v] != -1) {
                    triplets.emplace_back(i, idx_[v], w);
                } else {
                    b -= w * static_cast<dvec3>(points_[v]);
                }
            }

            B.row(i) = (Eigen::Vector3d) b;
        }

        A.setFromTriplets(triplets.begin(), triplets.end());

        // solve A*X = B
        Eigen::SimplicialLDLT<SparseMatrix> solver(A);
        Eigen::MatrixXd X = solver.solve(B);

        if (solver.info() != Eigen::Success) {
            std::cerr << "SurfaceMeshFairing: Could not solve linear system\n";
        } else {
            for (unsigned int i = 0; i < n; ++i) {
                const auto &tmp = X.row(i);
                points_[vertices[i]] = vec3(tmp(0), tmp(1), tmp(2));
            }
        }
    }

    //-----------------------------------------------------------------------------

    struct Triple {
        Triple() = default;

        Triple(SurfaceMesh::Vertex v, double weight, unsigned int degree)
                : vertex_(v), weight_(weight), degree_(degree) {
        }

        SurfaceMesh::Vertex vertex_;
        double weight_;
        unsigned int degree_;
    };

    //-----------------------------------------------------------------------------

    void SurfaceMeshFairing::setup_matrix_row(const SurfaceMesh::Vertex v,
                                              SurfaceMesh::VertexProperty<double> vweight,
                                              SurfaceMesh::EdgeProperty<double> eweight,
                                              unsigned int laplace_degree,
                                              std::map<SurfaceMesh::Vertex, double> &row) {
        Triple t(v, 1.0, laplace_degree);

        // init
        static std::vector<Triple> todo;
        todo.reserve(50);
        todo.push_back(t);
        row.clear();

        while (!todo.empty()) {
            t = todo.back();
            todo.pop_back();
            auto v = t.vertex_;
            auto d = t.degree_;

            if (d == 0) {
                row[v] += t.weight_;
            }

                // else if (d == 1 && mesh_.is_border(v))
                // {
                //     // ignore?
                // }

            else {
                auto ww = 0.0;

                for (auto h : mesh_->halfedges(v)) {
                    auto e = mesh_->edge(h);
                    auto vv = mesh_->target(h);
                    auto w = eweight[e];

                    if (d < laplace_degree)
                        w *= vweight[v];

                    w *= t.weight_;
                    ww -= w;

                    todo.emplace_back(vv, w, d - 1);
                }

                todo.emplace_back(v, ww, d - 1);
            }
        }
    }

//=============================================================================
} // namespace easy3d
//=============================================================================
