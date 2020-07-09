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

#include <easy3d/algo_ext/self_intersection.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>

#include <queue>

#define REMESH_INTERSECTIONS_TIMING

#ifdef REMESH_INTERSECTIONS_TIMING

#include <easy3d/util/stop_watch.h>

#endif

namespace easy3d {


    void SelfIntersection::mark_offensive(int f) {
        if (offending_.count(f) == 0) {
            // first time marking, initialize with new id and empty list
            offending_[f] = {};
        }
    }

    void SelfIntersection::count_intersection(int fa, int fb) {
        mark_offensive(fa);
        mark_offensive(fb);
    }

    bool SelfIntersection::double_shared_vertex(
            const Triangle &A,
            const Triangle &B,
            const std::vector<std::pair<int, int> > &shared) {
        // must be co-planar
        if (
                A.triangle.supporting_plane() != B.triangle.supporting_plane() &&
                A.triangle.supporting_plane() != B.triangle.supporting_plane().opposite()) {
            return false;
        }
        // Since A and B are non-degenerate the intersection must be a polygon
        // (triangle). Either
        //   - the vertex of A (B) opposite the shared edge of lies on B (A), or
        //   - an edge of A intersects and edge of B without sharing a vertex
        //
        // Determine if the vertex opposite edge (a0,a1) in triangle A lies in
        // (intersects) triangle B
        const auto &opposite_point_inside = [](const Triangle_3 &A, const int a0, const int a1,
                                               const Triangle_3 &B) -> bool {
            // get opposite index
            int a2 = -1;
            for (unsigned short c = 0; c < 3; c++) {
                if (c != a0 && c != a1) {
                    a2 = c;
                    break;
                }
            }
            assert(a2 != -1);
            bool ret = CGAL::do_intersect(A.vertex(a2), B);
            return ret;
        };

        // Determine if edge opposite vertex va in triangle A intersects edge
        // opposite vertex vb in triangle B.
        const auto &opposite_edges_intersect = [](const Triangle_3 &A, int va, const Triangle_3 &B, int vb) -> bool {
            Segment_3 sa(A.vertex((va + 1) % 3), A.vertex((va + 2) % 3));
            Segment_3 sb(B.vertex((vb + 1) % 3), B.vertex((vb + 2) % 3));
            bool ret = CGAL::do_intersect(sa, sb);
            return ret;
        };

        if (
                !opposite_point_inside(A.triangle, shared[0].first, shared[1].first, B.triangle) &&
                !opposite_point_inside(B.triangle, shared[0].second, shared[1].second, A.triangle) &&
                !opposite_edges_intersect(A.triangle, shared[0].first, B.triangle, shared[1].second) &&
                !opposite_edges_intersect(A.triangle, shared[1].first, B.triangle, shared[0].second)) {
            return false;
        }

        // there is an intersection indeed
        count_intersection(A.index, B.index);
        if (!construct_intersection_)
            return true;

        // Construct intersection
        try {
            // This can fail for Epick but not Epeck
            CGAL::Object result = CGAL::intersection(A.triangle, B.triangle);
            if (!result.empty()) {
                if (CGAL::object_cast<Segment_3>(&result)) {
                    // not coplanar
                    assert(false && "Co-planar non-degenerate triangles should intersect over triangle");
                    return false;
                } else if (CGAL::object_cast<Point_3>(&result)) {
                    // this "shouldn't" happen but does for inexact
                    assert(false && "Co-planar non-degenerate triangles should intersect over triangle");
                    return false;
                } else {
                    const int fa = A.index;
                    const int fb = B.index;
                    // Triangle object
                    offending_[fa].push_back({fb, result});
                    offending_[fb].push_back({fa, result});
                    return true;
                }
            } else {
                // CGAL::intersection is disagreeing with do_intersect
                assert(false && "CGAL::intersection should agree with predicate tests");
                return false;
            }
        }
        catch (...) {
            // This catches some cgal assertion:
            //     CGAL error: assertion violation!
            //     Expression : is_finite(d)
            //     File       : /opt/local/include/CGAL/GMP/Gmpq_type.h
            //     Line       : 132
            //     Explanation:
            // But only if NDEBUG is not defined, otherwise there's an uncaught
            // "Floating point exception: 8" SIGFPE
            return false;
        }
    }

    bool SelfIntersection::single_shared_vertex(
            const Triangle &A, const Triangle &B,
            int va, int vb) {
        if (single_shared_vertex(A, B, va))
            return true;
        return single_shared_vertex(B, A, vb);
    }

    bool SelfIntersection::single_shared_vertex(const Triangle &A, const Triangle &B, int va) {
        const int fa = A.index;
        const int fb = B.index;

        // This was not a good idea. It will not handle coplanar triangles well.
        Segment_3 sa(A.triangle.vertex((va + 1) % 3), A.triangle.vertex((va + 2) % 3));

        if (CGAL::do_intersect(sa, B.triangle)) {
            // can't put count_intersection(fa,fb) here since we use intersect below
            // and then it will be counted twice.
            if (!construct_intersection_) {
                count_intersection(fa, fb);
                return true;
            }
            CGAL::Object result = CGAL::intersection(sa, B.triangle);
            if (const Point_3 *p = CGAL::object_cast<Point_3>(&result)) {
                // Single intersection --> segment from shared point to intersection
                CGAL::Object seg = CGAL::make_object(Segment_3(A.triangle.vertex(va), *p));
                count_intersection(fa, fb);
                offending_[fa].push_back({fb, seg});
                offending_[fb].push_back({fa, seg});
                return true;
            } else if (CGAL::object_cast<Segment_3>(&result)) {
                // Need to do full test. Intersection could be a general poly.
                bool test = intersect(A, B);
                ((void) test);
                assert(test && "intersect should agree with do_intersect");
                return true;
            } else {
                LOG(ERROR) << "Segment ∩ triangle neither point nor segment?";
                assert(false);
            }
        }

        return false;
    }


    bool SelfIntersection::intersect(const Triangle &A, const Triangle &B) {
        const int fa = A.index;
        const int fb = B.index;

        // Determine whether there is an intersection
        if (!CGAL::do_intersect(A.triangle, B.triangle))
            return false;

        count_intersection(fa, fb);
        if (construct_intersection_) {
            // Construct intersection
            CGAL::Object result = CGAL::intersection(A.triangle, B.triangle);
            offending_[fa].push_back({fb, result});
            offending_[fb].push_back({fa, result});
        }
        return true;
    }


    bool SelfIntersection::do_intersect(const Triangle &A, const Triangle &B) {
        // Number of combinatorially shared vertices
        int num_comb_shared_vertices = 0;

        // Number of geometrically shared vertices (*not* including combinatorially shared)
        int num_geom_shared_vertices = 0;

        // Keep track of shared vertex indices
        std::vector<std::pair<int, int> > shared;
        for (unsigned short ea = 0; ea < 3; ++ea) {
            for (unsigned short eb = 0; eb < 3; ++eb) {
                if (A.vertices[ea] == B.vertices[eb]) {
                    ++num_comb_shared_vertices;
                    shared.emplace_back(ea, eb);
                } else if (A.triangle.vertex(ea) == B.triangle.vertex(eb)) {
                    ++num_geom_shared_vertices;
                    shared.emplace_back(ea, eb);
                }
            }
        }
        const int total_shared_vertices = num_comb_shared_vertices + num_geom_shared_vertices;
        if (num_comb_shared_vertices == 3) {
            assert(shared.size() == 3);
            // Combinatorially duplicated faces should be removed before calling SelftIntersection.
            ++total_comb_duplicated_faces_;
            return false;
        }
        if (total_shared_vertices == 3) {
            assert(shared.size() == 3);
            // Geometrically duplicate faces should be removed before calling SelftIntersection.
            ++total_geom_duplicated_faces_;
            return false;
        }
        if (total_shared_vertices == 2) {
            assert(shared.size() == 2);
            // TODO: What about coplanar?
            //
            // o    o
            // |\  /|
            // | \/ |
            // | /\ |
            // |/  \|
            // o----o
            return double_shared_vertex(A, B, shared);
        }
        assert(total_shared_vertices <= 1);
        if (total_shared_vertices == 1)
            return single_shared_vertex(A, B, shared[0].first, shared[0].second);
        else
            return intersect(A, B);
    }


    std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
    SelfIntersection::detect(SurfaceMesh *mesh, bool construct) {
        std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > result;
        if (!mesh)
            return result;

        if (!mesh->is_triangle_mesh()) {
            mesh->triangulate();
            LOG(WARNING) << "input mesh triangulated to perform self intersection detection";
        }

        construct_intersection_ = construct;
        offending_.clear();
        total_comb_duplicated_faces_ = 0;
        total_geom_duplicated_faces_ = 0;

        triangle_faces_ = mesh_to_cgal_triangle_list(mesh);

        // bounding boxes of the triangles
        std::vector<Box> boxes;
        for (TrianglesIterator it = triangle_faces_.begin(); it != triangle_faces_.end(); ++it) {
            if (!it->triangle.is_degenerate())
                boxes.push_back(Box(it->triangle.bbox(), it));
        }
        std::vector<std::pair<TrianglesIterator, TrianglesIterator> > intersecting_boxes;
        auto cb = [&intersecting_boxes](const Box &a, const Box &b) {
            intersecting_boxes.push_back({a.handle(), b.handle()});
        };
        CGAL::box_self_intersection_d(boxes.begin(), boxes.end(), cb);

        for (const auto &b : intersecting_boxes) {
            const Triangle &ta = *b.first;
            const Triangle &tb = *b.second;
            if (do_intersect(ta, tb))
                result.emplace_back(std::make_pair(ta.face, tb.face));
        }

        std::string msg("");
        if (total_comb_duplicated_faces_ > 0)
            msg += ("Model has " + std::to_string(total_comb_duplicated_faces_) +
                    " combinatorially duplicated faces. ");
        if (total_geom_duplicated_faces_ > 0)
            msg += ("Model has " + std::to_string(total_geom_duplicated_faces_) + " geometrically duplicated faces. ");
        if (total_comb_duplicated_faces_ > 0 || total_geom_duplicated_faces_ > 0) {
            msg += "Remove duplicated faces may produce better result";
            LOG(WARNING) << msg;
        }

        return result;
    }


    SelfIntersection::Triangles SelfIntersection::mesh_to_cgal_triangle_list(SurfaceMesh *mesh) {
        auto prop = mesh->get_vertex_property<vec3>("v:point");

        Triangles triangles;
        for (auto f : mesh->faces()) {
            std::vector<Point_3> points;
            std::vector<SurfaceMesh::Vertex> vertices;
            for (auto v : mesh->vertices(f)) {
                const vec3 &p = prop[v];
                points.push_back(Point_3(p.x, p.y, p.z));
                vertices.push_back(v);
            }

            if (points.size() == 3) {
                Triangle t(points[0], points[1], points[2], f);
                t.index = f.idx();
                t.vertices = vertices;
                triangles.push_back(t);
            } else {
                LOG_FIRST_N(WARNING, 1) << "only triangular meshes can be processed (this is the first record)";
                return triangles;
            }
        }
        return triangles;
    }


    void SelfIntersection::insert_into_cdt(const CGAL::Object &obj, const Plane_3 &P, CDT_plus_2 &cdt) {
        if (const Segment_3 *iseg = CGAL::object_cast<Segment_3>(&obj)) {
            // Add segment constraint
            cdt.insert_constraint(P.to_2d(iseg->vertex(0)), P.to_2d(iseg->vertex(1)));
        } else if (const Point_3 *ipoint = CGAL::object_cast<Point_3>(&obj)) {
            // Add point
            cdt.insert(P.to_2d(*ipoint));
        } else if (const Triangle_3 *itri = CGAL::object_cast<Triangle_3>(&obj)) {
            // Add 3 segment constraints
            cdt.insert_constraint(P.to_2d(itri->vertex(0)), P.to_2d(itri->vertex(1)));
            cdt.insert_constraint(P.to_2d(itri->vertex(1)), P.to_2d(itri->vertex(2)));
            cdt.insert_constraint(P.to_2d(itri->vertex(2)), P.to_2d(itri->vertex(0)));
        } else if (const std::vector<Point_3> *polyp = CGAL::object_cast<std::vector<Point_3> >(&obj)) {
            const std::vector<Point_3> &poly = *polyp;
            const std::size_t m = poly.size();
            assert(m >= 2);
            for (std::size_t p = 0; p < m; p++) {
                const std::size_t np = (p + 1) % m;
                cdt.insert_constraint(P.to_2d(poly[p]), P.to_2d(poly[np]));
            }
        } else {
            LOG(ERROR) << "Unknown intersection object!";
            throw std::runtime_error("Unknown intersection object!");
        }
    }


    void SelfIntersection::projected_cdt(
            const std::vector<CGAL::Object> &objects,
            const Plane_3 &P,
            std::vector<Point_3> &vertices,
            std::vector<std::vector<int> > &faces) {
        CDT_plus_2 cdt;
        for (const auto &obj : objects)
            insert_into_cdt(obj, P, cdt);

        // Read off vertices of the cdt, remembering index
        std::map<typename CDT_plus_2::Vertex_handle, int> v2i;
        int count = 0;
        for (
                auto itr = cdt.finite_vertices_begin();
                itr != cdt.finite_vertices_end();
                itr++) {
            vertices.push_back(P.to_3d(itr->point()));
            v2i[itr] = count;
            count++;
        }
        // Read off faces and store index triples
        for (
                auto itr = cdt.finite_faces_begin();
                itr != cdt.finite_faces_end();
                itr++) {
            faces.push_back(
                    {v2i[itr->vertex(0)], v2i[itr->vertex(1)], v2i[itr->vertex(2)]});
        }
    }


    SurfaceMesh *SelfIntersection::remesh(SurfaceMesh *mesh, bool stitch) {
#ifdef REMESH_INTERSECTIONS_TIMING
        StopWatch w;
        w.start();
        LOG(INFO) << "detecting intersections... ";
#endif

        const auto &intersecting_faces = detect(mesh, true);

#ifdef REMESH_INTERSECTIONS_TIMING
        LOG(INFO) << "done. " << intersecting_faces.size()
                   << " pairs of intersecting triangles. " << w.time_string();

        w.restart();
        LOG(INFO) << "overlap analysis... ";
#endif

        if (intersecting_faces.empty())
            return nullptr;

        typedef std::pair<std::size_t, std::size_t> Edge;
        struct EdgeHash {
            std::size_t operator()(const Edge &e) const {
                return (e.first * 805306457) ^ (e.second * 201326611);
            }
        };
        typedef std::unordered_map<Edge, std::vector<int>, EdgeHash> EdgeMap;

        const std::size_t num_faces = mesh->n_faces();
        const std::size_t num_base_vertices = mesh->n_vertices();
        assert(num_faces == triangle_faces_.size());

        std::vector<bool> is_offending(num_faces, false);
        for (const auto &itr : offending_)
            is_offending[itr.first] = true;

        // Cluster overlaps so that co-planar clusters are resolved only once
        std::unordered_map<int, std::vector<int> > intersecting_and_coplanar;
        for (const auto &itr : offending_) {
            const auto &fi = itr.first;
            const auto P = triangle_faces_[fi].triangle.supporting_plane();
            assert(!P.is_degenerate());
            for (const auto &jtr : itr.second) {
                const auto &fj = jtr.first;
                const auto &tj = triangle_faces_[fj];
                if (P.has_on(tj.triangle[0]) && P.has_on(tj.triangle[1]) && P.has_on(tj.triangle[2])) {
                    auto loc = intersecting_and_coplanar.find(fi);
                    if (loc == intersecting_and_coplanar.end())
                        intersecting_and_coplanar[fi] = {fj};
                    else
                        loc->second.push_back(fj);
                }
            }
        }
#ifdef REMESH_INTERSECTIONS_TIMING
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "preprocess... ";
#endif

        std::vector<std::vector<int> > resolved_faces;
        std::vector<int> source_faces;
        std::vector<Point_3> new_vertices;
        EdgeMap edge_vertices;
        // face_vertices: Given a face int, find vertices inside the face
        std::unordered_map<int, std::vector<int>> face_vertices;

        // Run constraint Delaunay triangulation on the plane.
        //
        // Inputs:
        //   P  plane to triangulate upone
        //   involved_faces  #F list of indices into triangle of involved faces
        // Outputs:
        //   vertices  #V list of vertex positions of output triangulation
        //   faces   #F list of face indices into vertices of output triangulation
        //
        auto delaunay_triangulation = [&](
                const Plane_3 &P,
                const std::vector<int> &involved_faces,
                std::vector<Point_3> &vertices,
                std::vector<std::vector<int> > &faces) -> void {
            std::vector<CGAL::Object> objects;

            CDT_plus_2 cdt;
            // insert each face into a common cdt
            for (const auto &fid : involved_faces) {
                const auto &itr = offending_.find(fid);
                const auto &tri = triangle_faces_[fid];
                objects.push_back(CGAL::make_object(tri.triangle));
                if (itr == offending_.end())
                    continue;

                for (const auto &index_obj : itr->second) {
                    //const auto& ofid = index_obj.first;
                    const auto &obj = index_obj.second;
                    objects.push_back(obj);
                }
            }
            projected_cdt(objects, P, vertices, faces);
        };

        // Given p on triangle indexed by ori_f, add point to list of vertices return index of p.
        //
        // Input:
        //   p  point to search for
        //   ori_f  index of triangle p is corner of
        // Returns global index of vertex (dependent on whether stitch_all flag is
        // set)
        //
        auto find_or_append_point = [&](const Point_3 &p, const std::size_t ori_f) -> int {
            if (!stitch) {
                // No need to check if p shared by multiple triangles because all shared
                // vertices would be merged later on.
                const int index = num_base_vertices + new_vertices.size();
                new_vertices.push_back(p);
                return index;
            } else {
                // Stitching triangles according to input connectivity.
                // This step is potentially costly.
                const auto &tri = triangle_faces_[ori_f];
                // Check if p is one of the triangle corners.
                for (unsigned short i = 0; i < 3; i++) {
                    if (p == tri.triangle[i]) {
                        SurfaceMesh::Vertex v = tri.vertices[i];
                        return v.idx();
                    }
                }

                // Check if p is on one of the edges.
                for (unsigned short i = 0; i < 3; i++) {
                    const Point_3 curr_corner = tri.triangle[i];
                    const Point_3 next_corner = tri.triangle[(i + 1) % 3];
                    const Segment_3 edge(curr_corner, next_corner);
                    if (edge.has_on(p)) {
                        SurfaceMesh::Vertex v_curr = tri.vertices[i];
                        SurfaceMesh::Vertex v_next = tri.vertices[(i + 1) % 3];

                        const int curr = v_curr.idx();
                        const int next = v_next.idx();
                        Edge key;
                        key.first = curr < next ? curr : next;
                        key.second = curr < next ? next : curr;
                        auto itr = edge_vertices.find(key);
                        if (itr == edge_vertices.end()) {
                            const int index = num_base_vertices + new_vertices.size();
                            edge_vertices.insert({key, {index}});
                            new_vertices.push_back(p);
                            return index;
                        } else {
                            for (const auto vid : itr->second) {
                                if (p == new_vertices[vid - num_base_vertices]) {
                                    return vid;
                                }
                            }
                            const int index = num_base_vertices + new_vertices.size();
                            new_vertices.push_back(p);
                            itr->second.push_back(index);
                            return index;
                        }
                    }
                }

                // p must be in the middle of the triangle.
                auto &existing_face_vertices = face_vertices[ori_f];
                for (const auto vid : existing_face_vertices) {
                    if (p == new_vertices[vid - num_base_vertices]) {
                        return vid;
                    }
                }
                const int index = num_base_vertices + new_vertices.size();
                new_vertices.push_back(p);
                existing_face_vertices.push_back(index);
                return index;
            }
        };

        // Determine the vertex indices for each corner of each output triangle.
        //
        // Inputs:
        //   vertices  #V list of vertices of cdt
        //   faces  #F list of list of face indices into vertices of cdt
        //   involved_faces  list of involved faces on the plane of cdt
        // Side effects:
        //   - add faces to resolved_faces
        //   - add corresponding original face to source_faces
        //   -
        auto post_triangulation_process = [&](
                const std::vector<Point_3> &vertices,
                const std::vector<std::vector<int> > &faces,
                const std::vector<int> &involved_faces) -> void {
            assert(involved_faces.size() > 0);
            // for all faces of the cdt
            for (const auto &f : faces) {
                const Point_3 &v0 = vertices[f[0]];
                const Point_3 &v1 = vertices[f[1]];
                const Point_3 &v2 = vertices[f[2]];
                Point_3 center(
                        (v0[0] + v1[0] + v2[0]) / 3.0,
                        (v0[1] + v1[1] + v2[1]) / 3.0,
                        (v0[2] + v1[2] + v2[2]) / 3.0);
                if (involved_faces.size() == 1) {
                    // If only there is only one involved face, all sub-triangles must
                    // belong to it and have the correct orientation.
                    const auto &ori_f = involved_faces[0];
                    std::vector<int> corners(3);
                    corners[0] = find_or_append_point(v0, ori_f);
                    corners[1] = find_or_append_point(v1, ori_f);
                    corners[2] = find_or_append_point(v2, ori_f);
                    resolved_faces.emplace_back(corners);
                    source_faces.push_back(ori_f);
                } else {
                    for (const auto &ori_f : involved_faces) {
                        const auto &tri = triangle_faces_[ori_f];
                        const Plane_3 P = tri.triangle.supporting_plane();
                        if (tri.triangle.has_on(center)) {
                            std::vector<int> corners(3);
                            corners[0] = find_or_append_point(v0, ori_f);
                            corners[1] = find_or_append_point(v1, ori_f);
                            corners[2] = find_or_append_point(v2, ori_f);
                            if (CGAL::orientation(P.to_2d(v0), P.to_2d(v1), P.to_2d(v2)) == CGAL::RIGHT_TURN) {
                                std::swap(corners[0], corners[1]);
                            }
                            resolved_faces.emplace_back(corners);
                            source_faces.push_back(ori_f);
                        }
                    }
                }
            }
        };

        // Process un-touched faces.
        for (auto f : mesh->faces()) {
            const int fid = f.idx();
            if (!is_offending[fid] && !triangle_faces_[fid].triangle.is_degenerate()) {
                SurfaceMesh::Face f = triangle_faces_[fid].face;
                std::vector<int> ids;
                for (auto v : mesh->vertices(f))
                    ids.push_back(v.idx());
                resolved_faces.push_back(ids);
                source_faces.push_back(fid);
            }
        }

        // Process self-intersecting faces.
        std::vector<bool> processed(num_faces, false);
        std::vector<std::pair<Plane_3, std::vector<int> > > cdt_inputs;
        for (const auto &itr : offending_) {
            const int fid = itr.first;
            if (processed[fid])
                continue;
            processed[fid] = true;

            const auto loc = intersecting_and_coplanar.find(fid);
            std::vector<int> involved_faces;
            if (loc == intersecting_and_coplanar.end()) {
                involved_faces.push_back(fid);
            } else {
                std::queue<int> Q;
                Q.push(fid);
                while (!Q.empty()) {
                    const auto index = Q.front();
                    involved_faces.push_back(index);
                    Q.pop();

                    const auto overlapping_faces = intersecting_and_coplanar.find(index);
                    assert(overlapping_faces != intersecting_and_coplanar.end());

                    for (const auto other_index : overlapping_faces->second) {
                        if (processed[other_index]) continue;
                        processed[other_index] = true;
                        Q.push(other_index);
                    }
                }
            }

            Plane_3 P = triangle_faces_[fid].triangle.supporting_plane();
            cdt_inputs.emplace_back(P, involved_faces);
        }

#ifdef REMESH_INTERSECTIONS_TIMING
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "CDT... ";
#endif

        const std::size_t num_cdts = cdt_inputs.size();
        std::vector<std::vector<Point_3> > cdt_vertices(num_cdts);
        std::vector<std::vector<std::vector<int> > > cdt_faces(num_cdts);

        //// Not clear whether this is safe because of reference counting on Point_3
        //// objects...
        ////
        //// I tried it and got random segfaults (via MATLAB). Seems this is not
        //// safe.
        //igl::parallel_for(num_cdts,[&](int i)
        for (std::size_t i = 0; i < num_cdts; i++) {
            auto &vertices = cdt_vertices[i];
            auto &faces = cdt_faces[i];
            const auto &P = cdt_inputs[i].first;
            const auto &involved_faces = cdt_inputs[i].second;
            delaunay_triangulation(P, involved_faces, vertices, faces);
        }
        //,1000);
#ifdef REMESH_INTERSECTIONS_TIMING
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "stitching... ";
#endif

        for (std::size_t i = 0; i < num_cdts; i++) {
            const auto &vertices = cdt_vertices[i];
            const auto &faces = cdt_faces[i];
            const auto &involved_faces = cdt_inputs[i].second;
            post_triangulation_process(vertices, faces, involved_faces);
        }
#ifdef REMESH_INTERSECTIONS_TIMING
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "store results... ";
#endif

        // Output resolved mesh.

        SurfaceMesh *result = new SurfaceMesh;
        std::vector<SurfaceMesh::Vertex> vertices;
        auto points = mesh->get_vertex_property<vec3>("v:point");
        for (auto p : mesh->vertices()) {
            SurfaceMesh::Vertex v = result->add_vertex(points[p]);
            vertices.push_back(v);
        }
        for (auto p : new_vertices) {
            SurfaceMesh::Vertex v = result->add_vertex(
                    vec3(CGAL::to_double(p[0]), CGAL::to_double(p[1]), CGAL::to_double(p[2])));
            vertices.push_back(v);
        }

        for (auto fvids : resolved_faces) {
            result->add_face({vertices[fvids[0]], vertices[fvids[1]], vertices[fvids[2]]});
        }

#ifdef REMESH_INTERSECTIONS_TIMING
        LOG(INFO) << "done. " << w.time_string();
#endif

        return result;
    }


} // namespace easy3d
