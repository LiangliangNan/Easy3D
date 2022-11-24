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

#include <easy3d/algo/surface_mesh_remeshing.h>

#include <cmath>
#include <algorithm>

#include <easy3d/algo/triangle_mesh_kdtree.h>
#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/util/progress.h>

namespace easy3d {

    SurfaceMeshRemeshing::SurfaceMeshRemeshing(SurfaceMesh *mesh)
            : mesh_(mesh), refmesh_(nullptr), kd_tree_(nullptr) {
        if (!mesh_->is_triangle_mesh())
            LOG(ERROR) << "input is not a pure triangle mesh!";

        points_ = mesh_->vertex_property<vec3>("v:point");

        mesh_->update_vertex_normals();
        vnormal_ = mesh_->vertex_property<vec3>("v:normal");
    }

    SurfaceMeshRemeshing::~SurfaceMeshRemeshing() = default;

    void SurfaceMeshRemeshing::uniform_remeshing(float edge_length,
                                                 unsigned int iterations,
                                                 bool use_projection) {
        uniform_ = true;
        use_projection_ = use_projection;
        target_edge_length_ = edge_length;

        preprocessing();

        ProgressLogger progress(iterations, false, false);
        for (unsigned int i = 0; i < iterations; ++i) {
            if (progress.is_canceled()) {
                LOG(WARNING) << "remeshing surface mesh cancelled";
                return;
            }

            split_long_edges();

            mesh_->update_vertex_normals();

            collapse_short_edges();

            flip_edges();

            tangential_smoothing(5);
            progress.next();
        }

        remove_caps();

        postprocessing();
    }

    void SurfaceMeshRemeshing::adaptive_remeshing(float min_edge_length,
                                                  float max_edge_length,
                                                  float approx_error,
                                                  unsigned int iterations,
                                                  bool use_projection) {
        uniform_ = false;
        min_edge_length_ = min_edge_length;
        max_edge_length_ = max_edge_length;
        approx_error_ = approx_error;
        use_projection_ = use_projection;

        preprocessing();

        ProgressLogger progress(iterations, false, false);
        for (unsigned int i = 0; i < iterations; ++i) {
            if (progress.is_canceled()) {
                LOG(WARNING) << "remeshing surface mesh cancelled";
                return;
            }
            split_long_edges();

            mesh_->update_vertex_normals();

            collapse_short_edges();

            flip_edges();

            tangential_smoothing(5);
            progress.next();
        }

        remove_caps();

        postprocessing();
    }

    void SurfaceMeshRemeshing::preprocessing() {
        // properties
        vfeature_ = mesh_->vertex_property<bool>("v:feature", false);
        efeature_ = mesh_->edge_property<bool>("e:feature", false);
        vlocked_ = mesh_->add_vertex_property<bool>("v:locked:SurfaceMeshRemeshing", false);
        elocked_ = mesh_->add_edge_property<bool>("e:locked:SurfaceMeshRemeshing", false);
        vsizing_ = mesh_->add_vertex_property<float>("v:sizing:SurfaceMeshRemeshing");

        // lock unselected vertices if some vertices are selected
        auto vselected = mesh_->get_vertex_property<bool>("v:selected");
        if (vselected) {
            bool has_selection = false;
            for (auto v : mesh_->vertices()) {
                if (vselected[v]) {
                    has_selection = true;
                    break;
                }
            }

            if (has_selection) {
                for (auto v : mesh_->vertices()) {
                    vlocked_[v] = !vselected[v];
                }

                // lock an edge if one of its vertices is locked
                for (auto e : mesh_->edges()) {
                    elocked_[e] = (vlocked_[mesh_->vertex(e, 0)] ||
                                   vlocked_[mesh_->vertex(e, 1)]);
                }
            }
        }

        // lock feature corners
        for (auto v : mesh_->vertices()) {
            if (vfeature_[v]) {
                int c = 0;
                for (auto h : mesh_->halfedges(v))
                    if (efeature_[mesh_->edge(h)])
                        ++c;

                if (c != 2)
                    vlocked_[v] = true;
            }
        }

        // compute sizing field
        if (uniform_) {
            for (auto v : mesh_->vertices()) {
                vsizing_[v] = target_edge_length_;
            }
        } else {
            // compute curvature for all mesh vertices, using cotan or Cohen-Steiner
            // don't use two-ring neighborhood, since we otherwise compute
            // curvature over sharp features edges, leading to high curvatures.
            // prefer tensor analysis over cotan-Laplace, since the former is more
            // robust and gives better results on the boundary.
            SurfaceMeshCurvature curv(mesh_);
            curv.analyze_tensor(1, true);

            // use vsizing_ to store/smooth curvatures to avoid another vertex property

            // curvature values for feature vertices and boundary vertices
            // are not meaningful. mark them as negative values.
            for (auto v : mesh_->vertices()) {
                if (mesh_->is_border(v) || (vfeature_ && vfeature_[v]))
                    vsizing_[v] = -1.0;
                else
                    vsizing_[v] = curv.max_abs_curvature(v);
            }

            // curvature values might be noisy. smooth them.
            // don't consider feature vertices' curvatures.
            // don't consider boundary vertices' curvatures.
            // do this for two iterations, to propagate curvatures
            // from non-feature regions to feature vertices.
            for (int iters = 0; iters < 2; ++iters) {
                for (auto v: mesh_->vertices()) {
                    double w, ww = 0.0;
                    double c, cc = 0.0;

                    for (auto h : mesh_->halfedges(v)) {
                        c = vsizing_[mesh_->target(h)];
                        if (c > 0.0) {
                            w = std::max(0.0, geom::cotan_weight(mesh_, mesh_->edge(h)));
                            ww += w;
                            cc += w * c;
                        }
                    }

                    if (std::abs(ww) > std::numeric_limits<double>::min())
                        cc /= ww;
                    vsizing_[v] = static_cast<float>(cc);
                }
            }

            // now convert per-vertex curvature into target edge length
            for (auto v: mesh_->vertices()) {
                double c = vsizing_[v];

                // get edge length from curvature
                const double r = 1.0 / c;
                const double e = approx_error_;
                double h;
                if (e < r) {
                    // see mathworld: "circle segment" and "equilateral triangle"
                    //h = std::sqrt(2.0*r*e-e*e) * 3.0 / std::sqrt(3.0);
                    h = std::sqrt(6.0 * e * r - 3.0 * e * e); // simplified...
                } else {
                    // this does not really make sense
                    h = e * 3.0 / std::sqrt(3.0);
                }

                // clamp to min. and max. edge length
                if (h < min_edge_length_)
                    h = min_edge_length_;
                else if (h > max_edge_length_)
                    h = max_edge_length_;

                // store target edge length
                vsizing_[v] = static_cast<float>(h);
            }
        }

        if (use_projection_) {
            // build reference mesh
            refmesh_ = new SurfaceMesh();
            refmesh_->assign(*mesh_);
            refmesh_->update_vertex_normals();
            refpoints_ = refmesh_->vertex_property<vec3>("v:point");
            refnormals_ = refmesh_->vertex_property<vec3>("v:normal");

            // copy sizing field from mesh_
            refsizing_ = refmesh_->add_vertex_property<float>("v:sizing");
            for (auto v : refmesh_->vertices()) {
                refsizing_[v] = vsizing_[v];
            }

            // build kd-tree
            kd_tree_ = new TriangleMeshKdTree(refmesh_, 0);
        }
    }

    void SurfaceMeshRemeshing::postprocessing() {
        // delete kd-tree and reference mesh
        if (use_projection_) {
            delete kd_tree_;
            delete refmesh_;
        }

        // remove properties
        mesh_->remove_vertex_property(vlocked_);
        mesh_->remove_edge_property(elocked_);
        mesh_->remove_vertex_property(vsizing_);
    }

    void SurfaceMeshRemeshing::project_to_reference(SurfaceMesh::Vertex v) {
        if (!use_projection_) {
            return;
        }

        // find the closest triangle of reference mesh
        TriangleMeshKdTree::NearestNeighbor nn = kd_tree_->nearest(points_[v]);
        const vec3 p = nn.nearest;
        const SurfaceMesh::Face f = nn.face;
        if (!f.is_valid()) {
            LOG(WARNING) << "could not find the nearest face for " << v << " (" << points_[v] << ")";
            return;
        }

        // get face data
        SurfaceMesh::VertexAroundFaceCirculator fvIt = refmesh_->vertices(f);
        const vec3 p0 = refpoints_[*fvIt];
        const vec3 n0 = refnormals_[*fvIt];
        const float s0 = refsizing_[*fvIt];
        ++fvIt;
        const vec3 p1 = refpoints_[*fvIt];
        const vec3 n1 = refnormals_[*fvIt];
        const float s1 = refsizing_[*fvIt];
        ++fvIt;
        const vec3 p2 = refpoints_[*fvIt];
        const vec3 n2 = refnormals_[*fvIt];
        const float s2 = refsizing_[*fvIt];

        // get barycentric coordinates
        const vec3 b = geom::barycentric_coordinates(p, p0, p1, p2);

        // interpolate normal
        vec3 n;
        n = (n0 * b[0]);
        n += (n1 * b[1]);
        n += (n2 * b[2]);
        n.normalize();
        assert(!std::isnan(n[0]));

        // interpolate sizing field
        float s;
        s = (s0 * b[0]);
        s += (s1 * b[1]);
        s += (s2 * b[2]);

        // set result
        points_[v] = p;
        vnormal_[v] = n;
        vsizing_[v] = s;
    }

    void SurfaceMeshRemeshing::split_long_edges() {
        SurfaceMesh::Vertex vnew, v0, v1;
        SurfaceMesh::Edge enew, e0, e1;
        SurfaceMesh::Face f0, f1, f2, f3;
        bool ok, is_feature, is_boundary;
        int i;

        for (ok = false, i = 0; !ok && i < 10; ++i) {
            ok = true;

            for (auto e : mesh_->edges()) {
                v0 = mesh_->vertex(e, 0);
                v1 = mesh_->vertex(e, 1);

                if (!elocked_[e] && is_too_long(v0, v1)) {
                    const vec3 &p0 = points_[v0];
                    const vec3 &p1 = points_[v1];

                    is_feature = efeature_[e];
                    is_boundary = mesh_->is_border(e);

                    vnew = mesh_->add_vertex((p0 + p1) * 0.5f);
                    mesh_->split(e, vnew);

                    // need normal or sizing for adaptive refinement
                    vnormal_[vnew] = mesh_->compute_vertex_normal(vnew);
                    vsizing_[vnew] = 0.5f * (vsizing_[v0] + vsizing_[v1]);

                    if (is_feature) {
                        enew = is_boundary ? SurfaceMesh::Edge(mesh_->n_edges() - 2)
                                           : SurfaceMesh::Edge(mesh_->n_edges() - 3);
                        efeature_[enew] = true;
                        vfeature_[vnew] = true;
                    } else {
                        project_to_reference(vnew);
                    }

                    ok = false;
                }
            }
        }
    }

    void SurfaceMeshRemeshing::collapse_short_edges() {
        SurfaceMesh::Vertex v0, v1;
        SurfaceMesh::Halfedge h0, h1, h01, h10;
        bool ok, b0, b1, l0, l1, f0, f1;
        int i;
        bool hcol01, hcol10;

        for (ok = false, i = 0; !ok && i < 10; ++i) {
            ok = true;

            for (auto e : mesh_->edges()) {
                if (!mesh_->is_deleted(e) && !elocked_[e]) {
                    h10 = mesh_->halfedge(e, 0);
                    h01 = mesh_->halfedge(e, 1);
                    v0 = mesh_->target(h10);
                    v1 = mesh_->target(h01);

                    if (is_too_short(v0, v1)) {
                        // get status
                        b0 = mesh_->is_border(v0);
                        b1 = mesh_->is_border(v1);
                        l0 = vlocked_[v0];
                        l1 = vlocked_[v1];
                        f0 = vfeature_[v0];
                        f1 = vfeature_[v1];
                        hcol01 = hcol10 = true;

                        // boundary rules
                        if (b0 && b1) {
                            if (!mesh_->is_border(e))
                                continue;
                        } else if (b0)
                            hcol01 = false;
                        else if (b1)
                            hcol10 = false;

                        // locked rules
                        if (l0 && l1)
                            continue;
                        else if (l0)
                            hcol01 = false;
                        else if (l1)
                            hcol10 = false;

                        // feature rules
                        if (f0 && f1) {
                            // edge must be a feature
                            if (!efeature_[e])
                                continue;

                            // the other two edges removed by collapse must not be features
                            h0 = mesh_->prev(h01);
                            h1 = mesh_->next(h10);
                            if (efeature_[mesh_->edge(h0)] ||
                                efeature_[mesh_->edge(h1)])
                                hcol01 = false;
                            // the other two edges removed by collapse must not be features
                            h0 = mesh_->prev(h10);
                            h1 = mesh_->next(h01);
                            if (efeature_[mesh_->edge(h0)] ||
                                efeature_[mesh_->edge(h1)])
                                hcol10 = false;
                        } else if (f0)
                            hcol01 = false;
                        else if (f1)
                            hcol10 = false;

                        // topological rules
                        bool collapse_ok = mesh_->is_collapse_ok(h01);

                        if (hcol01)
                            hcol01 = collapse_ok;
                        if (hcol10)
                            hcol10 = collapse_ok;

                        // both collapses possible: collapse into vertex w/ higher valence
                        if (hcol01 && hcol10) {
                            if (mesh_->valence(v0) < mesh_->valence(v1))
                                hcol10 = false;
                            else
                                hcol01 = false;
                        }

                        // try v1 -> v0
                        if (hcol10) {
                            // don't create too long edges
                            for (auto vv : mesh_->vertices(v1)) {
                                if (is_too_long(v0, vv)) {
                                    hcol10 = false;
                                    break;
                                }
                            }

                            if (hcol10) {
                                mesh_->collapse(h10);
                                ok = false;
                            }
                        }

                            // try v0 -> v1
                        else if (hcol01) {
                            // don't create too long edges
                            for (auto vv : mesh_->vertices(v0)) {
                                if (is_too_long(v1, vv)) {
                                    hcol01 = false;
                                    break;
                                }
                            }

                            if (hcol01) {
                                mesh_->collapse(h01);
                                ok = false;
                            }
                        }
                    }
                }
            }
        }

        mesh_->collect_garbage();
    }

    void SurfaceMeshRemeshing::flip_edges() {
        SurfaceMesh::Vertex v0, v1, v2, v3;
        SurfaceMesh::Halfedge h;
        int val0, val1, val2, val3;
        int val_opt0, val_opt1, val_opt2, val_opt3;
        int ve0, ve1, ve2, ve3, ve_before, ve_after;
        bool ok;
        int i;

        // precompute valences
        SurfaceMesh::VertexProperty<int> valence = mesh_->add_vertex_property<int>("valence");
        for (auto v : mesh_->vertices()) {
            valence[v] = mesh_->valence(v);
        }

        for (ok = false, i = 0; !ok && i < 10; ++i) {
            ok = true;

            for (auto e : mesh_->edges()) {
                if (!elocked_[e] && !efeature_[e]) {
                    h = mesh_->halfedge(e, 0);
                    v0 = mesh_->target(h);
                    v2 = mesh_->target(mesh_->next(h));
                    h = mesh_->halfedge(e, 1);
                    v1 = mesh_->target(h);
                    v3 = mesh_->target(mesh_->next(h));

                    if (!vlocked_[v0] && !vlocked_[v1] && !vlocked_[v2] &&
                        !vlocked_[v3]) {
                        val0 = valence[v0];
                        val1 = valence[v1];
                        val2 = valence[v2];
                        val3 = valence[v3];

                        val_opt0 = (mesh_->is_border(v0) ? 4 : 6);
                        val_opt1 = (mesh_->is_border(v1) ? 4 : 6);
                        val_opt2 = (mesh_->is_border(v2) ? 4 : 6);
                        val_opt3 = (mesh_->is_border(v3) ? 4 : 6);

                        ve0 = (val0 - val_opt0);
                        ve1 = (val1 - val_opt1);
                        ve2 = (val2 - val_opt2);
                        ve3 = (val3 - val_opt3);

                        ve0 *= ve0;
                        ve1 *= ve1;
                        ve2 *= ve2;
                        ve3 *= ve3;

                        ve_before = ve0 + ve1 + ve2 + ve3;

                        --val0;
                        --val1;
                        ++val2;
                        ++val3;

                        ve0 = (val0 - val_opt0);
                        ve1 = (val1 - val_opt1);
                        ve2 = (val2 - val_opt2);
                        ve3 = (val3 - val_opt3);

                        ve0 *= ve0;
                        ve1 *= ve1;
                        ve2 *= ve2;
                        ve3 *= ve3;

                        ve_after = ve0 + ve1 + ve2 + ve3;

                        if (ve_before > ve_after && mesh_->is_flip_ok(e)) {
                            mesh_->flip(e);
                            --valence[v0];
                            --valence[v1];
                            ++valence[v2];
                            ++valence[v3];
                            ok = false;
                        }
                    }
                }
            }
        }

        mesh_->remove_vertex_property(valence);
    }

    void SurfaceMeshRemeshing::tangential_smoothing(unsigned int iterations) {
        SurfaceMesh::Vertex v1, v2, v3, vv;
        SurfaceMesh::Edge e;
        float w, ww;
        vec3 u, n, t, b;

        // add property
        SurfaceMesh::VertexProperty <vec3> update = mesh_->add_vertex_property<vec3>("v:update");

        // project at the beginning to get valid sizing values and normal vectors
        // for vertices introduced by splitting
        if (use_projection_) {
            for (auto v : mesh_->vertices()) {
                if (!mesh_->is_border(v) && !vlocked_[v]) {
                    project_to_reference(v);
                }
            }
        }

        for (unsigned int iters = 0; iters < iterations; ++iters) {
            for (auto v : mesh_->vertices()) {
                if (!mesh_->is_border(v) && !vlocked_[v]) {
                    if (vfeature_[v]) {
                        u = vec3(0.0);
                        t = vec3(0.0);
                        ww = 0;
                        int c = 0;

                        for (auto h : mesh_->halfedges(v)) {
                            if (efeature_[mesh_->edge(h)]) {
                                vv = mesh_->target(h);

                                b = points_[v];
                                b += points_[vv];
                                b *= 0.5;

                                w = distance(points_[v], points_[vv]) /
                                    (0.5 * (vsizing_[v] + vsizing_[vv]));
                                ww += w;
                                u += w * b;

                                if (c == 0) {
                                    t += normalize(points_[vv] - points_[v]);
                                    ++c;
                                } else {
                                    ++c;
                                    t -= normalize(points_[vv] - points_[v]);
                                }
                            }
                        }

                        assert(c == 2);

                        u *= (1.0 / ww);
                        u -= points_[v];
                        t = normalize(t);
                        u = t * dot(u, t);

                        update[v] = u;
                    } else {
                        vec3 p(0);
                        try {
                            p = minimize_squared_areas(v);
                        }
                        catch (std::exception &e) {
                            (void)e;
                            p = weighted_centroid(v);
                        }
                        u = p - mesh_->position(v);

                        n = vnormal_[v];
                        u -= n * dot(u, n);

                        update[v] = u;
                    }
                }
            }

            // update vertex positions
            for (auto v : mesh_->vertices()) {
                if (!mesh_->is_border(v) && !vlocked_[v]) {
                    points_[v] += update[v];
                }
            }

            // update normal vectors (if not done so through projection)
            mesh_->update_vertex_normals();
        }

        // project at the end
        if (use_projection_) {
            for (auto v : mesh_->vertices()) {
                if (!mesh_->is_border(v) && !vlocked_[v]) {
                    project_to_reference(v);
                }
            }
        }

        // remove property
        mesh_->remove_vertex_property(update);
    }

    void SurfaceMeshRemeshing::remove_caps() {
        SurfaceMesh::Halfedge h;
        SurfaceMesh::Vertex v, vb, vd;
        SurfaceMesh::Face fb, fd;
        double a0, a1, amin, aa(std::cos(170.0 * M_PI / 180.0));
        vec3 a, b, c, d;

        for (auto e : mesh_->edges()) {
            if (!elocked_[e] && mesh_->is_flip_ok(e)) {
                h = mesh_->halfedge(e, 0);
                a = points_[mesh_->target(h)];

                h = mesh_->next(h);
                b = points_[vb = mesh_->target(h)];

                h = mesh_->halfedge(e, 1);
                c = points_[mesh_->target(h)];

                h = mesh_->next(h);
                d = points_[vd = mesh_->target(h)];

                a0 = dot(normalize(a - b), normalize(c - b));
                a1 = dot(normalize(a - d), normalize(c - d));

                if (a0 < a1) {
                    amin = a0;
                    v = vb;
                } else {
                    amin = a1;
                    v = vd;
                }

                // is it a cap?
                if (amin < aa) {
                    // feature edge and feature vertex -> seems to be intended
                    if (efeature_[e] && vfeature_[v])
                        continue;

                    // project v onto feature edge
                    if (efeature_[e])
                        points_[v] = (a + c) * 0.5f;

                    // flip
                    mesh_->flip(e);
                }
            }
        }
    }

    vec3 SurfaceMeshRemeshing::minimize_squared_areas(SurfaceMesh::Vertex v) {
        dmat3 A(0.0), D;
        dvec3 b(0), d, p, q, x;
        double w;

        for (auto h : mesh_->halfedges(v)) {
            assert(!mesh_->is_border(h));

            // get edge opposite to vertex v
            SurfaceMesh::Vertex v0 = mesh_->target(h);
            SurfaceMesh::Vertex v1 = mesh_->target(mesh_->next(h));
            p = (dvec3) points_[v0];
            q = (dvec3) points_[v1];
            d = q - p;
            w = 1.0 / norm(d);

            // build squared cross-product-with-d matrix
            D(0, 0) = d[1] * d[1] + d[2] * d[2];
            D(1, 1) = d[0] * d[0] + d[2] * d[2];
            D(2, 2) = d[0] * d[0] + d[1] * d[1];
            D(1, 0) = D(0, 1) = -d[0] * d[1];
            D(2, 0) = D(0, 2) = -d[0] * d[2];
            D(1, 2) = D(2, 1) = -d[1] * d[2];
            A += w * D;

            // build right-hand side
            b += w * D * p;
        }

        // compute minimizer
        try {
            x = inverse(A) * b;
        }
        catch (...) {
            std::string what = "SurfaceMeshRemeshing: matrix not invertible.";
            throw std::runtime_error(what);
        }

        return vec3(x);
    }

    vec3 SurfaceMeshRemeshing::weighted_centroid(SurfaceMesh::Vertex v) {
        auto p = vec3(0);
        double ww = 0;

        for (auto h : mesh_->halfedges(v)) {
            auto v1 = v;
            auto v2 = mesh_->target(h);
            auto v3 = mesh_->target(mesh_->next(h));

            auto b = points_[v1];
            b += points_[v2];
            b += points_[v3];
            b *= (1.0 / 3.0);

            double area =
                    norm(cross(points_[v2] - points_[v1], points_[v3] - points_[v1]));

            // take care of degenerate faces to avoid all zero weights and division
            // by zero later on
            if (area == 0)
                area = 1.0;

            double w =
                    area / pow((vsizing_[v1] + vsizing_[v2] + vsizing_[v3]) / 3.0, 2.0);

            p += w * b;
            ww += w;
        }

        p /= ww;

        return p;
    }

} // namespace easy3d