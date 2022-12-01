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

#include <easy3d/algo/surface_mesh_hole_filling.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/util/logging.h>

using SparseMatrix = Eigen::SparseMatrix<double>;
using Triplet = Eigen::Triplet<double>;


namespace easy3d {

    SurfaceMeshHoleFilling::SurfaceMeshHoleFilling(SurfaceMesh *mesh) : mesh_(mesh) {
        points_ = mesh_->get_vertex_property<vec3>("v:point");
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshHoleFilling::is_interior_edge(SurfaceMesh::Vertex _a, SurfaceMesh::Vertex _b) const {
        SurfaceMesh::Halfedge h = mesh_->find_halfedge(_a, _b);
        if (!h.is_valid())
            return false; // edge does not exist
        return (!mesh_->is_border(h) &&
                !mesh_->is_border(mesh_->opposite(h)));
    }

    //-----------------------------------------------------------------------------

    float
    SurfaceMeshHoleFilling::compute_area(SurfaceMesh::Vertex _a, SurfaceMesh::Vertex _b, SurfaceMesh::Vertex _c) const {
        return length2(cross(points_[_b] - points_[_a], points_[_c] - points_[_a]));
    }

    //-----------------------------------------------------------------------------

    vec3 SurfaceMeshHoleFilling::compute_normal(SurfaceMesh::Vertex _a, SurfaceMesh::Vertex _b,
                                                SurfaceMesh::Vertex _c) const {
        return normalize(
                cross(points_[_b] - points_[_a], points_[_c] - points_[_a]));
    }

    //-----------------------------------------------------------------------------

    float SurfaceMeshHoleFilling::compute_angle(const vec3 &_n1,
                                                const vec3 &_n2) const {
        return (1.0f - dot(_n1, _n2));
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshHoleFilling::fill_hole(SurfaceMesh::Halfedge _h) {
        // is it really a hole?
        if (!mesh_->is_border(_h)) {
            LOG(WARNING) << "user provided hole is not a real hole";
            return false;
        }

        bool ok = false;

        // lock vertices/edge that already exist, to be later able to
        // identify the filled-in vertices/edges
        vlocked_ =
                mesh_->add_vertex_property<bool>("SurfaceMeshHoleFilling:vlocked", false);
        elocked_ =
                mesh_->add_edge_property<bool>("SurfaceMeshHoleFilling:elocked", false);
        for (auto v : mesh_->vertices())
            vlocked_[v] = true;
        for (auto e : mesh_->edges())
            elocked_[e] = true;

        // first do minimal triangulation
        if (triangulate_hole(_h)) {
            // refine filled-in edges
            refine();
            ok = true;
        }

        // clean up
        hole_.clear();
        mesh_->remove_vertex_property(vlocked_);
        mesh_->remove_edge_property(elocked_);

        return ok;
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshHoleFilling::triangulate_hole(SurfaceMesh::Halfedge _h) {
        // trace hole
        hole_.clear();
        SurfaceMesh::Halfedge h = _h;
        do {
            // check for manifoldness
            if (!mesh_->is_manifold(mesh_->target(h))) {
                LOG(ERROR) << "model has a non-manifold hole that cannot be filled";
                return false;
            }

            hole_.push_back(h);
        } while ((h = mesh_->next(h)) != _h);
        const int n = static_cast<int>(hole_.size());

        // compute minimal triangulation by dynamic programming
        weight_.clear();
        weight_.resize(n, std::vector<Weight>(n, Weight()));
        index_.clear();
        index_.resize(n, std::vector<int>(n, 0));

        Weight w, wmin;

        // initialize 2-gons
        for (int i = 0; i < n - 1; ++i) {
            weight_[i][i + 1] = Weight(0, 0);
            index_[i][i + 1] = -1;
        }

        // n-gons with n>2
        for (int j = 2; j < n; ++j) {
            // for all n-gons [i,i+j]
            for (int i = 0; i < n - j; ++i) {
                int k = i + j;
                wmin = Weight();
                int imin = -1;

                // find best split i < m < i+j
                for (int m = i + 1; m < k; ++m) {
                    w = weight_[i][m] + compute_weight(i, m, k) + weight_[m][k];
                    if (w < wmin) {
                        wmin = w;
                        imin = m;
                    }
                }

                weight_[i][k] = wmin;
                index_[i][k] = imin;
            }
        }

        // now add triangles to mesh
        std::vector<ivec2> todo;
        todo.reserve(n);
        todo.emplace_back(ivec2(0, n - 1));
        while (!todo.empty()) {
            ivec2 tri = todo.back();
            todo.pop_back();
            int start = tri[0];
            int end = tri[1];
            if (end - start < 2)
                continue;
            int split = index_[start][end];

            // if one of the vertices is not valid (user input is not a valid hole, or maybe due to
            // some complicated topological difficulties) -> stop filling and return
            if (!hole_vertex(start).is_valid() || !hole_vertex(split).is_valid() || !hole_vertex(end).is_valid()) {
                LOG(ERROR) << "invalid hole (or complicated topology)";
                return false;
            }

            mesh_->add_triangle(hole_vertex(start), hole_vertex(split), hole_vertex(end));

            todo.emplace_back(ivec2(start, split));
            todo.emplace_back(ivec2(split, end));
        }

        // clean up
        weight_.clear();
        index_.clear();

        return true;
    }

    //-----------------------------------------------------------------------------

    SurfaceMeshHoleFilling::Weight SurfaceMeshHoleFilling::compute_weight(int _i, int _j,
                                                                          int _k) const {
        const SurfaceMesh::Vertex a = hole_vertex(_i);
        const SurfaceMesh::Vertex b = hole_vertex(_j);
        const SurfaceMesh::Vertex c = hole_vertex(_k);
        // if one of the vertices is not valid (user input is not a valid hole, or maybe due to
        // some complicated topological difficulties) -> prevent by giving an infinite weight
        if (!a.is_valid() || !b.is_valid() || !c.is_valid()) {
            LOG(ERROR) << "invalid hole (or complicated topology)";
            return Weight();
        }

        // if one of the potential edges already exists, this would result
        // in an invalid triangulation -> prevent by giving an infinite weight
        if (is_interior_edge(a, b) || is_interior_edge(b, c) ||
            is_interior_edge(c, a)) {
            return Weight();
        }

        // compute area
        const float area = compute_area(a, b, c);

        // compute dihedral angles with...
        float angle(0);
        const vec3 n = compute_normal(a, b, c);

        // ...neighbor to (i,j)
        SurfaceMesh::Vertex d = (_i + 1 == _j) ? opposite_vertex(_j) : hole_vertex(index_[_i][_j]);
        angle = std::max(angle, compute_angle(n, compute_normal(a, d, b)));

        // ...neighbor to (j,k)
        d = (_j + 1 == _k) ? opposite_vertex(_k) : hole_vertex(index_[_j][_k]);
        angle = std::max(angle, compute_angle(n, compute_normal(b, d, c)));

        // ...neighbor to (k,i) if (k,i)==(n-1, 0)
        if (_i == 0 && _k + 1 == (int) hole_.size()) {
            d = opposite_vertex(0);
            angle = std::max(angle, compute_angle(n, compute_normal(c, d, a)));
        }

        return Weight(angle, area);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshHoleFilling::refine() {
        const int n = static_cast<int>(hole_.size());

        // compute target edge length
        float l = 0.0f;
        for (int i = 0; i < n; ++i) {
            l += distance(points_[hole_vertex(i)],
                          points_[hole_vertex((i + 1) % n)]);
        }
        l /= (float) n;
        float lmin = 0.7f * l;
        float lmax = 1.5f * l;

        // do some iterations
        for (int iter = 0; iter < 10; ++iter) {
            split_long_edges(lmax);
            collapse_short_edges(lmin);
            flip_edges();
            relaxation();
        }
        fairing();
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshHoleFilling::split_long_edges(const float _lmax) {
        bool ok;
        int i;

        for (ok = false, i = 0; !ok && i < 10; ++i) {
            ok = true;

            for (auto e : mesh_->edges()) {
                if (!elocked_[e]) {
                    SurfaceMesh::Halfedge h10 = mesh_->halfedge(e, 0);
                    SurfaceMesh::Halfedge h01 = mesh_->halfedge(e, 1);
                    const vec3 &p0 = points_[mesh_->target(h10)];
                    const vec3 &p1 = points_[mesh_->target(h01)];

                    if (distance(p0, p1) > _lmax) {
                        mesh_->split(e, 0.5 * (p0 + p1));
                        ok = false;
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshHoleFilling::collapse_short_edges(const float _lmin) {
        bool ok;
        int i;

        for (ok = false, i = 0; !ok && i < 10; ++i) {
            ok = true;

            for (auto e : mesh_->edges()) {
                if (!mesh_->is_deleted(e) && !elocked_[e]) {
                    SurfaceMesh::Halfedge h10 = mesh_->halfedge(e, 0);
                    SurfaceMesh::Halfedge h01 = mesh_->halfedge(e, 1);
                    SurfaceMesh::Vertex v0 = mesh_->target(h10);
                    SurfaceMesh::Vertex v1 = mesh_->target(h01);
                    const vec3 &p0 = points_[v0];
                    const vec3 &p1 = points_[v1];

                    // edge too short?
                    if (distance(p0, p1) < _lmin) {
                        SurfaceMesh::Halfedge h;
                        if (!vlocked_[v0])
                            h = h01;
                        else if (!vlocked_[v1])
                            h = h10;

                        if (h.is_valid() && mesh_->is_collapse_ok(h)) {
                            mesh_->collapse(h);
                            ok = false;
                        }
                    }
                }
            }
        }

        mesh_->collect_garbage();
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshHoleFilling::flip_edges() {
        SurfaceMesh::Vertex v0, v1, v2, v3;
        SurfaceMesh::Halfedge h;
        int val0, val1, val2, val3;
        int val_opt0, val_opt1, val_opt2, val_opt3;
        int ve0, ve1, ve2, ve3, ve_before, ve_after;
        bool ok;
        int i;

        for (ok = false, i = 0; !ok && i < 10; ++i) {
            ok = true;

            for (auto e : mesh_->edges()) {
                if (!elocked_[e]) {
                    h = mesh_->halfedge(e, 0);
                    v0 = mesh_->target(h);
                    v2 = mesh_->target(mesh_->next(h));
                    h = mesh_->halfedge(e, 1);
                    v1 = mesh_->target(h);
                    v3 = mesh_->target(mesh_->next(h));

                    val0 = static_cast<int>(mesh_->valence(v0));
                    val1 = static_cast<int>(mesh_->valence(v1));
                    val2 = static_cast<int>(mesh_->valence(v2));
                    val3 = static_cast<int>(mesh_->valence(v3));

                    val_opt0 = (mesh_->is_border(v0) ? 4 : 6);
                    val_opt1 = (mesh_->is_border(v1) ? 4 : 6);
                    val_opt2 = (mesh_->is_border(v2) ? 4 : 6);
                    val_opt3 = (mesh_->is_border(v3) ? 4 : 6);

                    ve0 = (val0 - val_opt0);
                    ve1 = (val1 - val_opt1);
                    ve2 = (val2 - val_opt2);
                    ve3 = (val3 - val_opt3);

                    ve_before = ve0 * ve0 + ve1 * ve1 + ve2 * ve2 + ve3 * ve3;

                    --val0;
                    --val1;
                    ++val2;
                    ++val3;

                    ve0 = (val0 - val_opt0);
                    ve1 = (val1 - val_opt1);
                    ve2 = (val2 - val_opt2);
                    ve3 = (val3 - val_opt3);

                    ve_after = ve0 * ve0 + ve1 * ve1 + ve2 * ve2 + ve3 * ve3;

                    if (ve_before > ve_after && mesh_->is_flip_ok(e)) {
                        mesh_->flip(e);
                        ok = false;
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshHoleFilling::relaxation() {
        // properties
        SurfaceMesh::VertexProperty<int> idx =
                mesh_->add_vertex_property<int>("SurfaceMeshHoleFilling:idx", -1);

        // collect free vertices
        std::vector<SurfaceMesh::Vertex> vertices;
        vertices.reserve(mesh_->n_vertices());
        for (auto v : mesh_->vertices()) {
            if (!vlocked_[v]) {
                idx[v] = static_cast<int>(vertices.size());
                vertices.push_back(v);
            }
        }
        const int n = static_cast<int>(vertices.size());

        // setup matrix & rhs
        Eigen::MatrixXd B(n, 3);
        std::vector<Triplet> triplets;
        for (int i = 0; i < n; ++i) {
            SurfaceMesh::Vertex v = vertices[i];
            vec3 b(0, 0, 0);
            float c(0);

            for (auto vv : mesh_->vertices(v)) {
                if (vlocked_[vv])
                    b += points_[vv];
                else
                    triplets.emplace_back(i, idx[vv], -1.0);
                ++c;
            }

            if (vlocked_[v])
                b -= c * points_[v];
            else
                triplets.emplace_back(i, idx[v], c);

            B.row(i) = Eigen::Vector3d(b.x, b.y, b.z);
        }

        // solve least squares system
        SparseMatrix A(n, n);
        A.setFromTriplets(triplets.begin(), triplets.end());
        Eigen::SimplicialLDLT<SparseMatrix> solver(A);
        Eigen::MatrixXd X = solver.solve(B);
        if (solver.info() != Eigen::Success) {
            LOG(ERROR) << "SurfaceMeshHoleFilling failed to solve the linear system";
            return;
        }

        // copy solution to mesh vertices
        for (int i = 0; i < n; ++i) {
            const auto &tmp = X.row(i);
            points_[vertices[i]] = vec3(static_cast<float>(tmp(0)), static_cast<float>(tmp(1)), static_cast<float>(tmp(2)));
        }

        // clean up
        mesh_->remove_vertex_property(idx);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshHoleFilling::fairing() {
        // did the refinement insert new vertices?
        // if yes, then trigger fairing; otherwise don't.
        bool new_vertices = false;
        for (auto v : mesh_->vertices()) {
            if (!vlocked_[v]) {
                new_vertices = true;
                break;
            }
        }
        if (!new_vertices)
            return;

        // convert non-locked into selection
        auto vsel = mesh_->vertex_property<bool>("v:selected");
        for (auto v : mesh_->vertices())
            vsel[v] = !vlocked_[v];

        // fair new vertices
        SurfaceMeshFairing fairing(mesh_);
        fairing.minimize_curvature();

        // clean up
        mesh_->remove_vertex_property(vsel);
    }

}