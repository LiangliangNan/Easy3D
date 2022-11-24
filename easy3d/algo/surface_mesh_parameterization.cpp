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

#include <easy3d/algo/surface_mesh_parameterization.h>

#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    SurfaceMeshParameterization::SurfaceMeshParameterization(SurfaceMesh *mesh)
            : mesh_(mesh) {
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshParameterization::setup_boundary_constraints() {
        // get properties
        auto points = mesh_->vertex_property<vec3>("v:point");
        auto tex = mesh_->vertex_property<vec2>("v:texcoord");

        SurfaceMesh::VertexIterator vit, vend = mesh_->vertices_end();
        SurfaceMesh::Vertex vh;
        SurfaceMesh::Halfedge hh;
        std::vector<SurfaceMesh::Vertex> loop;

        // Initialize all texture coordinates to the origin.
        for (auto v : mesh_->vertices())
            tex[v] = vec2(0.5, 0.5);

        // find 1st boundary vertex
        for (vit = mesh_->vertices_begin(); vit != vend; ++vit)
            if (mesh_->is_border(*vit))
                break;

        // no boundary found ?
        if (vit == vend) {
            LOG(WARNING) << "SurfaceMeshParameterization works only on meshes with boundaries.";
            return false;
        }

        // collect boundary loop
        vh = *vit;
        hh = mesh_->out_halfedge(vh);
        do {
            loop.push_back(mesh_->target(hh));
            hh = mesh_->next(hh);
        } while (hh != mesh_->out_halfedge(vh));

        // map boundary loop to unit circle in texture domain
        unsigned int i, n = loop.size();
        float angle, l, length;
        vec2 t;

        // compute length of boundary loop
        for (i = 0, length = 0.0; i < n; ++i)
            length += distance(points[loop[i]], points[loop[(i + 1) % n]]);

        // map length intervals to unit circle intervals
        for (i = 0, l = 0.0; i < n;) {
            // go from 2pi to 0 to preserve orientation
            angle = 2.0 * M_PI * (1.0 - l / length);

            t[0] = 0.5 + 0.5 * cosf(angle);
            t[1] = 0.5 + 0.5 * sinf(angle);

            tex[loop[i]] = t;

            ++i;
            if (i < n) {
                l += distance(points[loop[i]], points[loop[(i + 1) % n]]);
            }
        }

        return true;
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshParameterization::harmonic(bool use_uniform_weights) {
        // map boundary to circle
        if (!setup_boundary_constraints()) {
            LOG(ERROR) << "failed setup boundary constraints.";
            return;
        }

        // get properties
        auto tex = mesh_->vertex_property<vec2>("v:texcoord");
        auto eweight = mesh_->add_edge_property<float>("e:param:SurfaceMeshParameterization");
        auto idx = mesh_->add_vertex_property<int>("v:idx:SurfaceMeshParameterization", -1);

        // compute Laplace weight per edge: cotan or uniform
        for (auto e : mesh_->edges()) {
            eweight[e] = use_uniform_weights ? 1.0 : std::max(0.0, geom::cotan_weight(mesh_, e));
        }

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

        // setup matrix A and rhs B
        const unsigned int n = free_vertices.size();
        Eigen::SparseMatrix<double> A(n, n);
        Eigen::MatrixXd B(n, 2);
        std::vector<Eigen::Triplet<double>>
                triplets;
        dvec2 b;
        double w, ww;
        SurfaceMesh::Vertex v, vv;
        SurfaceMesh::Edge e;
        for (i = 0; i < n; ++i) {
            v = free_vertices[i];

            // rhs row
            b = dvec2(0.0);

            // lhs row
            ww = 0.0;
            for (auto h : mesh_->halfedges(v)) {
                vv = mesh_->target(h);
                e = mesh_->edge(h);
                w = eweight[e];
                ww += w;

                if (mesh_->is_border(vv)) {
                    b -= -w * static_cast<dvec2>(tex[vv]);
                } else {
                    triplets.emplace_back(i, idx[vv], -w);
                }
            }
            triplets.emplace_back(i, i, ww);
            B.row(i) = (Eigen::Vector2d) b;
        }

        // build sparse matrix from triplets
        A.setFromTriplets(triplets.begin(), triplets.end());

        // solve A*X = B
        Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(A);
        Eigen::MatrixXd X = solver.solve(B);
        if (solver.info() != Eigen::Success) {
            LOG(ERROR) << "failed solving the linear system.";
        } else {
            // copy solution
            for (i = 0; i < n; ++i) {
                const auto &tmp = X.row(i);
                tex[free_vertices[i]] = vec2(tmp(0), tmp(1));
            }
        }

        // clean-up
        mesh_->remove_vertex_property(idx);
        mesh_->remove_edge_property(eweight);
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshParameterization::setup_lscm_boundary() {
        // constrain the two boundary vertices farthest from each other to fix
        // the translation and rotation of the resulting parameterization

        // vertex properties
        auto pos = mesh_->vertex_property<vec3>("v:point");
        auto tex = mesh_->vertex_property<vec2>("v:texcoord");
        auto locked = mesh_->add_vertex_property<bool>("v:locked:SurfaceMeshParameterization", false);

        // find boundary vertices and store handles in vector
        std::vector<SurfaceMesh::Vertex> boundary;
        for (auto v : mesh_->vertices())
            if (mesh_->is_border(v))
                boundary.push_back(v);

        // no boundary?
        if (boundary.empty()) {
            return false;
        }

        // find boundary vertices with the largest distance
        float diam(0.0), d;
        SurfaceMesh::Vertex v1, v2;
        for (auto vv1 : boundary) {
            for (auto vv2 : boundary) {
                d = distance(pos[vv1], pos[vv2]);
                if (d > diam) {
                    diam = d;
                    v1 = vv1;
                    v2 = vv2;
                }
            }
        }

        // pin these two boundary vertices
        for (auto v : mesh_->vertices()) {
            tex[v] = vec2(0.5, 0.5);
            locked[v] = false;
        }
        tex[v1] = vec2(0.0, 0.0);
        tex[v2] = vec2(1.0, 1.0);
        locked[v1] = true;
        locked[v2] = true;

        return true;
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshParameterization::lscm() {
        // boundary constraints
        if (!setup_lscm_boundary())
            return;

        // properties
        auto pos = mesh_->vertex_property<vec3>("v:point");
        auto tex = mesh_->vertex_property<vec2>("v:texcoord");
        auto idx = mesh_->add_vertex_property<int>("v:idx:SurfaceMeshParameterization", -1);
        auto weight = mesh_->add_halfedge_property<dvec2>("h:lscm:SurfaceMeshParameterization");
        auto locked = mesh_->get_vertex_property<bool>("v:locked:SurfaceMeshParameterization");
        assert(locked);

        // compute weights/gradients per face/halfedge
        for (auto f : mesh_->faces()) {
            // collect face halfedge
            auto fh_it = mesh_->halfedges(f);
            auto ha = *fh_it;
            ++fh_it;
            auto hb = *fh_it;
            ++fh_it;
            auto hc = *fh_it;

            // collect face vertices
            dvec3 a = (dvec3) pos[mesh_->target(ha)];
            dvec3 b = (dvec3) pos[mesh_->target(hb)];
            dvec3 c = (dvec3) pos[mesh_->target(hc)];

            // calculate local coordinate system
            dvec3 z = normalize(cross(normalize(c - b), normalize(a - b)));
            dvec3 x = normalize(b - a);
            dvec3 y = normalize(cross(z, x));

            // calculate local vertex coordinates
            dvec2 a2d(0.0, 0.0);
            dvec2 b2d(norm(b - a), 0.0);
            dvec2 c2d(dot(c - a, x), dot(c - a, y));

            // calculate double triangle area
            z = cross(c - b, a - b);
            double area = norm(z);
            if (area)
                area = 1.0 / area;

            // uncommend the following line if your meshes are very bad...
            //area = 1.0;

            // calculate W_j,Ti (index by corner a,b,c and real/imaginary)
            double w_ar = c2d[0] - b2d[0];
            double w_br = a2d[0] - c2d[0];
            double w_cr = b2d[0] - a2d[0];
            double w_ai = c2d[1] - b2d[1];
            double w_bi = a2d[1] - c2d[1];
            double w_ci = b2d[1] - a2d[1];

            // store matrix information per halfedge
            weight[ha] = dvec2(w_ar * area, w_ai * area);
            weight[hb] = dvec2(w_br * area, w_bi * area);
            weight[hc] = dvec2(w_cr * area, w_ci * area);
        }

        // collect free (non-boundary) vertices in array free_vertices[]
        // assign indices such that idx[ free_vertices[i] ] == i
        int id = 0;
        std::vector<SurfaceMesh::Vertex> free_vertices;
        free_vertices.reserve(mesh_->n_vertices());
        for (auto v : mesh_->vertices()) {
            if (!locked[v]) {
                idx[v] = id++;
                free_vertices.push_back(v);
            }
        }

        // build matrix and rhs
        const unsigned int nv2 = 2 * mesh_->n_vertices();
        const unsigned int nv = mesh_->n_vertices();
        const unsigned int n = free_vertices.size();
        SurfaceMesh::Vertex vi, vj;
        double si, sj0, sj1, sign;
        int row(0), c0, c1;

        Eigen::SparseMatrix<double> A(2 * n, 2 * n);
        Eigen::VectorXd b = Eigen::VectorXd::Zero(2 * n);
        std::vector<Eigen::Triplet<double>>
                triplets;

        for (unsigned int i = 0; i < nv2; ++i) {
            vi = SurfaceMesh::Vertex(i % nv);

            if (i < nv) {
                sign = 1.0;
                c0 = 0;
                c1 = 1;
            } else {
                sign = -1.0;
                c0 = 1;
                c1 = 0;
            }

            if (!locked[vi]) {
                si = 0;

                for (auto h : mesh_->halfedges(vi)) {
                    vj = mesh_->target(h);
                    sj0 = sj1 = 0;

                    if (!mesh_->is_border(h)) {
                        const dvec2 &wj = weight[h];
                        const dvec2 &wi = weight[mesh_->prev(h)];

                        sj0 += sign * wi[c0] * wj[0] + wi[c1] * wj[1];
                        sj1 += -sign * wi[c0] * wj[1] + wi[c1] * wj[0];
                        si += wi[0] * wi[0] + wi[1] * wi[1];
                    }

                    h = mesh_->opposite(h);
                    if (!mesh_->is_border(h)) {
                        const dvec2 &wi = weight[h];
                        const dvec2 &wj = weight[mesh_->prev(h)];

                        sj0 += sign * wi[c0] * wj[0] + wi[c1] * wj[1];
                        sj1 += -sign * wi[c0] * wj[1] + wi[c1] * wj[0];
                        si += wi[0] * wi[0] + wi[1] * wi[1];
                    }

                    if (!locked[vj]) {
                        triplets.emplace_back(row, idx[vj], sj0);
                        triplets.emplace_back(row, idx[vj] + n, sj1);
                    } else {
                        b[row] -= sj0 * tex[vj][0];
                        b[row] -= sj1 * tex[vj][1];
                    }
                }

                triplets.emplace_back(row, idx[vi] + (i < nv ? 0 : n), 0.5 * si);

                ++row;
            }
        }

        // build sparse matrix from triplets
        A.setFromTriplets(triplets.begin(), triplets.end());

        // solve A*X = B
        Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(A);
        Eigen::VectorXd x = solver.solve(b);
        if (solver.info() != Eigen::Success) {
            LOG(ERROR) << "failed solving the linear system";
        } else {
            // copy solution
            for (unsigned int i = 0; i < n; ++i) {
                tex[free_vertices[i]] = vec2(x[i], x[i + n]);
            }
        }

        // scale tex coordiantes to unit square
        vec2 bbmin(1, 1), bbmax(0, 0);
        for (auto v : mesh_->vertices()) {
            bbmin = comp_min(bbmin, tex[v]);
            bbmax = comp_max(bbmax, tex[v]);
        }
        bbmax -= bbmin;
        float s = std::max(bbmax[0], bbmax[1]);
        for (auto v : mesh_->vertices()) {
            tex[v] -= bbmin;
            tex[v] /= s;
        }

        // clean-up
        mesh_->remove_vertex_property(idx);
        mesh_->remove_vertex_property(locked);
        mesh_->remove_halfedge_property(weight);
    }

} // namespace easy3d