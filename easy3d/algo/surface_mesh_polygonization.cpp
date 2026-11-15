/** \cond LICENSE
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
 \endcond */

#include <algorithm>
#include <unordered_map>

#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/polygon_partition.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace {

        // A polygon with collinear vertices encloses no area. Float rounding can leave a tiny non-zero
        // Newell vector, so compare the area against the squared longest edge (a dimensionless aspect ratio).
        bool is_zero_area(const std::vector<vec3> &points) {
            if (points.size() < 3)
                return true;

            const vec3 origin = points[0];
            float max_len2 = 0.0f;
            vec3 n(0.0f);
            for (std::size_t i = 0; i < points.size(); ++i) {
                const vec3 &p = points[i];
                const vec3 &q = points[(i + 1) % points.size()];
                const vec3 e = q - p;
                max_len2 = std::max(max_len2, dot(e, e));
                n += cross(p - origin, q - origin);
            }
            if (!(max_len2 > 0.0f))
                return true;
            return 0.5f * length(n) <= 1e-6f * max_len2;
        }

        float cross_2d(const vec2 &u, const vec2 &v) {
            return u.x * v.y - u.y * v.x;
        }

        // Closed segments ab and cd intersect or touch. Adjacent segments share an endpoint, so callers skip them.
        bool segments_intersect(const vec2 &a, const vec2 &b, const vec2 &c, const vec2 &d) {
            const float d1 = cross_2d(b - a, c - a);
            const float d2 = cross_2d(b - a, d - a);
            const float d3 = cross_2d(d - c, a - c);
            const float d4 = cross_2d(d - c, b - c);
            if (((d1 > 0.0f && d2 < 0.0f) || (d1 < 0.0f && d2 > 0.0f)) &&
                ((d3 > 0.0f && d4 < 0.0f) || (d3 < 0.0f && d4 > 0.0f)))
                return true;

            auto between = [](float p, float q, float r) {
                return std::min(p, q) <= r && r <= std::max(p, q);
            };
            if (d1 == 0.0f && between(a.x, b.x, c.x) && between(a.y, b.y, c.y))
                return true;
            if (d2 == 0.0f && between(a.x, b.x, d.x) && between(a.y, b.y, d.y))
                return true;
            if (d3 == 0.0f && between(c.x, d.x, a.x) && between(c.y, d.y, a.y))
                return true;
            if (d4 == 0.0f && between(c.x, d.x, b.x) && between(c.y, d.y, b.y))
                return true;
            return false;
        }

        // A contour is simple if it does not visit the same position twice and no two non-adjacent edges cross or
        // touch. Only simple contours may be handed to PolygonPartition.
        bool is_simple_contour(const std::vector<vec2> &points) {
            const std::size_t n = points.size();
            if (n < 3)
                return false;

            std::vector<std::pair<float, float> > coords;
            coords.reserve(n);
            for (const auto &p : points)
                coords.emplace_back(p.x, p.y);
            std::sort(coords.begin(), coords.end());
            for (std::size_t i = 1; i < n; ++i) {
                if (coords[i] == coords[i - 1])
                    return false;
            }

            // Sort the edges by their left end and only compare an edge with those whose x-range overlaps it.
            struct Segment {
                std::size_t index;
                float min_x, max_x, min_y, max_y;
            };
            std::vector<Segment> segments(n);
            for (std::size_t i = 0; i < n; ++i) {
                const vec2 &p = points[i];
                const vec2 &q = points[(i + 1) % n];
                segments[i] = {i, std::min(p.x, q.x), std::max(p.x, q.x), std::min(p.y, q.y), std::max(p.y, q.y)};
            }
            std::sort(segments.begin(), segments.end(),
                      [](const Segment &a, const Segment &b) { return a.min_x < b.min_x; });

            for (std::size_t i = 0; i < n; ++i) {
                const Segment &s = segments[i];
                for (std::size_t j = i + 1; j < n && segments[j].min_x <= s.max_x; ++j) {
                    const Segment &t = segments[j];
                    if (t.min_y > s.max_y || t.max_y < s.min_y)
                        continue;
                    // skip the same edge and edges sharing a vertex
                    if (t.index == (s.index + 1) % n || (t.index + 1) % n == s.index)
                        continue;
                    if (segments_intersect(points[s.index], points[(s.index + 1) % n],
                                           points[t.index], points[(t.index + 1) % n]))
                        return false;
                }
            }
            return true;
        }

    }


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


    // splits a planar patch (outer boundaries and holes) into convex polygons. The patch boundaries are first
    // normalized with the CSG operators (tessellator.h), because PolygonPartition requires simple contours
    // without self-intersections or overlaps.
    std::vector<SurfaceMeshPolygonization::Contour>
    SurfaceMeshPolygonization::split_complex_contour(
            const std::vector<Contour> &outer_polys,
            const std::vector<Contour> &hole_polys,
            const vec3 &normal,
            const SurfaceMesh *mesh,
            SurfaceMeshBuilder &builder,
            const std::vector<SurfaceMesh::Vertex> &vertex_map) const
    {
        if (outer_polys.empty() || outer_polys.front().empty())
            return {};

        // Resolve self-intersections/overlaps and obtain simple contours. The patch material is on the left of
        // the outer boundaries, so the positive winding rule extracts exactly the patch interior.
        Tessellator tessellator;
        tessellator.set_boundary_only(true);
        tessellator.begin_polygon(normal);
        tessellator.set_winding_rule(Tessellator::WINDING_POSITIVE);
        auto add_contour = [&](const Contour &con) {
            tessellator.begin_contour();
            for (auto v: con)
                tessellator.add_vertex(mesh->position(v), v.idx());
            tessellator.end_contour();
        };
        for (const auto &poly : outer_polys)
            add_contour(poly);
        for (const auto &poly : hole_polys)
            add_contour(poly);
        tessellator.end_polygon();

        const auto &vertices = tessellator.vertices();
        const auto &elements = tessellator.elements();
        if (elements.empty())
            return {};

        // Build the shared vertex pool for PolygonPartition. Vertices of the input mesh are mapped to their
        // corresponding builder vertices; vertices created at intersections are added to the builder.
        const Plane3 plane(mesh->position(outer_polys.front().front()), normal);

        std::vector<vec2> points;
        std::vector<SurfaceMesh::Vertex> point_vertices;
        std::unordered_map<std::size_t, std::size_t> point_index;       // tessellator vertex -> pool index
        std::unordered_map<int, std::size_t> original_point_index;      // input vertex index -> pool index
        std::unordered_map<std::size_t, SurfaceMesh::Vertex> new_vertices;

        auto pool_index = [&](std::size_t i) -> std::size_t {
            auto it = point_index.find(i);
            if (it != point_index.end())
                return it->second;

            const auto *tv = vertices[i];
            const vec3 p(static_cast<float>((*tv)[0]), static_cast<float>((*tv)[1]),
                         static_cast<float>((*tv)[2]));

            std::size_t idx;
            if (tv->index >= 0) {
                // the same input vertex can appear in several contours
                auto original = original_point_index.find(tv->index);
                if (original != original_point_index.end())
                    return original->second;
                idx = points.size();
                points.push_back(plane.to_2d(p));
                point_vertices.push_back(vertex_map[tv->index]);
                original_point_index[tv->index] = idx;
            }
            else {
                auto created = new_vertices.find(i);
                SurfaceMesh::Vertex v;
                if (created != new_vertices.end())
                    v = created->second;
                else {
                    v = builder.add_vertex(p);
                    new_vertices[i] = v;
                }
                idx = points.size();
                points.push_back(plane.to_2d(p));
                point_vertices.push_back(v);
            }

            point_index[i] = idx;
            return idx;
        };

        std::vector<PolygonPartition::Polygon> polygons;    // counter-clockwise
        std::vector<PolygonPartition::Polygon> holes;       // clockwise

        for (const auto &element : elements) {
            if (element.size() < 3)
                continue;

            PolygonPartition::Polygon poly;
            poly.reserve(element.size());
            for (auto i : element)
                poly.push_back(pool_index(i));

            // The tessellator returns outer boundaries in counter-clockwise and holes in clockwise order.
            double twice_area = 0.0;
            for (std::size_t k = 0; k < poly.size(); ++k) {
                const vec2 &a = points[poly[k]];
                const vec2 &b = points[poly[(k + 1) % poly.size()]];
                twice_area += static_cast<double>(a.x) * b.y - static_cast<double>(b.x) * a.y;
            }
            if (twice_area > 0.0)
                polygons.push_back(poly);
            else if (twice_area < 0.0)
                holes.push_back(poly);
        }

        if (polygons.empty())
            return {};

        std::vector<PolygonPartition::Polygon> parts;
        if (!PolygonPartition::apply(points, polygons, holes, parts)) {
            LOG(WARNING) << "failed to perform convex partition of a complex polygon (the patch's faces are kept)";
#ifndef NDEBUG
            for (const auto &poly : polygons)
                LOG(WARNING) << "polygon: " << poly;
            for (const auto &hole : holes)
                LOG(WARNING) << "hole: " << hole;
#endif
            return {};
        }

        std::vector<Contour> result;
        result.reserve(parts.size());
        for (const auto &part : parts) {
            // A normalized boundary can touch itself at a vertex, and the partition then returns a part that
            // repeats that vertex (a pinch point). Split it into simple cycles sharing the pinch vertex, because
            // the builder rejects a polygon with a repeated vertex. The cycles cover the same area.
            std::vector<int> cycle;
            std::unordered_map<int, int> in_cycle;   // pool point -> position in cycle
            auto emit = [&](std::size_t from) {
                if (cycle.size() - from < 3)
                    return;
                // The partition can return zero-area slivers when the patch boundary has collinear vertices.
                std::vector<vec3> pts;
                pts.reserve(cycle.size() - from);
                for (std::size_t k = from; k < cycle.size(); ++k)
                    pts.push_back(vec3(points[cycle[k]].x, points[cycle[k]].y, 0.0f));
                if (is_zero_area(pts))
                    return;
                Contour ct;
                ct.reserve(pts.size());
                for (std::size_t k = from; k < cycle.size(); ++k)
                    ct.push_back(point_vertices[cycle[k]]);
                result.push_back(ct);
            };
            for (int id : part) {
                auto it = in_cycle.find(id);
                if (it != in_cycle.end()) {
                    emit(it->second);
                    for (std::size_t k = it->second + 1; k < cycle.size(); ++k)
                        in_cycle.erase(cycle[k]);
                    cycle.resize(it->second + 1);   // keep the repeated point as the start of the next cycle
                } else {
                    in_cycle[id] = static_cast<int>(cycle.size());
                    cycle.push_back(id);
                }
            }
            emit(0);
        }
        return result;
    }


    void SurfaceMeshPolygonization::internal_apply(SurfaceMesh *mesh, float angle_threshold) {
        SurfaceMesh model = *mesh;

        // Degenerate faces enclose no area (a zero-length edge or a zero-area collinear polygon). Remove them
        // before the planar partition, which cannot merge them and would report them on every pass.
        std::vector<SurfaceMesh::Face> degenerate_faces;
        for (auto f: model.faces()) {
            if (model.is_degenerate(f))
                degenerate_faces.push_back(f);
        }
        for (auto f: degenerate_faces)
            model.delete_face(f);

        const std::string partition_name = "f:planar_partition";
        auto patch_id = model.get_face_property<int>(partition_name);
        if (!patch_id)
            patch_id = model.add_face_property<int>(partition_name, -1);
        else {
            for (auto f: model.faces())
                patch_id[f] = -1;
        }

        const int num = SurfaceMeshEnumerator::enumerate_planar_components(&model, patch_id, angle_threshold);

        // the faces of each planar patch, and the faces that do not belong to any patch (degenerate components)
        std::vector< std::vector<SurfaceMesh::Face> > patch_faces(num);
        std::vector<SurfaceMesh::Face> unassigned_faces;
        for (auto f: model.faces()) {
            int id = patch_id[f];
            if (id >= 0)
                patch_faces[id].push_back(f);
            else
                unassigned_faces.push_back(f);
        }

        // for each planar patch, find all its boundary halfedges
        std::vector<std::set<SurfaceMesh::Halfedge> > boundary_edges(num);
        for (auto e: model.edges()) {
            if (model.is_border(e)) {
                auto h = model.halfedge(e, 0);
                if (model.is_border(h))
                    h = model.opposite(h);
                int id = patch_id[model.face(h)];
                if (id >= 0)    // unassigned faces are kept unchanged, so they contribute no boundary
                    boundary_edges[id].insert(h);
            } else {
                auto f0 = model.face(e, 0);
                auto f1 = model.face(e, 1);
                int id0 = patch_id[f0];
                int id1 = patch_id[f1];
                if (id0 != id1) {
                    if (id0 >= 0)
                        boundary_edges[id0].insert(model.halfedge(e, 0));
                    if (id1 >= 0)
                        boundary_edges[id1].insert(model.halfedge(e, 1));
                }
            }
        }

        // for each planar patch, determine its face normal
        std::vector<vec3> patch_normals(num, vec3(0, 0, 0));
        auto face_normals = model.get_face_property<vec3>("f:normal");
        if (!face_normals)
            model.update_face_normals();
        for (auto f: model.faces()) {
            int id = patch_id[f];
            if (id >= 0)
                patch_normals[id] += face_normals[f];
        }
        for (auto& n : patch_normals)
            n.normalize();

        mesh->clear();
        SurfaceMeshBuilder builder(mesh);
        builder.begin_surface();

        // map the vertices of the input mesh to the vertices of the rebuilt mesh. The new indices cannot be
        // assumed to coincide with the old ones (the input mesh may contain deleted/isolated vertices).
        std::vector<SurfaceMesh::Vertex> vertex_map(model.vertices_size());
        for (auto v: model.vertices())
            vertex_map[v.idx()] = builder.add_vertex(model.position(v));

        auto remap = [&vertex_map](const Contour &poly) {
            Contour result;
            result.reserve(poly.size());
            for (auto v: poly)
                result.push_back(vertex_map[v.idx()]);
            return result;
        };

        auto is_valid_polygon = [](const Contour &poly) {
            if (poly.size() < 3)
                return false;
            for (std::size_t i = 0; i < poly.size(); ++i) {
                if (poly[i] == poly[(i + 1) % poly.size()])
                    return false;
            }
            return true;
        };

        // keep the original faces of a patch when the patch cannot be merged
        auto add_original_faces = [&](const std::vector<SurfaceMesh::Face>& faces) {
            for (auto f: faces) {
                Contour poly;
                for (auto v: model.vertices(f))
                    poly.push_back(v);
                if (!is_valid_polygon(poly)) {
                    LOG_N_TIMES(3, WARNING) << "skipping an invalid face. " << COUNTER;
                    continue;
                }
                if (!builder.add_face(remap(poly)).is_valid())
                    LOG_N_TIMES(3, WARNING) << "failed to add a face to the surface mesh. " << COUNTER;
            }
        };

        add_original_faces(unassigned_faces);

        for (int patch_idx = 0; patch_idx < num; ++patch_idx) {
            auto &edges = boundary_edges[patch_idx]; // contains all the boundaries edges of a planar patch
            const auto &loops = extract_boundary_loop(&model, patch_id, patch_idx, edges);

            std::vector<Loop> outers;   // boundary loops enclosing patch material
            std::vector<Loop> holes;    // boundary loops enclosing holes
            if (loops.empty() || !classify(&model, loops, patch_normals[patch_idx], outers, holes)) {
                // the patch cannot be merged into polygons without holes; keep its faces unchanged
                add_original_faces(patch_faces[patch_idx]);
                continue;
            }

            std::vector<Contour> outer_polys;
            outer_polys.reserve(outers.size());
            bool valid = true;
            for (const auto& loop : outers) {
                Contour poly;
                poly.reserve(loop.size());
                for (auto h: loop)
                    poly.push_back(model.target(h));
                if (!is_valid_polygon(poly)) {
                    valid = false;
                    break;
                }
                outer_polys.push_back(poly);
            }
            if (!valid) {
                add_original_faces(patch_faces[patch_idx]);
                continue;
            }

            std::vector<Contour> hole_polys;
            hole_polys.reserve(holes.size());
            for (const auto& hs: holes) {
                Contour hole_poly;
                hole_poly.reserve(hs.size());
                for (auto h: hs)
                    hole_poly.push_back(model.target(h));
                if (!is_valid_polygon(hole_poly)) {
                    valid = false;
                    break;
                }
                hole_polys.push_back(hole_poly);
            }
            if (!valid) {
                add_original_faces(patch_faces[patch_idx]);
                continue;
            }

            // PolygonPartition requires simple contours, and the builder rejects a polygon with a repeated
            // vertex. A loop that revisits a position through different vertices, or crosses or touches itself,
            // is therefore normalized by the CSG operators, like every patch with holes. The projected loops are
            // kept because the partition works in the patch plane.
            const Plane3 plane(model.position(outer_polys.front().front()), patch_normals[patch_idx]);
            std::vector<std::vector<vec2> > projected(outer_polys.size());
            bool simple = holes.empty();
            for (std::size_t i = 0; i < outer_polys.size() && simple; ++i) {
                const auto& poly = outer_polys[i];
                projected[i].reserve(poly.size());
                for (auto v: poly)
                    projected[i].push_back(plane.to_2d(model.position(v)));
                simple = is_simple_contour(projected[i]);
            }

            std::vector<Contour> polygons;
            if (simple) {
                // Partition every outer loop into convex polygons. Hertel-Mehlhorn returns an already convex loop
                // unchanged, so a convex patch still becomes a single polygon face. The projection preserves the
                // orientation with respect to the patch normal, so counter-clockwise loops stay counter-clockwise.
                bool partition_ok = true;
                for (std::size_t i = 0; i < outer_polys.size() && partition_ok; ++i) {
                    const auto& poly = outer_polys[i];
                    std::vector<PolygonPartition::Polygon> parts;
                    if (!PolygonPartition::apply_HM(projected[i], parts) || parts.empty()) {
                        partition_ok = false;
                        break;
                    }

                    for (const auto& part : parts) {
                        Contour ct;
                        ct.reserve(part.size());
                        std::vector<vec3> pts;
                        pts.reserve(part.size());
                        for (auto id : part) {
                            ct.push_back(vertex_map[poly[id].idx()]);
                            // use the 3D positions: the projection can introduce rounding errors that make
                            // exactly collinear vertices look like a tiny non-zero area
                            pts.push_back(model.position(poly[id]));
                        }
                        if (is_zero_area(pts))
                            continue;
                        polygons.push_back(ct);
                    }
                }
                if (!partition_ok)
                    polygons.clear();   // normalize the boundaries below
            }

            if (polygons.empty()) {
                polygons = split_complex_contour(outer_polys, hole_polys, patch_normals[patch_idx],
                                                 &model, builder, vertex_map);
                if (polygons.empty()) {
                    add_original_faces(patch_faces[patch_idx]);
                    continue;
                }
            }

            for (const auto& ct : polygons) {
                if (!builder.add_face(ct).is_valid())
                    LOG_N_TIMES(3, WARNING) << "failed to add a face to the surface mesh. " << COUNTER;
            }
        }
        builder.end_surface(false);

        merge_colinear_edges(mesh, angle_threshold);
    }


    bool SurfaceMeshPolygonization::classify(const SurfaceMesh *mesh, const std::vector<Loop>& loops,
                                             const vec3& normal, std::vector<Loop>& outers,
                                             std::vector<Loop>& holes) const {
        outers.clear();
        holes.clear();

        for (const auto& loop : loops) {
            if (loop.size() < 3)
                return false;

            // The boundary halfedges are oriented with the patch material on their left. Thus, seen from the
            // patch normal, outer boundaries are counter-clockwise (positive area) and holes clockwise (negative).
            const vec3 origin = mesh->position(mesh->target(loop[0]));
            double twice_area = 0.0;
            for (std::size_t i = 0; i < loop.size(); ++i) {
                const vec3& p = mesh->position(mesh->target(loop[i]));
                const vec3& q = mesh->position(mesh->target(loop[(i + 1) % loop.size()]));
                twice_area += static_cast<double>(dot(cross(p - origin, q - origin), normal));
            }
            const double area = 0.5 * twice_area;
            if (!(area > 0.0) && !(area < 0.0))    // zero or NaN: cannot classify reliably
                return false;

            if (area > 0.0)
                outers.push_back(loop);
            else
                holes.push_back(loop);
        }

        return !outers.empty();
    }


    std::vector<SurfaceMeshPolygonization::Loop>
    SurfaceMeshPolygonization::extract_boundary_loop(const SurfaceMesh *mesh,
                                                     SurfaceMesh::FaceProperty<int> patch_id,
                                                     int comp_id,
                                                     std::set<SurfaceMesh::Halfedge> &boundary_edges) const {
        std::vector<Loop> loops;
        while (!boundary_edges.empty()) {
            SurfaceMesh::Halfedge start = *boundary_edges.begin();
            assert(patch_id[mesh->face(start)] == comp_id);

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
                    auto id = patch_id[f];
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
            if (std::abs(angle) < angle_threshold) {
                // Do not merge if a face sharing this vertex would collapse to a zero-area sliver: the
                // operation cannot be undone, and a degenerate face would break the manifold output.
                bool safe = true;
                std::set<SurfaceMesh::Face> incident;
                for (auto h: mesh->halfedges(v)) {
                    if (mesh->face(h).is_valid())
                        incident.insert(mesh->face(h));
                }
                for (auto f: incident) {
                    std::vector<vec3> pts;
                    for (auto vv: mesh->vertices(f)) {
                        if (vv != v)
                            pts.push_back(mesh->position(vv));
                    }
                    if (is_zero_area(pts)) {
                        safe = false;
                        break;
                    }
                }
                if (safe)
                    vertices.push_back(v);
            }
        }

        for (auto v: vertices)
            mesh->join_edges(v);

        if (!vertices.empty())
            mesh->collect_garbage();
    }
}