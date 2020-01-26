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

#include <easy3d/core/manifold_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    ManifoldBuilder::ManifoldBuilder(SurfaceMesh *mesh)
            : mesh_(mesh) {
    }


    ManifoldBuilder::~ManifoldBuilder() {

    }


    void ManifoldBuilder::begin() {
        num_faces_less_three_vertices_ = 0;
        num_faces_duplicated_vertices_ = 0;
        num_faces_out_of_range_vertices_ = 0;
        num_faces_unknown_topology_ = 0;

        face_vertices_.clear();
        copied_vertices_.clear();

        original_vertex_ = mesh_->add_vertex_property<SurfaceMesh::Vertex>("v:ManifoldBuilder:original_vertex");
    }


    void ManifoldBuilder::end() {
        const std::string name = mesh_->name().empty() ? "with unknown name" : mesh_->name();
        std::string msg = "mesh has topological issues:";
        bool report(false);

        // ----------------------------------------------------------------------------------

        if (num_faces_less_three_vertices_ > 0) {
            msg += "\n\t\t" + std::to_string(num_faces_less_three_vertices_) +
                   " faces with less than 3 vertices (ignored)";
            report = true;
        }

        if (num_faces_duplicated_vertices_ > 0) {
            msg += "\n\t\t" + std::to_string(num_faces_duplicated_vertices_) +
                   " faces with duplicated vertices (ignored)";
            report = true;
        }

        if (num_faces_out_of_range_vertices_ > 0) {
            msg += "\n\t\t" + std::to_string(num_faces_out_of_range_vertices_) +
                   " faces with out-of-range vertices (ignored)";
            report = true;
        }

        // ----------------------------------------------------------------------------------

        if (num_faces_unknown_topology_ > 0) {
            msg += "\n\t\t" + std::to_string(num_faces_unknown_topology_) +
                   " complex faces with unknown topology (ignored)";
            report = true;
        }

        // ----------------------------------------------------------------------------------

        std::size_t count_non_manifold_vertices(0);

        // non-manifold vertices in the original mesh
        // vertices that have been copied and in current mesh they are closed disks.
        for (auto v : copied_vertices_) {
            int idx = v.first;
            if (!mesh_->is_boundary(SurfaceMesh::Vertex(idx)))
                ++count_non_manifold_vertices;
        }
#if 1 // non-manifold vertices in the current mesh
        for (auto v : mesh_->vertices()) {
            if (!mesh_->is_manifold(v))
                ++count_non_manifold_vertices;
        }
#endif
        if (count_non_manifold_vertices > 0) {
            msg += "\n\t\t" + std::to_string(count_non_manifold_vertices) + " non-manifold vertices (not fixed)";
            report = true;
        }

        // ----------------------------------------------------------------------------------

        // TODO: Traverse all the edge and check the number of occurrences to know where a edge is copied from
#if 1
        std::size_t count_non_manifold_edges(0);
        for (const auto &targets : outgoing_halfedges_) {
            std::set<int> tmp(targets.begin(), targets.end());
            count_non_manifold_edges += (targets.size() - tmp.size());
        }
        if (count_non_manifold_edges > 0) {
            msg += "\n\t\t" + std::to_string(count_non_manifold_edges) + " non-manifold edges (fixed)";
            report = true;
        }
#endif

        // ----------------------------------------------------------------------------------

        std::size_t count_isolated_vertices(0);
        for (auto v : mesh_->vertices()) {
            if (mesh_->is_isolated(v)) {
                mesh_->delete_vertex(v);
                ++count_isolated_vertices;
            }
        }
        if (count_isolated_vertices > 0) {
            mesh_->garbage_collection();
            msg += "\n\t\t" + std::to_string(count_isolated_vertices) + " isolated vertices (removed)";
            report = true;
        }

        // ----------------------------------------------------------------------------------

        if (!copied_vertices_.empty()) {
            std::size_t count(0);
            for (auto copies : copied_vertices_)
                count += copies.second.size();
            msg += "\n\tSolution: \n\t\tcopied " + std::to_string(copied_vertices_.size()) + " vertices (" + std::to_string(count) +
                   " occurrences) to ensure manifoldness";

            if (count_isolated_vertices > 0)
                msg += "\n\t\tdeleted " + std::to_string(count_isolated_vertices) + " isolated vertices";
        }

        // ----------------------------------------------------------------------------------

        if (report) {
            msg += "\n\tResult: \n\t\t" +
                    std::to_string(mesh_->faces_size()) + " faces\n\t\t" +
                    std::to_string(mesh_->vertices_size()) + " vertices\n\t\t" +
                    std::to_string(mesh_->edges_size()) + " edges";
        }

#if 0
        for (auto g : copied_vertices_) {
            msg += "\n\tvertex v" + std::to_string(g.first) + " copied to ";
            for (auto v : g.second)
                msg += "v" + std::to_string(v.idx()) + " ";
        }
#endif

        count_non_manifold_vertices = 0;
        for (auto v : mesh_->vertices()) {
            LOG_IF(ERROR, !mesh_->is_valid(v)) << "vertex " << v << " is not valid";
            if (!mesh_->is_manifold(v))
                ++count_non_manifold_vertices;
        }
        LOG_IF(ERROR, count_non_manifold_vertices > 0)
                        << "mesh still has " << count_non_manifold_vertices << " non-manifold vertices";

        for (auto f : mesh_->faces())
            LOG_IF(ERROR, !mesh_->is_valid(f)) << "face " << f << " is not valid";
        for (auto e : mesh_->edges())
            LOG_IF(ERROR, !mesh_->is_valid(e)) << "edge " << e << " is not valid";
        for (auto h : mesh_->halfedges())
            LOG_IF(ERROR, !mesh_->is_valid(h)) << "halfedge " << h << " is not valid";

        LOG_IF(WARNING, report) << msg;

        outgoing_halfedges_.clear();
        mesh_->remove_vertex_property(original_vertex_);
        LOG(WARNING) << "the original_vertex_ vertex property was actually not used";
    }


    SurfaceMesh::Vertex ManifoldBuilder::add_vertex(const vec3 &p) {
        LOG_IF(ERROR, mesh_->faces_size() > 0) << "vertices should be added before adding any face";
        return mesh_->add_vertex(p);
    }


    bool ManifoldBuilder::face_can_be_added(const std::vector<SurfaceMesh::Vertex> &vertices) {
        std::size_t nb_vertices = vertices.size();

        // Check #1: a face has less than 3 vertices
        if (nb_vertices < 3) {
            ++num_faces_less_three_vertices_;
            return false;
        }

        // Check #2; a face has duplicated vertices
        for (std::size_t i = 0; i < nb_vertices; ++i) {
            for (std::size_t j = i + 1; j < nb_vertices; ++j) {
                if (vertices[i] == vertices[j]) {
                    ++num_faces_duplicated_vertices_;
                    return false;
                }
            }
        }

        // Check #3; a face has out-of-range vertices
        for (auto v : vertices) {
            if (v.idx() < 0 || v.idx() >= mesh_->vertices_size()) {
                ++num_faces_out_of_range_vertices_;
                return false;
            }
        }

        // More check?
        // A face has already been added, i.e., a previously added face has the same vertex indices.
        // We should allow this by duplicating its vertices (to avoid discarding faces).

        return true;
    }


    bool ManifoldBuilder::halfedge_has_duplication(easy3d::SurfaceMesh::Vertex s, easy3d::SurfaceMesh::Vertex t) const {
#if 1 // This works perfectly
        const auto &targets = outgoing_halfedges_[s.idx()];
        for (auto v : targets)
            if (v == t.idx())
                return true;
        return false;

#else	// This also works, but slightly slower and longer code.

        auto h = mesh_->find_halfedge(s, t);
        if (h.is_valid() && !mesh_->is_boundary(h)) {
            return true;
        }

        // test the duplicated edges using EACH copy of s and t
        auto s_pos = copied_vertices_.find(s.idx());
        if (s_pos != copied_vertices_.end()) {
            const auto& s_copies = s_pos->second;
            for (auto v : s_copies) {
                h = mesh_->find_halfedge(v, t);
                if (h.is_valid() && !mesh_->is_boundary(h)) {
                    return true;
                }
            }
        }

        // test the duplicated edges using s and EACH copy of t
        auto t_pos = copied_vertices_.find(t.idx());
        if (t_pos != copied_vertices_.end()) {
            const auto& t_copies = t_pos->second;
            for (auto v : t_copies) {
                h = mesh_->find_halfedge(s, v);
                if (h.is_valid() && !mesh_->is_boundary(h)) {
                    return true;
                }
            }

            // if reached here, test all combinations of the copies
            if (s_pos != copied_vertices_.end()) {
                const auto& s_copies = s_pos->second;
                for (auto vs : s_copies) {
                    for (auto vt : t_copies) {
                        h = mesh_->find_halfedge(vs, vt);
                        if (h.is_valid() && !mesh_->is_boundary(h)) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
#endif
    }

    SurfaceMesh::Face ManifoldBuilder::add_face(const std::vector<SurfaceMesh::Vertex> &vertices) {
        if (mesh_->faces_size() == 0) // the first face
            outgoing_halfedges_.resize(mesh_->vertices_size());

        if (!face_can_be_added(vertices))
            return SurfaceMesh::Face();

        std::size_t nb_vertices = vertices.size();
        face_vertices_.resize(nb_vertices);
        for (std::size_t i = 0; i < nb_vertices; ++i)
            face_vertices_[i] = get(vertices[i]);

        // Check and resolve duplicate edges.
        for (std::size_t s = 0, t = 1; s < nb_vertices; ++s, ++t, t %= nb_vertices) {
            if (halfedge_has_duplication(vertices[s], vertices[t])) {
                if (!halfedge_is_legal(face_vertices_[s], face_vertices_[t])) {
                    // In each iteration, we check t only. The handling of the last edge (i.e., last_vertex -> first_vertex)
                    // may make of copy of the first vertex. This is OK because a new copy won't change the validity of the
                    // first edge.
                    face_vertices_[t] = copy_vertex(vertices[t]);
                }
            }
        }

        // let's check if the face can be linked to the mesh
        if (!can_link_face(face_vertices_)) {
            // If it failed, it must have complex neighboring topology.
            // We simply duplicate all its vertices (this should always work).
            for (std::size_t i = 0; i < nb_vertices; ++i) {
                if (mesh_->halfedge(face_vertices_[i]).is_valid()) // no need to copy isolated vertices.
                    face_vertices_[i] = copy_vertex(vertices[i]);
            }
        }

        // now let's add this face
        auto face = mesh_->add_face(face_vertices_);
        if (face.is_valid()) {
            // put the halfedges into our record (of the original vertex indices)
            for (std::size_t s = 0, t = 1; s < nb_vertices; ++s, ++t, t %= nb_vertices) {
                outgoing_halfedges_[vertices[s].idx()].push_back(vertices[t].idx());
            }
        } else {
            ++num_faces_unknown_topology_;
            LOG(ERROR) << "fatal error: failed adding an isolated face: " << vertices;
        }

        return face;
    }


    SurfaceMesh::Vertex ManifoldBuilder::get(SurfaceMesh::Vertex v) {
        auto pos = copied_vertices_.find(v.idx());
        if (pos == copied_vertices_.end()) { // no copies
            if (mesh_->is_boundary(v))
                return v;
        } else { // has copies
            const auto &copies = pos->second;
            for (auto c : copies) {
                if (mesh_->is_boundary(c))
                    return c;
            }
        }

        // if reached here, we have to make a copy
        return copy_vertex(v);
    }


    bool ManifoldBuilder::halfedge_is_legal(SurfaceMesh::Vertex s, SurfaceMesh::Vertex t) const {
        auto h = mesh_->find_halfedge(s, t);
        if (!h.is_valid())  // the edge does not exist
            return true;

        if (!mesh_->is_boundary(h) ||   // is a boundary (i.e., the face is NULL)
            !mesh_->is_boundary(s) || !mesh_->is_boundary(t)) // one of the vertices is a closed disk
        {
            return false;
        }

        return true;
    }


    SurfaceMesh::Vertex ManifoldBuilder::copy_vertex(SurfaceMesh::Vertex v) {
        auto points = mesh_->vertex_property<vec3>("v:point");

        // const vec3 p = points[v]; // [Liangliang]: 'const vec3&' won't work because the vector is growing.
        auto new_v = mesh_->add_vertex(points[v]);

        original_vertex_[new_v] = v;
        copied_vertices_[v.idx()].push_back(new_v);

        // copy all vertex properties except "v:connectivity" and "v:deleted"
        auto &arrays = mesh_->vprops_.arrays();
        for (auto &a : arrays) {
            if (a->name() == "v:connectivity" || a->name() == "v:deleted")
                continue;
            a->copy(v.idx(), new_v.idx());
        }

        return new_v;
    }


    bool ManifoldBuilder::can_link_face(const std::vector<SurfaceMesh::Vertex> &vertices) const {
        const std::size_t n(vertices.size());

        // use global arrays to avoid new/delete of local arrays!!!
        std::vector<SurfaceMesh::Halfedge> halfedges(n);
        std::vector<bool> is_new(n);

        // test for topological errors
        for (std::size_t i = 0, ii = 1; i < n; ++i, ++ii, ii %= n) {
            halfedges[i] = mesh_->find_halfedge(vertices[i], vertices[ii]);
            is_new[i] = !halfedges[i].is_valid();
        }

        SurfaceMesh::Halfedge inner_next, inner_prev, outer_prev, boundary_next, boundary_prev;

        // re-link patches if necessary
        for (std::size_t i = 0, ii = 1; i < n; ++i, ++ii, ii %= n) {
            if (!is_new[i] && !is_new[ii]) {
                inner_prev = halfedges[i];
                inner_next = halfedges[ii];

                if (mesh_->next_halfedge(inner_prev) != inner_next) {
                    // here comes the ugly part... we have to relink a whole patch

                    // search a free gap
                    // free gap will be between boundary_prev and boundary_next
                    outer_prev = mesh_->opposite_halfedge(inner_next);
                    boundary_prev = outer_prev;
                    do {
                        boundary_prev = mesh_->opposite_halfedge(mesh_->next_halfedge(boundary_prev));
                    } while (!mesh_->is_boundary(boundary_prev) || boundary_prev == inner_prev);
                    boundary_next = mesh_->next_halfedge(boundary_prev);
                    assert(mesh_->is_boundary(boundary_prev));
                    assert(mesh_->is_boundary(boundary_next));


                    // ok ?
                    if (boundary_next == inner_next)
                        return false;
                }
            }
        }

        return true;
    }

}