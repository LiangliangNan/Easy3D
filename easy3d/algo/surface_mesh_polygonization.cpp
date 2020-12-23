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

#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/util/logging.h>




#include <easy3d/fileio/surface_mesh_io.h>

namespace easy3d {


    void SurfaceMeshPolygonization::apply(SurfaceMesh *mesh) {
        int orig_faces = mesh->n_faces();
        int prev_faces(orig_faces), diff(0);
        do {
            prev_faces = mesh->n_faces();
            internal_apply(mesh);
            diff = mesh->n_faces() - prev_faces;
        } while (diff != 0);

        int current_faces = mesh->n_faces();
        if (current_faces < orig_faces)
            LOG(INFO) << orig_faces << " faces merged into " << current_faces << std::endl;
    }


    void SurfaceMeshPolygonization::internal_apply(SurfaceMesh *mesh) {
        if (!mesh)
            return;

        SurfaceMesh model = *mesh;

        const std::string partition_name = "f:planar_partition";
        planar_segments_ = model.get_face_property<int>(partition_name);
        if (!planar_segments_)
            planar_segments_ = model.add_face_property<int>(partition_name, -1);
        else {
            for (auto f : model.faces())
                planar_segments_[f] = -1;
        }

        int num = SurfaceMeshEnumerator::enumerate_planar_components(&model, planar_segments_);
        // for each planar patch, find a starting halfedge
        std::vector<SurfaceMesh::Halfedge> starters(num, SurfaceMesh::Halfedge());

        for (auto e : model.edges()) {
            if (model.is_border(e)) {
                auto h = model.halfedge(e, 0);
                if (model.is_border(h))
                    h = model.opposite(h);
                int id = planar_segments_[model.face(h)];
                if (starters[id] == SurfaceMesh::Halfedge())
                    starters[id] = h;
            }
            else {
                auto f0 = model.face(e, 0);
                auto f1 = model.face(e, 1);
                int id0 = planar_segments_[f0];
                int id1 = planar_segments_[f1];
                if (id0 != id1) {
                    if (starters[id0] == SurfaceMesh::Halfedge())
                        starters[id0] = model.halfedge(e, 0);
                    if (starters[id1] == SurfaceMesh::Halfedge())
                        starters[id1] = model.halfedge(e, 1);
                }
            }
        }

        mesh->clear();
        ManifoldBuilder builder(mesh);
        builder.begin_surface();

        for (auto v : model.vertices())
            builder.add_vertex(model.position(v));

        for (std::size_t i = 0; i < starters.size(); ++i) {
            const std::vector<SurfaceMesh::Halfedge> &loop = extract_boundary_loop(&model, i, starters[i]);

            std::vector<SurfaceMesh::Vertex> vts;
            for (std::size_t j = 0; j < loop.size(); ++j) {
                SurfaceMesh::Vertex v = model.target(loop[j]);
                vts.push_back(v);
            }
            auto f = builder.add_face(vts);
            if (!f.is_valid()) {
                LOG_FIRST_N(WARNING, 1) << "failed to add a face to the surface mesh";
                // for debug. I had some extremely complex examples!!!!
//                SurfaceMeshIO::save("/Users/lnan/Desktop/bug2.off", mesh);
                break;
            }
        }
        builder.end_surface(false);

        merge_colinear_edges(mesh);
    }


    std::vector<SurfaceMesh::Halfedge>
    SurfaceMeshPolygonization::extract_boundary_loop(SurfaceMesh *mesh, int comp_id, SurfaceMesh::Halfedge start) {
        assert(planar_segments_[mesh->face(start)] == comp_id);

        std::vector<SurfaceMesh::Halfedge> loop;
        loop.push_back(start);

        SurfaceMesh::Halfedge cur = start;
        do {
            SurfaceMesh::Halfedge next = mesh->next(cur);
            if (mesh->is_border(mesh->opposite(next))) {
                cur = next;
                if (cur != start)
                    loop.push_back(cur);
            }
            else {
                SurfaceMesh::Halfedge test = mesh->opposite(next);
                auto f = mesh->face(test);
                auto id = planar_segments_[f];
                if (id != comp_id) {
                    cur = next;
                    if (cur != start)
                        loop.push_back(cur);
                }
                else
                    cur = test;
            }
        } while (cur != start);

        return loop;
    }


    void SurfaceMeshPolygonization::merge_colinear_edges(SurfaceMesh *mesh) {
        if (!mesh)
            return;

        auto to_vector = [](SurfaceMesh *m, SurfaceMesh::Halfedge h) -> vec3 {
            return m->position(m->target(h)) - m->position(m->source(h));
        };

        std::vector<SurfaceMesh::Vertex> vertices;
        for (auto v : mesh->vertices()) {
            if (mesh->valence(v) != 2)
                continue;

            SurfaceMesh::Halfedge h1 = mesh->out_halfedge(v);
            SurfaceMesh::Halfedge h2 = mesh->prev(h1);
            vec3 v1 = to_vector(mesh, h1);
            v1.normalize();
            vec3 v2 = to_vector(mesh, h2);
            v2.normalize();
            auto angle = geom::angle(v1, v2); // in [-pi, pi]
            angle = rad2deg(std::abs(angle));
            if (std::abs(angle) < 1.0f)
                vertices.push_back(v);
        }

        for (std::size_t i = 0; i < vertices.size(); ++i) {
            auto v = vertices[i];
            mesh->join_edges(v);
        }
        if (!vertices.empty())
            mesh->collect_garbage();
    }
}