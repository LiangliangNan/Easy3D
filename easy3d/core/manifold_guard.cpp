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

    void ManifoldGuard::begin() {
        num_faces_less_three_vertices_ = 0;
        num_faces_duplicated_vertices_= 0;
        num_complex_edges_= 0;
        num_non_manifold_vertices_= 0;
        num_isolated_vertices_= 0;
    }

    void ManifoldGuard::finish() {
        std::string msg = "mesh has topological issues:";
        bool report(false);

        for (auto v : mesh_->vertices()) {
            if (mesh_->is_isolated(v)) {
                mesh_->delete_vertex(v);
                ++num_isolated_vertices_;
            }
        }
        mesh_->garbage_collection();

        if (num_isolated_vertices_ > 0) {
            msg += "\n\t" + std::to_string(num_isolated_vertices_) + " isolated vertices (removed)";
            report = true;
        }

        for (auto v : mesh_->vertices()) {
            if (!mesh_->is_manifold(v))
                ++num_non_manifold_vertices_;
        }
        if (num_non_manifold_vertices_ > 0) {
            msg += "\n\t" + std::to_string(num_non_manifold_vertices_) + " non_manifold vertices (still remain)";
            report = true;
        }

        if (num_faces_less_three_vertices_ > 0) {
            msg += "\n\t" + std::to_string(num_faces_less_three_vertices_) +
                      " faces with less than 3 vertices (ignored)";
            report = true;
        }
        if (num_faces_duplicated_vertices_ > 0) {
            msg += "\n\t" + std::to_string(num_faces_duplicated_vertices_) +
                      " faces with duplicated vertices (ignored)";
            report = true;
        }
        if (num_complex_edges_ > 0) {
            msg += "\n\t" + std::to_string(num_complex_edges_) + " complex edges (fixed)";
            report = true;
        }

        LOG_IF(WARNING, report) << msg;
    }


    SurfaceMesh::Vertex ManifoldGuard::add_vertex(const vec3 &p) {
        return mesh_->add_vertex(p);
    }


    SurfaceMesh::Face ManifoldGuard::add_face(const std::vector<int> &vertices) {
        std::size_t nb_vertices = vertices.size();

        // check if a face has less than 3 vertices;
        if (nb_vertices < 3) {
            ++num_faces_less_three_vertices_;
            return SurfaceMesh::Face();
        }

        // check if a face has duplicated vertices
        for (int i = 0; i < nb_vertices; ++i) {
            for (int j = i+1; j < nb_vertices; ++j) {
                if (vertices[i] == vertices[j]) {
                    ++num_faces_duplicated_vertices_;
                    return SurfaceMesh::Face();
                }
            }
        }

        begin_face();

        face_vertices_.resize(nb_vertices);
        for (int i = 0; i < nb_vertices; ++i)
            face_vertices_[i] = SurfaceMesh::Vertex(vertices[i]);

        // Detect and remove non-manifold edges by duplicating the corresponding vertices.
        for (int s = 0; s < nb_vertices; s++) {
            int t = ((s + 1) % nb_vertices);
            auto h = mesh_->find_halfedge(face_vertices_[s], face_vertices_[t]);
            if (h.is_valid() && (!mesh_->is_boundary(h))) {
                ++num_complex_edges_;
                //std::cout << "complex edge (s <-> t): " << face_vertices_[s] << " <-> " << face_vertices_[t] << std::endl;
                face_vertices_[s] = copy_vertex(face_vertices_[s]);
                face_vertices_[t] = copy_vertex(face_vertices_[t]);
            }
        }

        auto face = mesh_->add_face(face_vertices_);
        end_face();
        return face;
    }


    SurfaceMesh::Vertex ManifoldGuard::copy_vertex(SurfaceMesh::Vertex v) {
        auto points = mesh_->vertex_property<vec3>("v:point");
        return mesh_->add_vertex(points[v]);
    }


    void ManifoldGuard::begin_face() {
        face_vertices_.clear();
    }

    void ManifoldGuard::end_face() {
    }

}
