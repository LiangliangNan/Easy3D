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
        LOG_IF(ERROR, original_vertex_) << "missing call to end_surface(), which must be in pair with begin_surface()";
    }


    void ManifoldBuilder::begin_surface() {
        num_faces_less_three_vertices_ = 0;
        num_faces_duplicated_vertices_ = 0;
        num_faces_out_of_range_vertices_ = 0;
        num_faces_unknown_topology_ = 0;

        face_vertices_.clear();
        copied_vertices_.clear();
        copied_vertices_for_linking_.clear();
        outgoing_halfedges_.clear();

        original_vertex_ = mesh_->add_vertex_property<SurfaceMesh::Vertex>("v:ManifoldBuilder:original_vertex");
    }


    void ManifoldBuilder::end_surface(bool log_issues) {
        // ----------------------------------------------------------------------------------

        // Step 1: Resolve non-manifold vertices (also collect data for the report).
        //
        // Vertices might be copied, for two reasons:
        //  - resolve non-manifoldness. In two phases: during the construction of the mesh by call to 'add_face()' and
        //    in 'resolve_non_manifold_vertices()'.
        //  - ensure boundary consistency. All happen during the construction of the mesh by call to 'add_face()'.

        // Resolve non-manifold vertices.
        resolve_non_manifold_vertices(mesh_);
        // Release memory immediately when not needed any more.
        mesh_->remove_vertex_property(original_vertex_);

        // now all copy occurrences are known
        // add a vertex property "v:lock" recording all copied vertices
        auto lock = mesh_->add_vertex_property<bool>("v:lock");
        std::size_t num_non_manifold_vertices = copied_vertices_.size();
        std::size_t num_copy_occurrences(0);
        for (const auto& copies : copied_vertices_) {
            LOG_IF(FATAL, copies.second.empty()) << "vertex " << copies.first << " not actually copied";
            num_copy_occurrences += copies.second.size();
            for (auto v : copies.second)
                lock[v] = true;
        }
        // Release memory immediately when not needed any more.
        copied_vertices_.clear();

        // Query the number of non-manifold edges.
        std::size_t num_non_manifold_edges(0);
        for (const auto &targets : outgoing_halfedges_) {
            std::set<int> tmp(targets.begin(), targets.end());
            num_non_manifold_edges += (targets.size() - tmp.size());
        }

        // Release memory immediately when not needed any more.
        outgoing_halfedges_.clear();

        // ----------------------------------------------------------------------------------

        // Step 2: remove isolated vertices
        std::size_t num_isolated_vertices(0);
        for (auto v : mesh_->vertices()) {
            if (mesh_->is_isolated(v)) {
                mesh_->delete_vertex(v);
                ++num_isolated_vertices;
            }
        }
        if (num_isolated_vertices > 0)
            mesh_->garbage_collection();

        // ---------------------------------------------------------------------------------

        // Let me do some more checks

        // Check if the mesh is valid
        for (auto f : mesh_->faces())
            LOG_IF(ERROR, !mesh_->is_valid(f)) << "face " << f << " is not valid";
        for (auto e : mesh_->edges())
            LOG_IF(ERROR, !mesh_->is_valid(e)) << "edge " << e << " is not valid";
        for (auto h : mesh_->halfedges())
            LOG_IF(ERROR, !mesh_->is_valid(h)) << "halfedge " << h << " is not valid";

        // Check if there are still non-manifold vertices
        std::size_t count(0);
        for (auto v : mesh_->vertices()) {
            if (!mesh_->is_manifold(v)) {
                LOG_FIRST_N(ERROR, 1) << "vertex " << v << " is not manifold (this is the first record)";
                ++count;
            }
        }
        LOG_IF(ERROR, count > 0) << "mesh still have " << count << " non-manifold vertices";

        // ----------------------------------------------------------------------------------

        if (!log_issues)
            return;

        // Prepare a brief report on the construction of the mesh.

        std::string issues("");
        if (num_faces_less_three_vertices_ > 0) {
            issues += "\n\t\t" + std::to_string(num_faces_less_three_vertices_) +
                      " faces with less than 3 vertices (ignored)";
        }

        if (num_faces_duplicated_vertices_ > 0) {
            issues += "\n\t\t" + std::to_string(num_faces_duplicated_vertices_) +
                      " faces with duplicated vertices (ignored)";
        }

        if (num_faces_out_of_range_vertices_ > 0) {
            issues += "\n\t\t" + std::to_string(num_faces_out_of_range_vertices_) +
                      " faces with out-of-range vertices (ignored)";
        }

        if (num_faces_unknown_topology_ > 0) {
            issues += "\n\t\t" + std::to_string(num_faces_unknown_topology_) +
                      " complex faces with unknown topology (ignored)";
        }

        // ----------------------------------------------------------------------------------

        if (num_non_manifold_vertices > 0) {
            issues += "\n\t\t" + std::to_string(num_non_manifold_vertices) + " non-manifold vertices (fixed)";
        }

        if (num_non_manifold_edges > 0) {
            issues += "\n\t\t" + std::to_string(num_non_manifold_edges) + " non-manifold edges (fixed)";
        }

        // ----------------------------------------------------------------------------------

        if (num_isolated_vertices > 0) {
            issues += "\n\t\t" + std::to_string(num_isolated_vertices) + " isolated vertices (removed)";
        }

        // ----------------------------------------------------------------------------------

        if (num_copy_occurrences > 0 || num_isolated_vertices > 0) {
            issues += "\n\tSolution:";
            if (num_copy_occurrences > 0) {
                issues += "\n\t\t" + std::to_string(num_non_manifold_vertices) + " vertices copied ("
                          + std::to_string(num_copy_occurrences) + " occurrences) to ensure manifoldness";

                if (copied_vertices_for_linking_.size() > 0) {
                    std::size_t occurrences(0);
                    for (const auto &copies : copied_vertices_for_linking_) {
                        LOG_IF(FATAL, copies.second.empty()) << "vertex " << copies.first << " not actually copied";
                        occurrences += copies.second.size();
                    }
                    issues += " (among which " + std::to_string(copied_vertices_for_linking_.size()) + " vertices with "
                              + std::to_string(occurrences) + " occurrences are for linking new faces)";
                    copied_vertices_for_linking_.clear();
                }
            }
            if (num_isolated_vertices > 0)
                issues += "\n\t\t" + std::to_string(num_isolated_vertices) + " isolated vertices deleted";
        }

        // ----------------------------------------------------------------------------------

        const std::string header = "mesh \"" + mesh_->name() + "\"";
        if (!issues.empty()) {
            LOG(WARNING) << header << " has topological issues:" << issues
                         << "\n\tResult: \n\t\t"
                         << std::to_string(mesh_->n_faces()) + " faces\n\t\t"
                         << std::to_string(mesh_->n_vertices()) + " vertices\n\t\t"
                         << std::to_string(mesh_->n_edges()) + " edges";
        }
    }


    SurfaceMesh::Vertex ManifoldBuilder::add_vertex(const vec3 &p) {
        DLOG_IF(ERROR, !original_vertex_) << "you must call begin_surface() before the construction";
        DLOG_IF(ERROR, mesh_->n_faces() > 0) << "vertices should be added before adding faces";
        SurfaceMesh::Vertex v = mesh_->add_vertex(p);
        original_vertex_[v] = v;
        return v;
    }


    bool ManifoldBuilder::vertices_valid(const std::vector<SurfaceMesh::Vertex> &vertices) {
        const std::size_t n = vertices.size();

        // Check #1: a face has less than 3 vertices
        if (n < 3) {
            LOG_FIRST_N(ERROR, 1) << "face has less than 3 vertices: " << vertices << " (this is the first record)";
            ++num_faces_less_three_vertices_;
            return false;
        }

        // Check #2; a face has duplicated vertices
        for (std::size_t s = 0, t = 1; s < n; ++s, ++t, t %= n) {
            if (vertices[s] == vertices[t]) {
                LOG_FIRST_N(ERROR, 1) << "face has duplicated vertices: " << vertices
                                      << " (this is the first record)";
                ++num_faces_duplicated_vertices_;
                return false;
            }
        }

        // Check #3; a face has out-of-range vertices
        for (auto v : vertices) {
            if (v.idx() < 0 || v.idx() >= static_cast<int>(mesh_->n_vertices())) {
                LOG_FIRST_N(ERROR, 1) << "face has out-of-range vertices: " << vertices << " (number of vertices is "
                                      << mesh_->n_vertices() << ") (this is the first record)";
                ++num_faces_out_of_range_vertices_;
                return false;
            }
        }

        // More check?
        // A face has already been added, i.e., a previously added face has the same vertex indices.
        // We should allow this by duplicating its vertices (to avoid discarding faces).

        return true;
    }


//    bool ManifoldBuilder::halfedge_has_duplication(easy3d::SurfaceMesh::Vertex s, easy3d::SurfaceMesh::Vertex t) const {
//        const auto &targets = outgoing_halfedges_[s.idx()];
//        for (auto v : targets)
//            if (v == t.idx())
//                return true;
//        return false;
//    }


    SurfaceMesh::Face ManifoldBuilder::add_face(const std::vector<SurfaceMesh::Vertex> &vertices) {
        DLOG_IF(ERROR, mesh_->n_vertices() == 0)
                        << "you must add vertices by calling add_vertex() before adding a face";
        if (mesh_->n_faces() == 0) // the first face
            outgoing_halfedges_.resize(mesh_->n_vertices());

        if (!vertices_valid(vertices))
            return SurfaceMesh::Face();

        std::size_t n = vertices.size();
        face_vertices_.resize(n);

        // try to use the newly copied vertices first to avoid unnecessary copies.
        for (std::size_t i = 0; i < n; ++i)
            face_vertices_[i] = get(vertices[i]);

        // ---------------------------------------------------------------------------------------------------------

        std::vector<SurfaceMesh::Halfedge> halfedges(n);
        std::vector<char> halfedge_esists(n);

        // Check and resolve duplicate edges.

        // For each edge, we check the 'to' vertex only. The handling of the last edge (i.e., last_vertex -> first_vertex)
        // may make of copy of the first vertex. This is OK because a new copy won't change the validity of the first edge.
        for (std::size_t s = 0, t = 1; s < n; ++s, ++t, t %= n) {
            auto h = mesh_->find_halfedge(face_vertices_[s], face_vertices_[t]);
            if (h.is_valid() && !mesh_->is_boundary(h)) {
                face_vertices_[t] = copy_vertex(vertices[t]);
                h = mesh_->find_halfedge(face_vertices_[s], face_vertices_[t]);
            }

            halfedges[s] = h;
            halfedge_esists[s] = h.is_valid();
        }

        // ---------------------------------------------------------------------------------------------------------

        // Check and resolve linking issue.

        // Let's check if the face can be linked to the mesh
        SurfaceMesh::Halfedge inner_next, inner_prev, outer_prev, boundary_next, boundary_prev;
        for (std::size_t s = 0, t = 1; s < n; ++s, ++t, t %= n) {
            if (halfedge_esists[s] && halfedge_esists[t]) {
                inner_prev = halfedges[s];
                inner_next = halfedges[t];
                if (mesh_->next_halfedge(inner_prev) != inner_next) {
                    // search a free gap
                    // free gap will be between boundary_prev and boundary_next
                    outer_prev = mesh_->opposite_halfedge(inner_next);
                    boundary_prev = outer_prev;
                    do {
                        boundary_prev = mesh_->opposite_halfedge(mesh_->next_halfedge(boundary_prev));
                    } while (!mesh_->is_boundary(boundary_prev) || boundary_prev == inner_prev);
                    boundary_next = mesh_->next_halfedge(boundary_prev);
                    DLOG_IF(FATAL, !mesh_->is_boundary(boundary_prev));
                    DLOG_IF(FATAL, !mesh_->is_boundary(boundary_next));
                    if (boundary_next == inner_next) {
                        face_vertices_[t] = copy_vertex(vertices[t]);

                        // keep a record that this copy if for linking a face to the mesh. This is just for the report.
                        copied_vertices_for_linking_[vertices[t]].push_back(face_vertices_[t]);
                    }
                }
            }
        }

        // ---------------------------------------------------------------------------------------------------------

        // Now we should be able to link the new face to the current mesh.
        auto face = mesh_->add_face(face_vertices_);
        if (face.is_valid()) {
            // put the halfedges into our record (of the original vertex indices)
            for (std::size_t s = 0, t = 1; s < n; ++s, ++t, t %= n) {
                outgoing_halfedges_[vertices[s].idx()].push_back(vertices[t].idx());
            }
        } else {
            ++num_faces_unknown_topology_;
            LOG_FIRST_N(ERROR, 1) << "fatal error: failed adding face (" << vertices << ") (this is the first record)";
        }

        return face;
    }


    SurfaceMesh::Face
    ManifoldBuilder::add_triangle(SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, SurfaceMesh::Vertex v3) {
        return add_face({v1, v2, v3});
    }


    SurfaceMesh::Face ManifoldBuilder::add_quad(SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, SurfaceMesh::Vertex v3,
                                                SurfaceMesh::Vertex v4) {
        return add_face({v1, v2, v3, v4});
    }


    SurfaceMesh::Vertex ManifoldBuilder::get(SurfaceMesh::Vertex v) {
        auto pos = copied_vertices_.find(v);
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


    SurfaceMesh::Vertex ManifoldBuilder::copy_vertex(SurfaceMesh::Vertex v) {
        auto points = mesh_->vertex_property<vec3>("v:point");

        const vec3 p = points[v]; // [Liangliang]: 'const vec3&' won't work because the vector is growing.
        auto new_v = mesh_->add_vertex(p);
        original_vertex_[new_v] = v;
        copied_vertices_[v].push_back(new_v);

        // copy all vertex properties except "v:connectivity" and "v:deleted"
        auto &arrays = mesh_->vprops_.arrays();
        for (auto &a : arrays) {
            if (a->name() == "v:connectivity" || a->name() == "v:deleted")
                continue;
            a->copy(v.idx(), new_v.idx());
        }

        return new_v;
    }


    std::size_t ManifoldBuilder::resolve_non_manifold_vertices(SurfaceMesh *mesh) {
        // We have two types of non-manifold vertices:
        //  - type 1: Vertices touching closed disks.
        //
        //  - type 2: Vertices shared by multiple umbrellas. This type of non-manifold vertices have not been resolved
        //            yet. We will have to resolve them by calling to resolve_non_manifold_vertices().

        auto null_h = SurfaceMesh::Halfedge();

        auto known_nm_vertices = mesh->add_vertex_property<bool>("v:ManifoldBuilder:known_nm_vertices", false);
        auto visited_vertices = mesh->add_vertex_property<SurfaceMesh::Halfedge>("v:ManifoldBuilder:visited_vertices",
                                                                                 null_h);
        auto visited_halfedges = mesh->add_halfedge_property<bool>("h:ManifoldBuilder:visited_halfedges", false);


        // keep a record that the vertex copies are occurred in the 'resolve_non_manifold_vertices()' phase.
        // NOTE: not possible to reuse 'copied_vertices_', because this phase requires a clean record but some vertices
        //       might have already been copied in the previous phase (i.e., in add_face()).
        CopyRecord copy_record;

        std::vector<SurfaceMesh::Halfedge> non_manifold_cones;
        for (auto h : mesh->halfedges()) {
            // If 'h' is not visited yet, we walk around the target of 'h' and mark these
            // halfedges as visited. Thus, if we are here and the target is already marked as visited,
            // it means that the vertex is non manifold.
            if (!visited_halfedges[h]) {
                visited_halfedges[h] = true;
                bool is_non_manifold = false;

                auto v = mesh->to_vertex(h);
                if (visited_vertices[v] != null_h) // already seen this vertex, but not from this star
                {
                    is_non_manifold = true;
                    // if this is the second time we visit that vertex and the first star was manifold, we have
                    // never reported the first star, but we must now
                    if (!known_nm_vertices[v]) {
                        // that's a halfedge of the first star we've seen 'v' in
                        non_manifold_cones.push_back(visited_vertices[v]);
                    }
                } else {
                    // first time we meet this vertex, just mark it so, and keep the halfedge we found the vertex with in memory
                    visited_vertices[v] = h;
                }

                // While walking the star of this halfedge, if we meet a border halfedge more than once,
                // it means the mesh is pinched and we are also in the case of a non-manifold situation
                auto ih = h, done = ih;
                int border_counter = 0;
                do {
                    visited_halfedges[ih] = true;
                    if (mesh->is_boundary(ih))
                        ++border_counter;

                    ih = mesh->prev_halfedge(mesh->opposite_halfedge(ih));
                } while (ih != done);

                if (border_counter > 1)
                    is_non_manifold = true;

                if (is_non_manifold) {
                    non_manifold_cones.push_back(h);
                    known_nm_vertices[v] = true;
                }
            }
        }

        // for each umbrella
        for (auto h : non_manifold_cones)
            resolve_non_manifold_vertex(h, mesh, copy_record);

#if 0    // This is the history how vertices were duplicated.
        for (const auto& copy : dmap) {
            LOG(INFO) << "Non-manifold vertex " << copy.first << " was fixed by creating";
            for (auto v : copy.second)
                std::cout << " " << v;
            std::cout << std::endl;
        }
#endif

        mesh->remove_vertex_property(known_nm_vertices);
        mesh->remove_vertex_property(visited_vertices);
        mesh->remove_halfedge_property(visited_halfedges);

        return copy_record.size();
    }


    std::size_t
    ManifoldBuilder::resolve_non_manifold_vertex(SurfaceMesh::Halfedge h, SurfaceMesh *mesh, CopyRecord &copy_record) {
        auto create_new_vertex_for_sector = [this](SurfaceMesh::Halfedge sector_begin_h,
                                                   SurfaceMesh::Halfedge sector_last_h,
                                                   SurfaceMesh *mesh) -> SurfaceMesh::Vertex {
            auto old_v = mesh->to_vertex(sector_begin_h);

            auto old_v_original = original_vertex_[old_v];
            auto new_v = copy_vertex(old_v_original);

            mesh->set_halfedge(new_v, sector_begin_h);
            auto h = sector_begin_h;
            do {
                mesh->set_vertex(h, new_v);
                if (h == sector_last_h)
                    break;
                else
                    h = mesh->prev_halfedge(mesh->opposite_halfedge(h));
            } while (h != sector_begin_h); // for safety
            DLOG_ASSERT(h != sector_begin_h);
            return new_v;
        };

        std::size_t nb_new_vertices = 0;
        auto old_v = mesh_->to_vertex(h);

        // count the number of borders
        int border_counter = 0;
        auto ih = h, done = ih, border_h = h;
        do {
            if (mesh->is_boundary(ih)) {
                border_h = ih;
                ++border_counter;
            }

            ih = mesh->prev_halfedge(mesh->opposite_halfedge(ih));
        } while (ih != done);

        bool is_non_manifold_within_umbrella = (border_counter > 1);
        if (!is_non_manifold_within_umbrella) {
            if (copy_record.find(old_v) == copy_record.end()) { // first time meeting the vertex
                // The star is manifold, so if it is the first time we have met that vertex,
                // there is nothing to do, we just keep the same vertex.
                mesh->set_halfedge(old_v, h); // to ensure halfedge(old_v, pm) stays valid
                copy_record[old_v]; // so that we know we have met old_v already, next time, we'll have to duplicate
            } else {
                // This is not the canonical star associated to 'v'.
                // Create a new vertex, and move the whole star to that new vertex
                auto last_h = mesh->opposite_halfedge(mesh->next_halfedge(h));
                auto new_v = create_new_vertex_for_sector(h, last_h, mesh);
                copy_record[old_v].push_back(new_v);
                nb_new_vertices = 1;
            }
        }
            // if there is more than one sector, look at each sector and split them away from the main one
        else {
            // the first manifold sector, described by two halfedges
            auto sector_start_h = border_h;
            DLOG_ASSERT(mesh->is_boundary(border_h));

            bool should_stop = false;
            bool is_main_sector = true;
            do {
                DLOG_ASSERT(mesh->is_boundary(sector_start_h));

                // collect the sector and split it away if it must be
                auto sector_last_h = sector_start_h;
                do {
                    auto next_h = mesh->prev_halfedge(mesh->opposite_halfedge(sector_last_h));
                    if (mesh->is_boundary(next_h))
                        break;

                    sector_last_h = next_h;
                } while (sector_last_h != sector_start_h);
                DLOG_ASSERT(!mesh->is_boundary(sector_last_h));
                DLOG_ASSERT(sector_last_h != sector_start_h);

                auto next_start_h = mesh->prev_halfedge(mesh->opposite_halfedge(sector_last_h));

                // there are multiple CCs incident to this particular vertex, and we should create a new vertex
                // if it's not the first umbrella around 'old_v' or not the first sector, but only not if it's
                // both the first umbrella and first sector.
                bool must_create_new_vertex = (!is_main_sector || copy_record.find(old_v) != copy_record.end());

                // In any case, we must set up the next pointer correctly
                mesh->set_next_halfedge(sector_start_h, mesh->opposite_halfedge(sector_last_h));

                if (must_create_new_vertex) {
                    auto new_v = create_new_vertex_for_sector(sector_start_h, sector_last_h, mesh);
                    copy_record[old_v].push_back(new_v);
                    ++nb_new_vertices;
                } else {
                    // Ensure that halfedge(old_v, pm) stays valid
                    mesh->set_halfedge(old_v, sector_start_h);
                }

                is_main_sector = false;
                sector_start_h = next_start_h;
                should_stop = (sector_start_h == border_h);
            } while (!should_stop);
        }

        return nb_new_vertices;
    }

}
