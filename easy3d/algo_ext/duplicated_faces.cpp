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

#include <easy3d/algo_ext/duplicated_faces.h>

#include <set>
#include <unordered_map>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {


    DuplicatedFaces::Triangles DuplicatedFaces::mesh_to_cgal_triangle_list(SurfaceMesh* mesh) {
        auto prop = mesh->get_vertex_property<vec3>("v:point");

        Triangles triangles;
        for (auto f : mesh->faces()) {
            std::vector< Point_3 > points;
            std::vector< SurfaceMesh::Vertex > vertices;
            for (auto v : mesh->vertices(f)) {
                const vec3& p = prop[v];
                points.push_back(Point_3(p.x, p.y, p.z));
                vertices.push_back(v);
            }

            if (points.size() == 3) {
                Triangle t(points[0], points[1], points[2], f);
                t.vertices = vertices;
                triangles.push_back(t);
            }
            else {
                LOG_FIRST_N(WARNING, 1) << "only triangular meshes can be processed (this is the first record)";
                return triangles;
            }
        }
        return triangles;
    }


    bool DuplicatedFaces::do_duplicate(const Triangle &A, const Triangle &B, bool exact, double sqr_eps)
    {
        // Number of combinatorially shared vertices
        int num_comb_shared_vertices = 0;

        // Number of geometrically shared vertices (*not* including combinatorially shared)
        int num_geom_shared_vertices = 0;

        // Keep track of shared vertex indices
        std::vector< std::pair<SurfaceMesh::Vertex, SurfaceMesh::Vertex> > shared;
        for (unsigned short ea = 0; ea < 3; ++ea) {
            for (unsigned short eb = 0; eb < 3; ++eb) {
                if (A.vertices[ea] == B.vertices[eb]) {
                    ++num_comb_shared_vertices;
                    shared.emplace_back(ea, eb);
                }
                else if (exact) {
                    if (A.triangle.vertex(ea) == B.triangle.vertex(eb)) {
                        ++num_geom_shared_vertices;
                        shared.emplace_back(ea, eb);
                    }
                }
                else {  // use distance threshold
                    if (CGAL::squared_distance(A.triangle.vertex(ea), B.triangle.vertex(eb)) < sqr_eps) {
                        ++num_geom_shared_vertices;
                        shared.emplace_back(ea, eb);
                    }
                }
            }
        }
        const int total_shared_vertices = num_comb_shared_vertices + num_geom_shared_vertices;
        if (num_comb_shared_vertices == 3) {
            assert(shared.size() == 3);		// Combinatorially duplicated faces
            return true;
        }
        if (total_shared_vertices == 3) {
            assert(shared.size() == 3);		// Geometrically duplicate faces.
            return true;
        }

        return false;
    }


    std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
    DuplicatedFaces::detect(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > > result;
        if (!mesh)
            return result;

        if (!mesh->is_triangle_mesh()) {
            mesh->triangulate();
            LOG(WARNING) << "input mesh triangulated to perform duplication detection";
        }

        triangle_faces_ = mesh_to_cgal_triangle_list(mesh);

        // bounding boxes of the triangles
        std::vector<Box> boxes;
        for (TrianglesIterator it = triangle_faces_.begin(); it != triangle_faces_.end(); ++it) {
            if (!it->triangle.is_degenerate())
                boxes.push_back(Box(it->triangle.bbox(), it));
        }
        std::vector< std::pair<TrianglesIterator, TrianglesIterator> > intersecting_boxes;
        auto cb = [&intersecting_boxes](const Box &a, const Box &b) {
            intersecting_boxes.push_back({ a.handle(), b.handle() });
        };
        CGAL::box_self_intersection_d(boxes.begin(), boxes.end(), cb);

        std::unordered_map< SurfaceMesh::Face, std::set<SurfaceMesh::Face>, SurfaceMesh::Face::Hash> duplicated_faces;
        double sqr_eps = dist_threshold * dist_threshold;
        for (const auto& b : intersecting_boxes) {
            const Triangle& ta = *b.first;
            const Triangle& tb = *b.second;
            if (do_duplicate(ta, tb, exact, sqr_eps)) {
                duplicated_faces[ta.face].insert(tb.face);
                duplicated_faces[tb.face].insert(ta.face);
            }
        }

        // collect the result in the requested format
        result.resize(duplicated_faces.size());
        std::size_t idx = 0;
        for (const auto& elem : duplicated_faces) {
            result[idx].first = elem.first;
            const auto& faces = elem.second;
            result[idx].second = std::vector<SurfaceMesh::Face>(faces.begin(), faces.end());
            ++idx;
        }

        return result;
    }


    unsigned int DuplicatedFaces::remove(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        const auto& duplicated_faces = detect(mesh, exact, dist_threshold);
        if (duplicated_faces.empty())
            return 0;

        unsigned int prev_num_faces = mesh->n_faces();

        // in each duplication set, we keep only one of the duplicated faces
        // we create a new property to mark if the face should be deleted.
        auto deleted = mesh->get_face_property<bool>("f:deleted");

    //#if 0	// randomly keep one in a duplication set
        for (const auto& entry : duplicated_faces) {
            SurfaceMesh::Face face = entry.first;
            if (deleted[face]) // this duplication set has been processed
                continue;
            // keep face, delete the duplicated ones
            for (auto f : entry.second) {
                if (!deleted[f]) {
                    if (f != face)
                        mesh->delete_face(f);
                    else
                        LOG(ERROR) << "a face was marked duplicated with it self";
                }
            }
        }

    //#else
    //	// faces from smaller connected components will be deleted first. This way I try to keep the
    //	// connected components' original orientation.

    //	MeshComponentsExtractor extractor;
    //	const MeshComponentList& components = extractor.extract_components(m);
    //	MeshFacetAttribute<const MeshComponent*> face_comp(m);
    //	for (std::size_t i = 0; i < components.size(); ++i) {
    //		const MeshComponent* comp = components[i];
    //		FOR_EACH_FACET_CONST(MeshComponent, comp, it)
    //			face_comp[it] = comp;
    //	}

    //	struct FaceAndComponent {
    //		FaceAndComponent(const Mesh::Facet* f, const MeshComponent* c) : face(f), comp(c) {}
    //		const Mesh::Facet*   face;
    //		const MeshComponent* comp;
    //	};

    //	struct CompareGreater {
    //		bool operator()(const FaceAndComponent& fc0, const FaceAndComponent& fc1) const {
    //			int size_fc0 = fc0.comp->size_of_facets();
    //			int size_fc1 = fc1.comp->size_of_facets();
    //			// in case the parent components have the same size, we choose to
    //			// delete those from the same component
    //			if (size_fc0 == size_fc1)
    //				return fc0.comp > fc1.comp;
    //			else
    //				return size_fc0 > size_fc1;
    //		}
    //	};

    //	// now for each duplication set, we keep the one from the largest component
    //	for (const auto& entry : duplicated_faces) {
    //		bool had_processed = 0;
    //		std::vector<FaceAndComponent> duplication_set;
    //		Mesh::Facet* face = entry.first;
    //		if (deleted[face]) {
    //			had_processed = true;
    //			continue;
    //		}
    //		duplication_set.push_back(FaceAndComponent(face, face_comp[face]));
    //		for (auto f : entry.second) {
    //			if (deleted[f]) {
    //				had_processed = true;
    //				break;
    //			}
    //			if (f != entry.first) {
    //				duplication_set.push_back(FaceAndComponent(f, face_comp[f]));
    //			}
    //			else {
    //				LOG(ERROR) << "error: a face was marked duplicated with it self";
    //			}
    //		}
    //		if (had_processed)
    //			continue;

    //		std::sort(duplication_set.begin(), duplication_set.end(), CompareGreater());
    //		// keep the first one (which belong to the largest component)
    //		for (std::size_t i = 1; i < duplication_set.size(); ++i) {
    //			deleted[duplication_set[i].face] = true;
    //		}
    //	}

    //	face_comp.unbind();

    //#endif

        mesh->garbage_collection();

        return prev_num_faces - mesh->n_faces();
    }


}
