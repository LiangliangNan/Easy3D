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

#include <easy3d/core/manifold_guard.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    ManifoldGuard::ManifoldGuard(SurfaceMesh *mesh)
    : mesh_(mesh)
    {
    }


    ManifoldGuard::~ManifoldGuard() {

    }


    void ManifoldGuard::begin(const std::string &original_vertex_name) {
        num_faces_less_three_vertices_ = 0;
        num_faces_duplicated_vertices_ = 0;
        num_faces_unknown_structure_ = 0;

        num_non_manifold_vertices_= 0;
        num_isolated_vertices_= 0;

        face_original_vertices_.clear();
        face_vertices_.clear();

        copied_vertices_.clear();
        copied_edges_.clear();

        original_vertex_ = mesh_->add_vertex_property<SurfaceMesh::Vertex>(original_vertex_name);
    }


    void ManifoldGuard::finish(bool clean) {
        const std::string name = mesh_->name().empty() ? "with unknown name" : mesh_->name();
        std::string msg = "mesh \n\t" + name + "\n\thas topological issues:";
        bool report(false);

        for (auto v : mesh_->vertices()) {
            if (mesh_->is_isolated(v)) {
                mesh_->delete_vertex(v);
                ++num_isolated_vertices_;
            }
        }
        mesh_->garbage_collection();
        if (num_isolated_vertices_ > 0) {
            msg += "\n\t\t" + std::to_string(num_isolated_vertices_) + " isolated vertices (removed)";
            report = true;
        }

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
        if (!copied_edges_.empty()) {
            msg += "\n\t\t" + std::to_string(copied_edges_.size()) + " duplicated edges (fixed)";
            report = true;
        }

        for (auto v : mesh_->vertices()) {
            if (!mesh_->is_manifold(v))
                ++num_non_manifold_vertices_;
        }
        if (num_non_manifold_vertices_ > 0) {
            msg += "\n\t\t" + std::to_string(num_non_manifold_vertices_) + " non_manifold vertices (not fixed)";
            report = true;
        }

        if (num_faces_unknown_structure_ > 0) {
            msg += "\n\t\t" + std::to_string(num_faces_unknown_structure_) + " complex faces with unknown structure (ignored)";
            report = true;

            msg += "\nTODO: resolve non-manifold vertices. If still 'SurfaceMesh::add_face: patch re-linking failed'"
                   "\n\tthen check duplicated faces";
        }

		if (report) {
			msg += "\n\t#face: " + std::to_string(mesh_->faces_size())
				+ ", #vertex: " + std::to_string(mesh_->vertices_size())
				+ ", #edge: " + std::to_string(mesh_->edges_size());
		}

#if 0
        for (auto g : copied_vertices_) {
            msg += "\n\tvertex v" + std::to_string(g.first.idx()) + " copied to ";
            for (auto v : g.second)
                msg += "v" + std::to_string(v.idx()) + " ";
        }

        for (auto e : copied_edges_) {
            auto s = mesh_->from_vertex(e.first);
            auto t = mesh_->to_vertex(e.first);
            msg += "\n\tedge h" + std::to_string(e.first.idx()) + " (v" + std::to_string(s.idx()) + " -> v" + std::to_string(t.idx()) + ") copied to ";
            s = mesh_->from_vertex(e.second);
            t = mesh_->to_vertex(e.second);
            msg += "h" + std::to_string(e.second.idx()) + " (v" + std::to_string(s.idx()) + " -> v" + std::to_string(t.idx()) + ")";
        }
#endif

        for (auto v : mesh_->vertices())
            LOG_IF(ERROR, !mesh_->is_valid(v)) << "vertex " << v << " is not valid";
        for (auto f : mesh_->faces())
            LOG_IF(ERROR, !mesh_->is_valid(f)) << "face " << f << " is not valid";
        for (auto e : mesh_->edges())
            LOG_IF(ERROR, !mesh_->is_valid(e)) << "edge " << e << " is not valid";
        for (auto h : mesh_->halfedges())
            LOG_IF(ERROR, !mesh_->is_valid(h)) << "halfedge " << h << " is not valid";

        LOG_IF(WARNING, report) << msg;

        if (clean)
            mesh_->remove_vertex_property(original_vertex_);
    }


    SurfaceMesh::Vertex ManifoldGuard::add_vertex(const vec3 &p) {
        return mesh_->add_vertex(p);
    }


    bool ManifoldGuard::face_can_be_added(const std::vector<SurfaceMesh::Vertex> &vertices) {
        std::size_t nb_vertices = vertices.size();

        // Check #1: a face has less than 3 vertices
        if (nb_vertices < 3) {
            ++num_faces_less_three_vertices_;
            return false;
        }

        // Check #2; a face has duplicated vertices
        for (std::size_t i = 0; i < nb_vertices; ++i) {
            for (std::size_t j = i+1; j < nb_vertices; ++j) {
                if (vertices[i] == vertices[j]) {
                    ++num_faces_duplicated_vertices_;
                    return false;
                }
            }
        }

        // More check?
        // A face has already been added, i.e., a previously added face has the same vertex indices.
        // We should allow this by duplicating its vertices (to avoid discarding faces).

        return true;
    }


    bool ManifoldGuard::halfedge_has_duplication(easy3d::SurfaceMesh::Vertex s, easy3d::SurfaceMesh::Vertex t) const {
        auto h = mesh_->find_halfedge(s, t);
        if (h.is_valid() && !mesh_->is_boundary(h)) {
//            std::cout << "edge (" << s << " -> " << t << ") already exists"<< std::endl;
            return true;
        }

        // test the duplicated edges using EACH copy of s and t
        auto s_pos = copied_vertices_.find(s);
        if (s_pos != copied_vertices_.end()) {
            const auto& s_copies = s_pos->second;
            for (auto v : s_copies) {
                h = mesh_->find_halfedge(v, t);
                if (h.is_valid() && !mesh_->is_boundary(h)) {
//                    std::cout << "edge (" << s << " -> " << t << ") duplicates existing edge (" << v << " -> " << t << ")" << std::endl;
                    return true;
                }
            }
        }

        // test the duplicated edges using s and EACH copy of t
        auto t_pos = copied_vertices_.find(t);
        if (t_pos != copied_vertices_.end()) {
            const auto& t_copies = t_pos->second;
            for (auto v : t_copies) {
                h = mesh_->find_halfedge(s, v);
                if (h.is_valid() && !mesh_->is_boundary(h)) {
//                    std::cout << "edge (" << s << " -> " << t << ") duplicates existing edge (" << s << " -> " << v << ")" << std::endl;
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
//                            std::cout << "edge (" << s << " -> " << t << ") duplicates existing edge (" << vs << " -> " << vt << ")" << std::endl;
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }


    SurfaceMesh::Face ManifoldGuard::add_face(const std::vector<SurfaceMesh::Vertex> &vertices) {
        if (!face_can_be_added(vertices))
            return SurfaceMesh::Face();

//        std::cout << "\n\n----- add face: original " << vertices << " ---------" << std::endl;

        std::size_t nb_vertices = vertices.size();
        face_original_vertices_ = vertices;
        face_vertices_.resize(nb_vertices);
        for (int i = 0; i < nb_vertices; ++i)
            face_vertices_[i] = get(face_original_vertices_[i]);

        // Check and resolve duplicate edges.
        for (std::size_t s = 0; s < nb_vertices; ++s) {
            std::size_t t = ((s + 1) % nb_vertices);
            if (halfedge_has_duplication(face_original_vertices_[s], face_original_vertices_[t])) {
                if (!halfedge_is_legal(face_vertices_[s], face_vertices_[t])) {
                    // In each iteration, we check t only. The handling of the last edge (i.e., last_vertex -> first_vertex)
                    // may make of copy of the first vertex. This is OK because a new copy won't change the validity of the
                    // first edge.
                    face_vertices_[t] = copy_vertex(face_original_vertices_[t]);
                }

                copied_edges_.push_back(std::make_pair(
                        Edge(face_original_vertices_[s], face_original_vertices_[t]),
                        Edge(face_vertices_[s], face_vertices_[t])
                ));
            }
        }

#define MANIFOLD_ON_THE_FLY 0
#if MANIFOLD_ON_THE_FLY
        // Now we don't have a complex edge. Check if adding the face results in non-manifold vertex.
        // This enforce the mesh is manifold after adding EVERY face. It has two limitations:
        //  - Sensitive to face orders;
        //  - It cannot report the actual number of non-manifold vertices (non-manifold vertices are always fixed
        //    before adding a face).
        // TODO: A better idea is to resolve non-manifold vertices after the whole mesh is constructed.
        for (std::size_t cur = 0; cur < nb_vertices; cur++) {
            std::size_t prev = ((cur + nb_vertices - 1) % nb_vertices);
            std::size_t next = ((cur + 1) % nb_vertices);

            // a non-manifold vertex may occur if:
            //  - cur is not isolated and is on boundary.
            //  - the two new edges both not exist.
            if ((mesh_->halfedge(face_vertices_[cur]).is_valid()) &&  // not isolated
                (mesh_->is_boundary(face_vertices_[cur])) &&          // on the boundary
                (!mesh_->find_halfedge(face_vertices_[prev], face_vertices_[cur]).is_valid()) &&
                (!mesh_->find_halfedge(face_vertices_[cur], face_vertices_[next]).is_valid()))
            {
                //std::cout << "prev -> cur -> next: " << prev << " -> " << cur << " -> " << next << std::endl;
                face_vertices_[cur] = copy_vertex(face_original_vertices_[cur]);
            }
        }
#endif

        auto face = mesh_->add_face(face_vertices_);
        if (face.is_valid()) {
//            std::cout << "      GOOD: added face "<< face_vertices_ <<  std::endl;
        }
        else {
            ++num_faces_unknown_structure_;
//            std::cout << "      ERROR: add face failed "<< face_vertices_ <<  std::endl;
            LOG(ERROR) << "always add the face by duplicating all it vertices";
        }

        return face;
    }



    SurfaceMesh::Vertex ManifoldGuard::get(SurfaceMesh::Vertex v) {
        if (copied_vertices_.find(v) == copied_vertices_.end()) { // no copies
            if (mesh_->is_boundary(v))
                return v;
        }
        else { // has copies
            const auto& copies = copied_vertices_[v];
            for (auto c : copies) {
                if (mesh_->is_boundary(c))
                    return c;
            }
        }

        // if reached here, we have to make a copy
        return copy_vertex(v);
    }


    bool ManifoldGuard::halfedge_is_legal(SurfaceMesh::Vertex s, SurfaceMesh::Vertex t) const {
        auto h = mesh_->find_halfedge(s, t);

        // the edge does not exist or it is a boundary (i.e., the face is NULL)
        if (h.is_valid() && !mesh_->is_boundary(h))
            return false;

        // one of the vertices is a closed disk
        if (!mesh_->is_boundary(s) || !mesh_->is_boundary(t))
            return false;

        return true;
    }


    SurfaceMesh::Vertex ManifoldGuard::copy_vertex(SurfaceMesh::Vertex v) {
        auto points = mesh_->vertex_property<vec3>("v:point");

		// const vec3 p = points[v]; // [Liangliang]: 'const vec3&' won't work because the vector is growing.
        auto new_v = mesh_->add_vertex(points[v]);

        original_vertex_[new_v] = v;
        copied_vertices_[v].push_back(new_v);

        // copy all vertex properties except "v:connectivity" and "v:deleted"
        auto &arrays = mesh_->vprops_.arrays();
        for (auto &a : arrays) {
            if (a->name() == "v:connectivity" || a->name() == "v:deleted")
                continue;
            a->copy(v.idx(), new_v.idx());
        }

//        std::cout << "vertex " << v << " copied to " << new_v << std::endl;

		return new_v;
    }

}
