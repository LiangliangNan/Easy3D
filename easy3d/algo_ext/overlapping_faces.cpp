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

#include <easy3d/algo_ext/overlapping_faces.h>

#include <set>
#include <unordered_map>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    OverlappingFaces::Triangles OverlappingFaces::mesh_to_cgal_triangle_list(SurfaceMesh* mesh) {
        auto prop = mesh->get_vertex_property<vec3>("v:point");

        Triangles triangles;
        for (auto f : mesh->faces()) {
            std::vector< Point_3 > points;
            std::vector< SurfaceMesh::Vertex > vertices;
            for (auto v : mesh->vertices(f)) {
                const vec3& p = prop[v];
                points.emplace_back(Point_3(p.x, p.y, p.z));
                vertices.push_back(v);
            }

            if (points.size() == 3) {
                Triangle t(points[0], points[1], points[2], f);
                t.vertices = vertices;
                triangles.push_back(t);
            }
            else {
                LOG_N_TIMES(3, WARNING) << "only triangular meshes can be processed. " << COUNTER;
                return triangles;
            }
        }
        return triangles;
    }


    OverlappingFaces::OverlapType OverlappingFaces::do_overlap(const Triangle &A, const Triangle &B, double sqr_eps) {
        // Number of combinatorially shared vertices
        int num_comb_shared_vertices = 0;

        // Number of geometrically shared vertices (*not* including combinatorially shared)
        int num_geom_shared_vertices = 0;

        // Keep track of shared vertex indices
        std::vector< std::pair<unsigned short, unsigned short> > shared;

        for (unsigned short ea = 0; ea < 3; ++ea) {
            for (unsigned short eb = 0; eb < 3; ++eb) {
                if (A.vertices[ea] == B.vertices[eb]) {
                    ++num_comb_shared_vertices;
                    shared.emplace_back(ea, eb);
                }
                else if (CGAL::squared_distance(A.triangle.vertex(ea), B.triangle.vertex(eb)) < sqr_eps) {
                    ++num_geom_shared_vertices;
                    shared.emplace_back(ea, eb);
                }
            }
        }
        const int total_shared_vertices = num_comb_shared_vertices + num_geom_shared_vertices;
        if (num_comb_shared_vertices == 3) {
            assert(shared.size() == 3);		// Combinatorially duplicate faces
            return OT_SAME;
        }
        if (total_shared_vertices == 3) {
            assert(shared.size() == 3);		// Geometrically duplicate faces.
            return OT_SAME;
        }

#if 1   // Liangliang: let's handle coplanar and partial overlap
        if (total_shared_vertices == 2) {
            //
            //  2  o    o 3
            //     |\  /|
            //     | \/ |
            //     | /\ |
            //     |/  \|
            //  0  o----o 1
            //
            auto normalize = [](Vector_3& n) -> void { n /= std::sqrt(n.squared_length()); };
            auto na = A.triangle.supporting_plane().orthogonal_vector();
            auto nb = B.triangle.supporting_plane().orthogonal_vector();
            normalize(na);
            normalize(nb);
            auto error = std::abs(std::abs(na * nb) - 1.0);
            if (error < 1e-6) {    // considered coplanar
                const auto plane = A.triangle.supporting_plane();

                unsigned short A_v0 = shared[0].first;
                auto v0 = plane.to_2d(A.triangle.vertex(A_v0));
                unsigned short A_v1 = shared[1].first;
                auto v1 = plane.to_2d(A.triangle.vertex(A_v1));

                // the index of the detached vertex in A (6 = 0 + 1 + 2)
                unsigned short A_v2 = 3 - shared[0].first - shared[1].first;
                auto v2 = plane.to_2d(A.triangle.vertex(A_v2));
                // the index of the detached vertex in B
                unsigned short B_v3 = 3 - shared[0].second - shared[1].second;
                auto v3 = plane.to_2d(B.triangle.vertex(B_v3));

                // v2 and v3 on the same side of line v0-v1: the two triangles partially overlap
                if (CGAL::orientation(v0, v1, v2) == CGAL::orientation(v0, v1, v3)) {
//                    std::cout << A.triangle << std::endl;
//                    std::cout << B.triangle << std::endl;
                    return OT_FOLDING;
                }
            }
        }
#endif
        return OT_NONE;
    }


    void OverlappingFaces::detect(
            SurfaceMesh *mesh,
            std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > &duplicate_faces,
            std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > &folding_faces,
            double dist_threshold)
    {
        if (!mesh)
            return;

        if (!mesh->is_triangle_mesh()) {
            mesh->triangulate();
            LOG(WARNING) << "input mesh triangulated to perform duplication detection";
        }

        duplicate_faces.clear();
        folding_faces.clear();

        triangle_faces_ = mesh_to_cgal_triangle_list(mesh);

        // bounding boxes of the triangles
        std::vector<Box> boxes;
        for (auto it = triangle_faces_.begin(); it != triangle_faces_.end(); ++it) {
            if (!it->triangle.is_degenerate())
                boxes.emplace_back(Box(it->triangle.bbox(), it));
        }
        std::vector< std::pair<TrianglesIterator, TrianglesIterator> > intersecting_boxes;
        auto cb = [&intersecting_boxes](const Box &a, const Box &b) {
            intersecting_boxes.emplace_back(std::make_pair(a.handle(), b.handle()));
        };
        CGAL::box_self_intersection_d(boxes.begin(), boxes.end(), cb);

        double sqr_eps = dist_threshold * dist_threshold;
        for (const auto& b : intersecting_boxes) {
            const Triangle& ta = *b.first;
            const Triangle& tb = *b.second;

            auto type = do_overlap(ta, tb, sqr_eps);
            if (type == OT_SAME)
                duplicate_faces.emplace_back(std::make_pair(ta.face, tb.face));
            else if (type == OT_FOLDING)
                folding_faces.emplace_back(std::make_pair(ta.face, tb.face));
        }
    }


    unsigned int OverlappingFaces::remove(SurfaceMesh *input, bool delete_folding_faces, double dist_threshold)
    {
        // remove faces may leave the mesh in an invalid state (e.g., non-manifold)
        // so we use the manifold builder to construct the final mesh.

        SurfaceMesh mesh = *input;

        std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > duplicate_faces;
        std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > folding_faces;
        detect(&mesh, duplicate_faces, folding_faces, dist_threshold);

        if (duplicate_faces.empty() && folding_faces.empty())
            return 0;

        unsigned int prev_num_faces = mesh.n_faces();

        // in each duplication set, we keep only one of the duplicate faces
        auto deleted = mesh.face_property<bool>("f:deleted", false);

#if 1
        // for each duplication set, keep one face and delete all its duplications
        for (const auto& entry : duplicate_faces) {
            if (deleted[entry.first]) // this duplication set has been processed
                continue;
            // delete the duplicated one
            mesh.delete_face(entry.second);
        }

        for (const auto& entry : folding_faces) {
            if (deleted[entry.first]) // this duplication set has been processed
                continue;
            // delete the duplicated one
            mesh.delete_face(entry.second);
        }

        LOG(INFO) << duplicate_faces.size() << " pairs of duplicate faces, " << folding_faces.size()
                  << " pairs of folding faces";

#else   // just for debugging: visualizing the folding faces

        auto to_delete = mesh->add_face_property<bool>("f:remain", true);
        for (const auto &entry : folding_faces) {
            to_delete[entry.first] = false;
            to_delete[entry.second] = false;
        }

        for (auto f : mesh->faces()) {
            if (to_delete[f]) {
                mesh->delete_face(f);
            }
        }
#endif
        mesh.collect_garbage();

        // construct the new mesh
        input->clear();
        auto points = mesh.get_vertex_property<vec3>("v:point");

        SurfaceMeshBuilder builder(input);
        builder.begin_surface();

        for (auto v : mesh.vertices())
            builder.add_vertex(points[v]);

        for (auto f : mesh.faces()) {
            std::vector<SurfaceMesh::Vertex> vts;
            for (auto v : mesh.vertices(f))
                vts.push_back(v);
            builder.add_face(vts);
        }

        builder.end_surface(false);

        return prev_num_faces - input->n_faces();
    }

}
