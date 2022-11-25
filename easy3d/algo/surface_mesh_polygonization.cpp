/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    void SurfaceMeshPolygonization::apply(SurfaceMesh *mesh, float angle_threshold) {
        if (!mesh)
            return;

        int orig_faces = static_cast<int>(mesh->n_faces());
        int diff(0);
        do {
            int prev_faces = static_cast<int>(mesh->n_faces());
            internal_apply(mesh, angle_threshold);
            diff = static_cast<int>(mesh->n_faces()) - prev_faces;
        } while (diff != 0);

        int current_faces = static_cast<int>(mesh->n_faces());
        if (current_faces < orig_faces)
            LOG(INFO) << orig_faces << " faces merged into " << current_faces;
    }


    void SurfaceMeshPolygonization::internal_apply(SurfaceMesh *mesh, float angle_threshold) {
        SurfaceMesh model = *mesh;

        const std::string partition_name = "f:planar_partition";
        planar_segments_ = model.get_face_property<int>(partition_name);
        if (!planar_segments_)
            planar_segments_ = model.add_face_property<int>(partition_name, -1);
        else {
            for (auto f: model.faces())
                planar_segments_[f] = -1;
        }

        int num = SurfaceMeshEnumerator::enumerate_planar_components(&model, planar_segments_, angle_threshold);
        // for each planar patch, find all its boundaries halfedges
        std::vector<std::set<SurfaceMesh::Halfedge> > boundary_edges(num);

        for (auto e: model.edges()) {
            if (model.is_border(e)) {
                auto h = model.halfedge(e, 0);
                if (model.is_border(h))
                    h = model.opposite(h);
                int id = planar_segments_[model.face(h)];
                boundary_edges[id].insert(h);
            } else {
                auto f0 = model.face(e, 0);
                auto f1 = model.face(e, 1);
                int id0 = planar_segments_[f0];
                int id1 = planar_segments_[f1];
                if (id0 != id1) {
                    boundary_edges[id0].insert(model.halfedge(e, 0));
                    boundary_edges[id1].insert(model.halfedge(e, 1));
                }
            }
        }

        mesh->clear();
        SurfaceMeshBuilder builder(mesh);
        builder.begin_surface();

        for (auto v: model.vertices())
            builder.add_vertex(model.position(v));

        typedef std::vector<vec3> Hole;

        int num_faces_with_holes = 0;
        for (std::size_t i = 0; i < boundary_edges.size(); ++i) {
            auto &edges = boundary_edges[i]; // contains all the boundaries edges of a planar region
            const auto &loops = extract_boundary_loop(&model, static_cast<int>(i), edges);

            Loop outer;
            std::vector<Loop> holes;
            if (loops.size() == 1)
                outer = loops[0];
            else if (loops.size() > 1) {
                classify(&model, loops, outer, holes);
                ++num_faces_with_holes;
            } else // empty
                break;

            std::vector<SurfaceMesh::Vertex> vts;
            for (auto h: outer) {
                SurfaceMesh::Vertex v = model.target(h);
                vts.push_back(v);
            }

            auto f = builder.add_face(vts);
            if (!f.is_valid()) {
                LOG_N_TIMES(3, WARNING) << "failed to add a face to the surface mesh. " << COUNTER;
                break;
            }

            for (const auto &hole: holes) {
                auto face_holes = mesh->face_property<std::vector<Hole> >("f:holes");
                Hole a_hole;
                for (auto h: hole)
                    a_hole.push_back(model.position(model.target(h)));
                face_holes[f].push_back(a_hole);
            }
        }
        builder.end_surface(false);

        // print a warning message
        if (num_faces_with_holes > 0) {
            if (num_faces_with_holes == 1)
                LOG(WARNING) << num_faces_with_holes << " face has holes (not handled)";
            else
                LOG(WARNING) << num_faces_with_holes << " faces have holes (not handled)";
        }

        merge_colinear_edges(mesh, angle_threshold);
    }


    // classify the loops of a planar region into an "outer" loop and several "holes".
    void SurfaceMeshPolygonization::classify(const SurfaceMesh *mesh, const std::vector<Loop> &loops, Loop &outer,
                                             std::vector<Loop> &holes) {
        auto loop_length = [](const SurfaceMesh *m, const Loop &loop) -> float {
            float length = 0.0f;
            for (auto h: loop)
                length += m->edge_length(h);
            return length;
        };

        std::size_t outer_idx = 0;
        float longest_length = loop_length(mesh, loops[0]);
        for (std::size_t i = 1; i < loops.size(); ++i) {
            float length = loop_length(mesh, loops[i]);
            if (length > longest_length) {
                longest_length = length;
                outer_idx = i;
            }
        }

        outer.clear();
        holes.clear();
        for (std::size_t i = 0; i < loops.size(); ++i) {
            if (i == outer_idx)
                outer = loops[i];
            else
                holes.push_back(loops[i]);
        }
    }


    std::vector<SurfaceMeshPolygonization::Loop>
    SurfaceMeshPolygonization::extract_boundary_loop(const SurfaceMesh *mesh, int comp_id,
                                                     std::set<SurfaceMesh::Halfedge> &boundary_edges) {
        std::vector<Loop> loops;
        while (!boundary_edges.empty()) {
            SurfaceMesh::Halfedge start = *boundary_edges.begin();
            assert(planar_segments_[mesh->face(start)] == comp_id);

            Loop loop;
            loop.push_back(start);
            boundary_edges.erase(start);

            SurfaceMesh::Halfedge cur = start;
            do {
                SurfaceMesh::Halfedge next = mesh->next(cur);
                if (mesh->is_border(mesh->opposite(next))) {
                    cur = next;
                    if (cur != start) {
                        loop.push_back(cur);
                        boundary_edges.erase(cur);
                    }
                } else {
                    SurfaceMesh::Halfedge test = mesh->opposite(next);
                    auto f = mesh->face(test);
                    auto id = planar_segments_[f];
                    if (id != comp_id) {
                        cur = next;
                        if (cur != start) {
                            loop.push_back(cur);
                            boundary_edges.erase(cur);
                        }
                    } else
                        cur = test;
                }
            } while (cur != start);

            loops.push_back(loop);
        }
        return loops;
    }


    void SurfaceMeshPolygonization::merge_colinear_edges(SurfaceMesh *mesh, float angle_threshold) {
        if (!mesh)
            return;

        auto to_vector = [](SurfaceMesh *m, SurfaceMesh::Halfedge h) -> vec3 {
            return m->position(m->target(h)) - m->position(m->source(h));
        };

        std::vector<SurfaceMesh::Vertex> vertices;
        for (auto v: mesh->vertices()) {
            if (mesh->valence(v) != 2)
                continue;

            SurfaceMesh::Halfedge h1 = mesh->out_halfedge(v);
            SurfaceMesh::Halfedge h2 = mesh->prev(h1);
            vec3 v1 = to_vector(mesh, h1);
            v1.normalize();
            vec3 v2 = to_vector(mesh, h2);
            v2.normalize();
            auto angle = geom::angle(v1, v2); // in [-pi, pi]
            angle = geom::to_degrees(std::abs(angle));
            if (std::abs(angle) < angle_threshold)
                vertices.push_back(v);
        }

        for (auto v: vertices)
            mesh->join_edges(v);

        if (!vertices.empty())
            mesh->collect_garbage();
    }
}