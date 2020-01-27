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

        original_vertex_ = mesh_->vertex_property<SurfaceMesh::Vertex>("v:ManifoldBuilder:original_vertex");
    }


    void ManifoldBuilder::end() {
        const std::string name = mesh_->name().empty() ? "(with unknown name)" : mesh_->name();
        std::string msg = "mesh " + name + "\n\tTopological issues detected:";
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
            if (!mesh_->is_boundary(v.first))
                ++count_non_manifold_vertices;
        }
        // resolve non-manifold vertices in the current mesh
        count_non_manifold_vertices += resolve_non_manifold_vertices(mesh_);
        if (count_non_manifold_vertices > 0) {
            msg += "\n\t\t" + std::to_string(count_non_manifold_vertices) + " non-manifold vertices (fixed)";

            std::size_t count = resolve_non_manifold_vertices(mesh_);
            LOG_IF(ERROR, count != 0) << count << " more non-manifold vertices resolved in another round";
            report = true;
        }

        // ----------------------------------------------------------------------------------

        std::size_t count_non_manifold_edges(0);
        for (const auto &targets : outgoing_halfedges_) {
            std::set<int> tmp(targets.begin(), targets.end());
            count_non_manifold_edges += (targets.size() - tmp.size());
        }
        if (count_non_manifold_edges > 0) {
            msg += "\n\t\t" + std::to_string(count_non_manifold_edges) + " non-manifold edges (fixed)";
            report = true;
        }

        // ----------------------------------------------------------------------------------

#if 0
        // ATTENTION: this record is valid only before deleting the isolated vertices.
        for (auto g : copied_vertices_) {
            LOG(INFO) << "\tvertex " << g.first << " copied to " << g.second;
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

        if (!copied_vertices_.empty() || count_isolated_vertices > 0) {
            msg += "\n\tSolution:";
            if (!copied_vertices_.empty()) {
                std::size_t count(0);
                for (auto copies : copied_vertices_)
                    count += copies.second.size();
                msg += "\n\t\tcopied " + std::to_string(copied_vertices_.size()) + " vertices (" +
                       std::to_string(count) +
                       " occurrences) to ensure manifoldness";
            }
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

        // ----------------------------------------------------------------------------------

        count_non_manifold_vertices = 0;
        for (auto v : mesh_->vertices()) {
            LOG_IF(ERROR, !mesh_->is_valid(v)) << "vertex " << v << " is not valid";
            if (!mesh_->is_manifold(v))
                ++count_non_manifold_vertices;
        }
        LOG_IF(ERROR, count_non_manifold_vertices > 0)
                        << "failed to resolve " << count_non_manifold_vertices << " non-manifold vertices";

        for (auto f : mesh_->faces())
            LOG_IF(ERROR, !mesh_->is_valid(f)) << "face " << f << " is not valid";
        for (auto e : mesh_->edges())
            LOG_IF(ERROR, !mesh_->is_valid(e)) << "edge " << e << " is not valid";
        for (auto h : mesh_->halfedges())
            LOG_IF(ERROR, !mesh_->is_valid(h)) << "halfedge " << h << " is not valid";

        LOG_IF(WARNING, report) << msg;

        outgoing_halfedges_.clear();
        mesh_->remove_vertex_property(original_vertex_);
    }


    SurfaceMesh::Vertex ManifoldBuilder::add_vertex(const vec3 &p) {
        LOG_IF(ERROR, mesh_->faces_size() > 0) << "vertices should be added before adding any face";
        SurfaceMesh::Vertex v = mesh_->add_vertex(p);
        original_vertex_[v] = v;
        return v;
    }


    bool ManifoldBuilder::vertices_valid(const std::vector<SurfaceMesh::Vertex> &vertices) {
        std::size_t n = vertices.size();

        // Check #1: a face has less than 3 vertices
        if (n < 3) {
            ++num_faces_less_three_vertices_;
            return false;
        }

        // Check #2; a face has duplicated vertices
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
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


//    bool ManifoldBuilder::halfedge_has_duplication(easy3d::SurfaceMesh::Vertex s, easy3d::SurfaceMesh::Vertex t) const {
//        const auto &targets = outgoing_halfedges_[s.idx()];
//        for (auto v : targets)
//            if (v == t.idx())
//                return true;
//        return false;
//    }


    SurfaceMesh::Face ManifoldBuilder::add_face(const std::vector<SurfaceMesh::Vertex> &vertices) {
        if (mesh_->faces_size() == 0) // the first face
            outgoing_halfedges_.resize(mesh_->vertices_size());

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
                    if (boundary_next == inner_next)
                        face_vertices_[t] = copy_vertex(vertices[t]);
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
            LOG_FIRST_N(ERROR, 1) << "fatal error: failed adding face (" << vertices << ") (logged only first record)";
        }

        return face;
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
        copied_vertices_[v].insert(new_v);

        // copy all vertex properties except "v:connectivity" and "v:deleted"
        auto &arrays = mesh_->vprops_.arrays();
        for (auto &a : arrays) {
            if (a->name() == "v:connectivity" || a->name() == "v:deleted")
                continue;
            a->copy(v.idx(), new_v.idx());
        }

        return new_v;
    }


    std::size_t ManifoldBuilder::resolve_non_manifold_vertices(SurfaceMesh* mesh)
    {
        auto null_h = SurfaceMesh::Halfedge();

        auto known_nm_vertices = mesh->add_vertex_property<bool>("v:ManifoldBuilder:known_nm_vertices", false);
        auto visited_vertices = mesh->add_vertex_property<SurfaceMesh::Halfedge>("v:ManifoldBuilder:visited_vertices", null_h);
        auto visited_halfedges = mesh->add_halfedge_property<bool>("h:ManifoldBuilder:visited_vertices", false);

        std::vector<SurfaceMesh::Halfedge> non_manifold_cones;

        for( auto h : mesh->halfedges())
        {
            // If 'h' is not visited yet, we walk around the target of 'h' and mark these
            // halfedges as visited. Thus, if we are here and the target is already marked as visited,
            // it means that the vertex is non manifold.
            if(! visited_halfedges[h])
            {
                visited_halfedges[h] = true;
                bool is_non_manifold = false;

                auto v = mesh->to_vertex(h);
                if(visited_vertices[v] != null_h) // already seen this vertex, but not from this star
                {
                    is_non_manifold = true;
                    // if this is the second time we visit that vertex and the first star was manifold, we have
                    // never reported the first star, but we must now
                    if(!known_nm_vertices[v]) {
                        non_manifold_cones.push_back(
                                visited_vertices[v]); // that's a halfedge of the first star we've seen 'v' in

                    }
                }
                else
                {
                    // first time we meet this vertex, just mark it so, and keep the halfedge we found the vertex with in memory
                    visited_vertices[v] = h;
                }

                // While walking the star of this halfedge, if we meet a border halfedge more than once,
                // it means the mesh is pinched and we are also in the case of a non-manifold situation
                auto ih = h, done = ih;
                int border_counter = 0;
                do
                {
                    visited_halfedges[ih] = true;
                    if(mesh->is_boundary(ih))
                        ++border_counter;

                    ih = mesh->prev_halfedge(mesh->opposite_halfedge(ih));
                }
                while(ih != done);

                if(border_counter > 1)
                    is_non_manifold = true;

                if(is_non_manifold) {
                    non_manifold_cones.push_back(h);
                    known_nm_vertices[v] = true;
                }
            }
        }

        for (auto h : non_manifold_cones) {
            //std::cout << "vertex " << mesh->to_vertex(h) << " is non-manifold" << std::endl;
            std::size_t nb_new_vertices = resolve_non_manifold_vertex(h, mesh);
            std::cout << "vertex " << mesh_->to_vertex(h) << ": num vertices created: " << nb_new_vertices << std::endl;
        }

        mesh->remove_vertex_property(known_nm_vertices);
        mesh->remove_vertex_property(visited_vertices);
        mesh->remove_halfedge_property(visited_halfedges);

        return non_manifold_cones.size();
    }


    std::size_t ManifoldBuilder::resolve_non_manifold_vertex(SurfaceMesh::Halfedge h, SurfaceMesh* mesh)
    {
        typedef std::map<SurfaceMesh::Vertex, std::vector<SurfaceMesh::Vertex> > CopyRecord;

        auto has_vertex = [](SurfaceMesh::Vertex v, CopyRecord& record) ->bool {
            return record.find(v) != record.end();
        };
        auto collect_vertices = [](SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, CopyRecord& record) -> void {
            auto& verts = record[v1];
            if(verts.empty())
                verts.push_back(v1);
            verts.push_back(v2);
        };

        auto create_new_vertex_for_sector = [this](SurfaceMesh::Halfedge sector_begin_h, SurfaceMesh::Halfedge sector_last_h, SurfaceMesh* mesh) -> SurfaceMesh::Vertex {
            auto old_vd = mesh->to_vertex(sector_begin_h);

            auto old_vd_org = original_vertex_[old_vd];
            auto new_vd = copy_vertex(old_vd_org);

            mesh->set_halfedge(new_vd, sector_begin_h);
            auto h = sector_begin_h;
            do {
                mesh->set_vertex(h, new_vd);
                if(h == sector_last_h)
                    break;
                else
                    h = mesh->prev_halfedge(mesh->opposite_halfedge(h));
            }
            while(h != sector_begin_h); // for safety
            DLOG_ASSERT(h != sector_begin_h);
            return new_vd;
        };

        CopyRecord dmap;
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
            const bool first_time_meeting_v = !has_vertex(old_v, dmap);
            if (first_time_meeting_v) {
                // The star is manifold, so if it is the first time we have met that vertex,
                // there is nothing to do, we just keep the same vertex.
                mesh->set_halfedge(old_v, h); // to ensure halfedge(old_v, pm) stays valid
                dmap[old_v]; // so that we know we have met old_v already, next time, we'll have to duplicate
            } else {
                // This is not the canonical star associated to 'v'.
                // Create a new vertex, and move the whole star to that new vertex
                auto last_h = mesh->opposite_halfedge(mesh->next_halfedge(h));
                auto new_v = create_new_vertex_for_sector(h, last_h, mesh);
                collect_vertices(old_v, new_v, dmap);
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
                bool must_create_new_vertex = (!is_main_sector || has_vertex(old_v, dmap));

                // In any case, we must set up the next pointer correctly
                mesh->set_next_halfedge(sector_start_h, mesh->opposite_halfedge(sector_last_h));

                if (must_create_new_vertex) {
                    SurfaceMesh::Vertex new_v = create_new_vertex_for_sector(sector_start_h, sector_last_h, mesh);
                    collect_vertices(old_v, new_v, dmap);
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
