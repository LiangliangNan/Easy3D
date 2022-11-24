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

#include <easy3d/algo/surface_mesh_triangulation.h>


namespace easy3d {

    SurfaceMeshTriangulation::SurfaceMeshTriangulation(SurfaceMesh *mesh) : mesh_(mesh) {
        points_ = mesh_->get_vertex_property<vec3>("v:point");
        objective_ = MIN_AREA;
        objective_ = MAX_ANGLE;
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshTriangulation::triangulate(Objective o) {
        for (auto f: mesh_->faces())
            triangulate(f, o);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshTriangulation::triangulate(SurfaceMesh::Face f, Objective o) {
        // store objective
        objective_ = o;


        // collect polygon halfedges
        SurfaceMesh::Halfedge h0 = mesh_->halfedge(f);
        halfedges_.clear();
        vertices_.clear();
        SurfaceMesh::Halfedge h = h0;
        do {
            if (!mesh_->is_manifold(mesh_->target(h))) {
                std::cerr << "[SurfaceMeshTriangulation] Non-manifold polygon\n";
                return;
            }

            halfedges_.push_back(h);
            vertices_.push_back(mesh_->target(h));
        } while ((h = mesh_->next(h)) != h0);

        // do we have at least four vertices?
        const int n = static_cast<int>(halfedges_.size());
        if (n <= 3) return;

        // compute minimal triangulation by dynamic programming
        weight_.clear();
        weight_.resize(n, std::vector<float>(n, std::numeric_limits<float>::max()));
        index_.clear();
        index_.resize(n, std::vector<int>(n, 0));

        int i, j, m, k, imin;
        float w, wmin;

        // initialize 2-gons
        for (i = 0; i < n - 1; ++i) {
            weight_[i][i + 1] = 0.0;
            index_[i][i + 1] = -1;
        }

        // n-gons with n>2
        for (j = 2; j < n; ++j) {
            // for all n-gons [i,i+j]
            for (i = 0; i < n - j; ++i) {
                k = i + j;
                wmin = std::numeric_limits<float>::max();
                imin = -1;

                // find best split i < m < i+j
                for (m = i + 1; m < k; ++m) {
                    switch (objective_) {
                        case MIN_AREA:
                            w = weight_[i][m] + compute_weight(i, m, k) + weight_[m][k];
                            break;
                        case MAX_ANGLE:
                            w = std::max(weight_[i][m], std::max(compute_weight(i, m, k), weight_[m][k]));
                            break;
                        default:
                            // should never happen
                            LOG(ERROR) << "should never happen";
                            break;
                    }

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

            insert_edge(start, split);
            insert_edge(split, end);

            todo.emplace_back(ivec2(start, split));
            todo.emplace_back(ivec2(split, end));
        }


        // clean up
        weight_.clear();
        index_.clear();
        halfedges_.clear();
        vertices_.clear();
    }

    //-----------------------------------------------------------------------------

    float SurfaceMeshTriangulation::compute_weight(int i, int j, int k) const {
        const SurfaceMesh::Vertex a = vertices_[i];
        const SurfaceMesh::Vertex b = vertices_[j];
        const SurfaceMesh::Vertex c = vertices_[k];

        // If one of the potential edges already exists this would result in an
        // invalid triangulation. This happens for suzanne.obj. Prevent this by
        // giving infinite weight.
        if (is_edge(a, b) && is_edge(b, c) && is_edge(c, a))
            return std::numeric_limits<float>::max();

        const vec3 &pa = points_[a];
        const vec3 &pb = points_[b];
        const vec3 &pc = points_[c];

        float w = std::numeric_limits<float>::max();
        switch (objective_) {
            // compute squared triangle area
            case MIN_AREA:
                w = length2(cross(pb - pa, pc - pa));
                break;

                // compute one over minimum angle
                // or cosine of minimum angle
                // maximum cosine (which should then be minimized)
            case MAX_ANGLE:
                float cosa = dot(normalize(pb - pa), normalize(pc - pa));
                float cosb = dot(normalize(pa - pb), normalize(pc - pb));
                float cosc = dot(normalize(pa - pc), normalize(pb - pc));
                w = std::max(cosa, std::max(cosb, cosc));
                break;
        }

        return w;
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshTriangulation::is_edge(SurfaceMesh::Vertex a, SurfaceMesh::Vertex b) const {
        return mesh_->find_halfedge(a, b).is_valid();
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshTriangulation::insert_edge(int i, int j) {
        SurfaceMesh::Halfedge h0 = halfedges_[i];
        SurfaceMesh::Halfedge h1 = halfedges_[j];
        SurfaceMesh::Vertex v0 = vertices_[i];
        SurfaceMesh::Vertex v1 = vertices_[j];

        // does edge already exist?
        if (mesh_->find_halfedge(v0, v1).is_valid()) {
            return false;
        }

        // can we reach v1 from h0?
        {
            SurfaceMesh::Halfedge h = h0;
            do {
                h = mesh_->next(h);
                if (mesh_->target(h) == v1) {
                    mesh_->insert_edge(h0, h);
                    return true;
                }
            } while (h != h0);
        }

        // can we reach v0 from h1?
        {
            SurfaceMesh::Halfedge h = h1;
            do {
                h = mesh_->next(h);
                if (mesh_->target(h) == v0) {
                    mesh_->insert_edge(h1, h);
                    return true;
                }
            } while (h != h1);
        }

        std::cerr << "[SurfaceMeshTriangulation] This should not happen...\n";
        return false;
    }

}