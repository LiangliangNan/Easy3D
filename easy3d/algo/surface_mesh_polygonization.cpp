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

#include <unordered_map>

#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/polygon_partition.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    void SurfaceMeshPolygonization::apply(SurfaceMesh *mesh, float angle_threshold) {
        if (!mesh)
            return;

        int orig_faces = static_cast<int>(mesh->n_faces());
        int num_reduced(0);
        do {
            int prev_faces = static_cast<int>(mesh->n_faces());
            internal_apply(mesh, angle_threshold);
            num_reduced = prev_faces - static_cast<int>(mesh->n_faces());
        } while (num_reduced > 0);

        int current_faces = static_cast<int>(mesh->n_faces());
        if (current_faces < orig_faces)
            LOG(INFO) << orig_faces << " faces merged into " << current_faces;
    }


    // split a complex face (with duplicate vertices, thus non-manifold) into a few simple faces
    std::vector<SurfaceMeshPolygonization::Contour>
    SurfaceMeshPolygonization::split_complex_contour(
            const Contour &outer_poly,                      // the outer polygon represented by a list of SurfaceMesh::Vertex
            const std::vector<Contour> &input_hole_polys,   // the holes each represented by a list of SurfaceMesh::Vertex
            const vec3 &normal,                     // the normal of the polygon
            const SurfaceMesh *mesh) const
    {
        if (input_hole_polys.empty()) {
            // check if 'vts' has duplicate vertex, if so, returns the first found vertex
            std::vector<SurfaceMesh::Vertex> tmp = outer_poly;
            std::sort(tmp.begin(), tmp.end());
            auto pos = std::adjacent_find(tmp.begin(), tmp.end());
            if (pos == tmp.end()) // this is a simple contour
                return {outer_poly};

                // if reached here, it is a complex polygon. We do convex partition.
#ifndef NDEBUG
            LOG(INFO) << "complex contour: " << outer_poly;
            LOG(INFO) << "first duplicate vertex: " << *pos;
#endif
        }

        // Note: the "no hole" cases can also be treated as general cases using the same code below.
        //       But since the "no hole" cases are more common, it is thus preferred to use the OPT algorithm
        //       (for general cases with holes, only the HM algorithm is available).
        if (input_hole_polys.empty()) {
            std::unordered_map<std::size_t, SurfaceMesh::Vertex> index_map;

            // construct the supporting plane of this planar region
            Plane3 plane(mesh->position(outer_poly[0]), normal);
            // project all points onto the supporting plane
            std::vector<vec2> polygon;
            for (std::size_t i = 0; i < outer_poly.size(); ++i) {
                const auto &v = outer_poly[i];
                const auto &p = mesh->position(v);
                polygon.push_back(plane.to_2d(p));
                index_map[i] = v;
            }

            std::vector<PolygonPartition::Polygon> parts;
            if (!PolygonPartition::apply_OPT(polygon, parts)) {
                LOG(WARNING) << "failed to perform convex partition of a complex polygon (the polygon ignored)";
#ifndef NDEBUG
                LOG(WARNING) << "outer loop: " << outer_poly;
#endif
                return {};
            }

            std::vector<SurfaceMeshPolygonization::Contour> result;
            for (auto &poly: parts) {
                std::vector<SurfaceMesh::Vertex> vts;
                for (const auto &id: poly)
                    vts.push_back(index_map[id]);
                result.push_back(vts);
            }
            return result;
        }
        else {
            // all the vertex coordinates
            std::vector<vec2> points;
            // the SurfaceMesh::Vertex corresponding to the i-th point in "points"
            std::unordered_map<std::size_t, SurfaceMesh::Vertex> index_map;

            std::vector<PolygonPartition::Polygon> input_polys; // non-hole polygons, in counter-clockwise order.
            std::vector<PolygonPartition::Polygon> hole_polys; // hole polygons, in clockwise order.

            // construct the supporting plane of this planar region
            Plane3 plane(mesh->position(outer_poly[0]), normal);

            // project all points of the outer_polygon onto the supporting plane;
            // meanwhile, collect the vertex indices of the non-hole polygon
            PolygonPartition::Polygon nonhole_poly;
            int vtx_idx = 0;
            for (std::size_t i = 0; i < outer_poly.size(); ++i) {
                const auto &v = outer_poly[i];
                const auto &p = mesh->position(v);
                points.push_back(plane.to_2d(p));
                index_map[vtx_idx] = v;
                nonhole_poly.push_back(vtx_idx);
                ++vtx_idx;
            }
            input_polys.push_back(nonhole_poly);

            // project all points of the hole-polygons onto the supporting plane;
            // meanwhile, collect the vertex indices of the hole polygons
            for (auto& hole : input_hole_polys) {
                PolygonPartition::Polygon hole_poly;
                for (std::size_t i = 0; i < hole.size(); ++i) {
                    const auto &v = hole[i];
                    const auto &p = mesh->position(v);
                    points.push_back(plane.to_2d(p));
                    index_map[vtx_idx] = v;
                    hole_poly.push_back(vtx_idx);
                    ++vtx_idx;
                }
                hole_polys.push_back(hole_poly);
            }

            // now let's do the convex partition
            std::vector<PolygonPartition::Polygon> parts;
            if (!PolygonPartition::apply(points, input_polys, hole_polys, parts)) {
                LOG(WARNING) << "failed to perform convex partition of a complex polygon (the polygon ignored)";
#ifndef NDEBUG
                LOG(WARNING) << "outer loop: " << outer_poly;
                for (const auto& hole : input_hole_polys) {
                    LOG(WARNING) << "hole: " << hole;
                }
#endif
                return {};
            }

            std::vector<SurfaceMeshPolygonization::Contour> result;
            for (auto &poly: parts) {
                std::vector<SurfaceMesh::Vertex> vts;
                for (const auto &id: poly)
                    vts.push_back(index_map[id]);
                result.push_back(vts);
            }
            return result;
        }
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

        const int num = SurfaceMeshEnumerator::enumerate_planar_components(&model, planar_segments_, angle_threshold);
        // for each planar patch, find all its boundary halfedges
        std::vector<std::set<SurfaceMesh::Halfedge> > boundary_edges(num);  // the boundary halfedges for each planar patch

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

        // for each planar patch, determine its face normal
        std::vector<vec3> region_normals(num, vec3(0, 0, 0));  // the normal for each planar patch
        auto face_normals = model.get_face_property<vec3>("f:normal");
        if (!face_normals)
            model.update_face_normals();
        for (auto f: model.faces()) {
            int region_id = planar_segments_[f];
            region_normals[region_id] += face_normals[f];
        }
        for (auto& n : region_normals)
            n.normalize();

        mesh->clear();
        SurfaceMeshBuilder builder(mesh);
        builder.begin_surface();

        for (auto v: model.vertices())
            builder.add_vertex(model.position(v));

        for (int region_idx = 0; region_idx < num; ++region_idx) {
            auto &edges = boundary_edges[region_idx]; // contains all the boundaries edges of a planar region
            const auto &loops = extract_boundary_loop(&model, region_idx, edges);

            // the outer contour represented by a list of SurfaceMesh::Halfedge
            Loop outer;
            // the holes each represented by a list of SurfaceMesh::Halfedge
            std::vector<Loop> holes;
            if (loops.size() == 1)
                outer = loops[0];
            else if (loops.size() > 1)
                classify(&model, loops, outer, holes);
            else // empty
                break;

            // the outer polygon represented by a list of SurfaceMesh::Vertex
            Contour outer_poly;
            for (auto h: outer) {
                SurfaceMesh::Vertex v = model.target(h);
                outer_poly.push_back(v);
            }

            // the holes each represented by a list of SurfaceMesh::Vertex
            std::vector<Contour> hole_polys;
            for (const auto& hs: holes) {
                Contour hole_poly;
                for (auto h: hs) {
                    SurfaceMesh::Vertex v = model.target(h);
                    hole_poly.push_back(v);
                }
                hole_polys.push_back(hole_poly);
            }

            const auto& normal = region_normals[region_idx];
            const auto contours = split_complex_contour(outer_poly, hole_polys, normal, &model);
            for (auto ct : contours) {
                auto f = builder.add_face(ct);
                if (!f.is_valid()) {
                    LOG_N_TIMES(3, WARNING) << "failed to add a face to the surface mesh. " << COUNTER;
                    continue;
                }
            }
        }
        builder.end_surface(false);

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