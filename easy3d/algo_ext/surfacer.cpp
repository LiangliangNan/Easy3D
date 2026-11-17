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

#include <easy3d/algo_ext/surfacer.h>
#include <easy3d/algo_ext/overlapping_faces.h>
#include <easy3d/algo_ext/self_intersection.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/surface_mesh_builder.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/clip.h>
#include <CGAL/Polygon_mesh_processing/repair_degeneracies.h>
#include <CGAL/Polygon_mesh_slicer.h>

/// ToDo: useCGAL implementation: #include <CGAL/Polygon_mesh_processing/self_intersections.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3>                      CGALMesh;
typedef K::Plane_3                                          Plane_3;


namespace easy3d {

    namespace internal {

        void to_cgal(SurfaceMesh *input, CGALMesh &output) {
            if (input->has_garbage())
                input->collect_garbage();

            output.clear();

            for (auto v : input->vertices()) {
                const vec3 &p = input->position(v);
                output.add_vertex(K::Point_3(p.x, p.y, p.z));
            }

            for (auto f : input->faces()) {
                std::vector<CGALMesh::Vertex_index> vts;
                for (auto v : input->vertices(f))
                    vts.emplace_back(CGALMesh::Vertex_index(v.idx()));
                output.add_face(vts);
            }
        }

        /// The faces of every connected component of the mesh, as a face list each.
        std::vector<std::vector<SurfaceMesh::Face> > connected_components(SurfaceMesh *mesh) {
            if (mesh->has_garbage())
                mesh->collect_garbage();

            std::vector<std::vector<SurfaceMesh::Face> > components;
            std::vector<char> taken(mesh->n_faces(), 0);
            std::vector<SurfaceMesh::Face> pending;
            for (auto f : mesh->faces()) {
                if (taken[f.idx()])
                    continue;
                components.emplace_back();
                pending.push_back(f);
                taken[f.idx()] = 1;
                while (!pending.empty()) {
                    const auto current = pending.back();
                    pending.pop_back();
                    components.back().push_back(current);
                    const auto first = mesh->halfedge(current);
                    auto h = first;
                    do {
                        const auto neighbor = mesh->face(mesh->opposite(h));
                        h = mesh->next(h);
                        if (neighbor.is_valid() && !taken[neighbor.idx()]) {
                            taken[neighbor.idx()] = 1;
                            pending.push_back(neighbor);
                        }
                    } while (h != first);
                }
            }
            return components;
        }

        void to_cgal(SurfaceMesh *input, const std::vector<SurfaceMesh::Face> &faces, CGALMesh &output) {
            if (input->has_garbage())
                input->collect_garbage();

            output.clear();

            // Only the vertices the faces use, so that the part handed to corefinement is just that
            // part and nothing else.
            std::vector<CGALMesh::Vertex_index> map(input->n_vertices());
            for (auto f : faces) {
                std::vector<CGALMesh::Vertex_index> vts;
                for (auto v : input->vertices(f)) {
                    if (!map[v.idx()].is_valid()) {
                        const vec3 &p = input->position(v);
                        map[v.idx()] = output.add_vertex(K::Point_3(p.x, p.y, p.z));
                    }
                    vts.emplace_back(map[v.idx()]);
                }
                output.add_face(vts);
            }
        }

        void to_easy3d(CGALMesh &input, SurfaceMesh *output) {
            if (input.has_garbage())
                input.collect_garbage();

            output->clear();
            for (auto v : input.vertices()) {
                const K::Point_3 &p = input.point(v);
                output->add_vertex({static_cast<float>(CGAL::to_double(p.x())),
                                    static_cast<float>(CGAL::to_double(p.y())),
                                    static_cast<float>(CGAL::to_double(p.z()))});
            }

            for (auto f : input.faces()) {
                std::vector<SurfaceMesh::Vertex> vts;
                auto h = input.halfedge(f);
                for (auto v : input.vertices_around_face(h))
                    vts.emplace_back(SurfaceMesh::Vertex(static_cast<int>(v.idx())));
                output->add_face(vts);
            }
        }


        /// Stacks a mesh on the end of another one. The parts cut below share nothing, so putting
        /// them one after the other is the whole result.
        void append(CGALMesh &input, CGALMesh *output) {
            if (input.has_garbage())
                input.collect_garbage();

            std::vector<CGALMesh::Vertex_index> map(input.number_of_vertices());
            for (auto v : input.vertices())
                map[v.idx()] = output->add_vertex(input.point(v));

            for (auto f : input.faces()) {
                std::vector<CGALMesh::Vertex_index> vts;
                auto h = input.halfedge(f);
                for (auto v : input.vertices_around_face(h))
                    vts.emplace_back(map[v.idx()]);
                output->add_face(vts);
            }
        }


        // Drops the faces that carry no area: two vertices at the same location, or three collinear
        // ones. Corefinement cannot tolerate them - it asks an exact side-of-triangle predicate about
        // the vertices it inserts, and such a face answers COLLINEAR in every coordinate projection,
        // which CGAL asserts on (predicates/kernel_ftC3.h).
        std::size_t drop_degenerate_faces(CGALMesh *mesh) {
            std::size_t dropped = 0;
            std::vector<CGALMesh::Face_index> degenerate;
            // Removing a face can leave a neighbour degenerate, so scan until nothing is found.
            // Every round drops at least one face, so the scan terminates.
            for (;;) {
                degenerate.clear();
                CGAL::Polygon_mesh_processing::degenerate_faces(*mesh, std::back_inserter(degenerate));
                if (degenerate.empty())
                    break;
                for (auto f : degenerate)
                    mesh->remove_face(f);
                dropped += degenerate.size();
                mesh->collect_garbage();
            }
            return dropped;
        }


        // The same for a mesh of Easy3D's own, where the test is the one the surface mesh reports.
        // Results of a corefinement come back through float coordinates, and two vertices that were
        // distinct doubles can well land on the same float, so this is worth doing after conversion
        // and not only before it: it is what keeps a plane operation from handing the next one - or
        // the next render, or any other algorithm - faces it will call degenerate.
        std::size_t drop_degenerate_faces(SurfaceMesh *mesh) {
            std::vector<SurfaceMesh::Face> degenerate;
            for (auto f : mesh->faces()) {
                if (mesh->is_degenerate(f))
                    degenerate.push_back(f);
            }
            for (auto f : degenerate)
                mesh->delete_face(f);
            if (!degenerate.empty())
                mesh->collect_garbage();
            return degenerate.size();
        }


        /// Distance from the plane, positive on the side its normal points to.
        float distance_to(const Plane3 &plane, const vec3 &p) {
            const float norm = vec3(plane.a(), plane.b(), plane.c()).length();
            return (plane.a() * p.x + plane.b() * p.y + plane.c() * p.z + plane.d()) / norm;
        }


        /// How far a vertex may sit from the plane and still count as lying on it. A mesh that an
        /// earlier plane operation cut already has border vertices on the plane, and writing them
        /// out as floats leaves an error of this order rather than an exact zero.
        float plane_slack(const SurfaceMesh *mesh) {
            const vec3 extent =
                    mesh->bounding_box().max_point() - mesh->bounding_box().min_point();
            return 1e-6f * extent.length();
        }


        /// Whether some triangle has vertices on both sides of the plane. Without one of them the
        /// plane cannot cut anything new, and the mesh must not be given to corefinement: the
        /// splitter then runs along border edges the mesh already has, and the intersection nodes
        /// collected on a constrained edge are no longer distinct points, which CGAL asserts on.
        bool crosses_a_triangle(const SurfaceMesh *mesh, const Plane3 &plane, float slack) {
            for (auto f : mesh->faces()) {
                bool negative = false, positive = false;
                for (auto v : mesh->vertices(f)) {
                    const float distance = distance_to(plane, mesh->position(v));
                    positive = positive || distance > slack;
                    negative = negative || distance < -slack;
                }
                if (positive && negative)
                    return true;
            }
            return false;
        }


        void to_cgal(const std::vector<vec3> &input_points,
                     const std::vector<Surfacer::Polygon> &input_polygons,
                     std::vector<K::Point_3> &points,
                     std::vector<std::vector<std::size_t> > &polygons) {
            points.resize(input_points.size());
            polygons.resize(input_polygons.size());

            for (std::size_t i = 0; i < input_points.size(); ++i) {
                const vec3 &p = input_points[i];
                points[i] = K::Point_3(p.x, p.y, p.z);
            }

            for (std::size_t i = 0; i < input_polygons.size(); ++i) {
                const auto &input_plg = input_polygons[i];
                auto &plg = polygons[i];
                plg.resize(input_plg.size());
                for (std::size_t j = 0; j < input_plg.size(); ++j)
                    plg[j] = input_plg[j];
            }
        }

        void to_easy3d(const std::vector<K::Point_3> &input_points,
                       const std::vector<std::vector<std::size_t> > &input_polygons,
                       std::vector<vec3> &points,
                       std::vector<Surfacer::Polygon> &polygons) {
            points.resize(input_points.size());
            polygons.resize(input_polygons.size());

            for (std::size_t i = 0; i < input_points.size(); ++i) {
                const auto &p = input_points[i];
                points[i] = vec3({static_cast<float>(CGAL::to_double(p.x())),
                                  static_cast<float>(CGAL::to_double(p.y())),
                                  static_cast<float>(CGAL::to_double(p.z()))});
            }

            for (std::size_t i = 0; i < input_polygons.size(); ++i) {
                const auto &input_plg = input_polygons[i];
                auto &plg = polygons[i];
                plg.resize(input_plg.size());
                for (std::size_t j = 0; j < input_plg.size(); ++j)
                    plg[j] = static_cast<int>(input_plg[j]);
            }
        }


        void remove_duplicate_vertices(std::vector<vec3> &points,
                                       std::vector<Surfacer::Polygon> &polygons) {
            // helper class for identifying coincident vertices
            class CompareVec {
            public:
                explicit CompareVec(float _eps = FLT_MIN) : eps_(_eps) {}

                bool operator()(const vec3 &v0, const vec3 &v1) const {
                    if (fabs(v0[0] - v1[0]) <= eps_) {
                        if (fabs(v0[1] - v1[1]) <= eps_)
                            return (v0[2] < v1[2] - eps_);
                        else
                            return (v0[1] < v1[1] - eps_);
                    } else
                        return (v0[0] < v1[0] - eps_);
                }

            private:
                float eps_;
            };

            CompareVec comp(FLT_MIN);
            std::map<vec3, int, CompareVec> vMap(comp);
            std::map<vec3, int, CompareVec>::iterator vMapIt;
            std::map<int, int> index_map;

            std::vector<vec3> result_points = points;
            std::vector<Surfacer::Polygon> input_polygons = polygons;
            points.clear();
            for (auto &plg : polygons)
                plg.clear();

            for (std::size_t i = 0; i < result_points.size(); ++i) {
                const vec3 &p = result_points[i];

                // has the point been referenced before?
                vMapIt = vMap.find(p);
                if (vMapIt == vMap.end()) {
                    // No : add vertex and remember idx/vector mapping
                    vMap[p] = static_cast<int>(points.size());
                    index_map[static_cast<int>(i)] = static_cast<int>(points.size());
                    points.emplace_back(vec3(p.x, p.y, p.z));
                } else {
                    // Yes : get index from map
                    index_map[static_cast<int>(i)] = static_cast<int>(vMapIt->second);
                }
            }
            for (std::size_t i = 0; i < input_polygons.size(); ++i) {
                for (auto id : input_polygons[i])
                    polygons[i].push_back(index_map[id]);
            }
        }


        void to_polygon_soup(SurfaceMesh *mesh, std::vector<vec3> &points,
                             std::vector<Surfacer::Polygon> &polygons) {
            if (!mesh)
                return;

            points = mesh->points();
            polygons.resize(mesh->n_faces());
            for (auto f : mesh->faces()) {
                auto &plg = polygons[f.idx()];
                for (auto v : mesh->vertices(f))
                    plg.push_back(v.idx());
            }
        }

        void to_polygon_mesh(const std::vector<vec3> &points,
                             const std::vector<Surfacer::Polygon> &polygons,
                             SurfaceMesh *mesh) {
            mesh->clear();

            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();

            for (auto p : points)
                builder.add_vertex(p);

            for (const auto &plg : polygons) {
                std::vector<SurfaceMesh::Vertex> vts;
                for (auto v : plg)
                    vts.emplace_back(SurfaceMesh::Vertex(v));
                builder.add_face(vts);
            }
            builder.end_surface(false);
        }
    }



    int Surfacer::stitch_borders(SurfaceMesh *input) {
        CGALMesh mesh;
        internal::to_cgal(input, mesh);
        auto num = CGAL::Polygon_mesh_processing::stitch_borders(mesh);
        internal::to_easy3d(mesh, input);
        return static_cast<int>(num);
    }


    bool Surfacer::orient_and_stitch_polygon_soup(SurfaceMesh *mesh) {
        if (!mesh)
            return false;

        std::vector<vec3> points;
        std::vector<Polygon> polygons;
        internal::to_polygon_soup(mesh, points, polygons);

        bool status = orient_and_stitch_polygon_soup(points, polygons);
        internal::to_polygon_mesh(points, polygons, mesh);

        return status;
    }


    void Surfacer::merge_reversible_connected_components(SurfaceMesh *input) {
        CGALMesh mesh;
        internal::to_cgal(input, mesh);

        typedef boost::property_map<CGALMesh, CGAL::dynamic_face_property_t<std::size_t> >::type Fccmap;
        Fccmap fccmap = CGAL::get(CGAL::dynamic_face_property_t<std::size_t>(), mesh);
        auto count_prev = CGAL::Polygon_mesh_processing::connected_components(mesh, fccmap);
        if (count_prev <= 1) {
            return; // nothing to be merged
        }

        // iteratively merge the connected components until the no more connected components can be merged
        do {
            CGAL::Polygon_mesh_processing::merge_reversible_connected_components(mesh);
            auto count_now = CGAL::Polygon_mesh_processing::connected_components(mesh, fccmap);
            if (count_now == 1 || count_now == count_prev)
                break;
            count_prev = count_now;
        } while (true);

        internal::to_easy3d(mesh, input);
    }


    void Surfacer::orient_closed_triangle_mesh(SurfaceMesh* input_mesh) {
        if (!input_mesh->is_triangle_mesh() || !input_mesh->is_closed()) {
            LOG(WARNING) << "only closed triangle meshes can be oriented";
            return;
        }

        CGALMesh mesh;
        internal::to_cgal(input_mesh, mesh);
        CGAL::Polygon_mesh_processing::orient(mesh);
        internal::to_easy3d(mesh, input_mesh);
    }


    bool Surfacer::orient_and_stitch_polygon_soup(std::vector<vec3> &input_points, std::vector<Polygon> &input_polygons) {
        const auto num_vertices = input_points.size();
        const auto num_faces = input_polygons.size();

        // stitch
        internal::remove_duplicate_vertices(input_points, input_polygons);

        std::vector<K::Point_3> points;
        std::vector<std::vector<std::size_t> > polygons(input_polygons.size());
        internal::to_cgal(input_points, input_polygons, points, polygons);

        bool status = CGAL::Polygon_mesh_processing::orient_polygon_soup(points, polygons);
        if (!CGAL::Polygon_mesh_processing::is_polygon_soup_a_polygon_mesh(polygons)) {
            LOG(WARNING) << "the polygons after orientation do not define a valid polygon mesh";
            return false;
        }

        // convert to easy3d
        internal::to_easy3d(points, polygons, input_points, input_polygons);

        std::string msg;
        int diff_vertices = static_cast<int>(num_vertices - input_points.size());
        int diff_faces = static_cast<int>(num_faces - input_polygons.size());
        if (diff_vertices != 0) {
            msg += std::to_string(std::abs(diff_vertices)) + " vertices " +
                   (diff_vertices >= 0 ? "removed" : "inserted");
            if (diff_faces != 0)
                msg += " and ";
        }
        if (diff_faces != 0)
            msg += std::to_string(std::abs(diff_faces)) + " faces " + (diff_faces >= 0 ? "removed" : "inserted");
        if (!msg.empty())
            LOG(INFO) << msg;

        return status;
    }


    void Surfacer::repair_polygon_soup(std::vector<vec3> &input_points, std::vector<Polygon> &input_polygons) {
        const auto num_vertices = input_points.size();
        const auto num_faces = input_polygons.size();

        internal::remove_duplicate_vertices(input_points, input_polygons);

        std::vector<K::Point_3> points;
        std::vector<std::vector<std::size_t> > polygons(input_polygons.size());
        internal::to_cgal(input_points, input_polygons, points, polygons);

        CGAL::Polygon_mesh_processing::repair_polygon_soup(points, polygons);

        // let's also try to orient the polygon soup
        CGAL::Polygon_mesh_processing::orient_polygon_soup(points, polygons);
        if (!CGAL::Polygon_mesh_processing::is_polygon_soup_a_polygon_mesh(polygons)) {
            LOG(WARNING) << "the polygons after orientation do not define a valid polygon mesh";
            return;
        }

        internal::to_easy3d(points, polygons, input_points, input_polygons);

        int diff_vertices = static_cast<int>(num_vertices - input_points.size());
        int diff_faces = static_cast<int>(num_faces - input_polygons.size());
        if (diff_vertices + diff_faces != 0) {
            std::string msg;
            if (diff_vertices != 0) {
                msg += std::to_string(std::abs(diff_vertices)) + " vertices " +
                       (diff_vertices >= 0 ? "removed" : "inserted");
                if (diff_faces != 0)
                    msg += " and ";
            }
            if (diff_faces != 0)
                msg += std::to_string(std::abs(diff_faces)) + " faces " + (diff_faces >= 0 ? "removed" : "inserted");
            LOG(INFO) << msg;
        }
    }


    void Surfacer::repair_polygon_soup(SurfaceMesh *mesh) {
        if (!mesh)
            return;

        std::vector<vec3> points;
        std::vector<Polygon> polygons;
        internal::to_polygon_soup(mesh, points, polygons);

        repair_polygon_soup(points, polygons);

        // convert to easy3d
        internal::to_polygon_mesh(points, polygons, mesh);
    }


    bool Surfacer::clip(SurfaceMesh *input_mesh, const Plane3 &input_plane, bool clip_volume) {
        if (!input_mesh->is_triangle_mesh()) {
            LOG(WARNING) << "only triangle meshes can be clipped";
            return false;
        }

        // A plane that no triangle crosses cannot cut anything: it only runs along geometry the mesh
        // already has, which corefinement refuses. Clipping then reduces to the selection between the
        // parts that are already separated, and since no cut is made the result is as manifold as the
        // mesh was.
        const float slack = internal::plane_slack(input_mesh);
        if (!internal::crosses_a_triangle(input_mesh, input_plane, slack)) {
            std::vector<SurfaceMesh::Face> discard;
            for (auto f : input_mesh->faces()) {
                for (auto v : input_mesh->vertices(f)) {
                    if (internal::distance_to(input_plane, input_mesh->position(v)) > slack) {
                        discard.push_back(f);
                        break;
                    }
                }
            }
            for (auto f : discard)
                input_mesh->delete_face(f);
            input_mesh->collect_garbage();
            LOG(INFO) << "the plane does not cut the mesh: " << discard.size()
                      << " faces on its positive side removed";
            return true;
        }

        const Plane_3 plane(input_plane.a(), input_plane.b(), input_plane.c(), input_plane.d());
        CGALMesh cut;
        bool status = true;
        std::size_t dropped = 0;
        // One component at a time, because corefinement cannot take them together: an earlier cut
        // left the two sides of its opening at the same locations, and a plane crossing that opening
        // then meets the same point twice on one of its own constrained edges, which CGAL asserts on.
        // The components do not touch each other, so cutting them apart and putting the results back
        // is the same cut as over the whole mesh.
        for (const auto &component : internal::connected_components(input_mesh)) {
            CGALMesh part;
            internal::to_cgal(input_mesh, component, part);
            dropped += internal::drop_degenerate_faces(&part);
            try {
                status = status && CGAL::Polygon_mesh_processing::clip(
                        part,
                        plane,
                        CGAL::Polygon_mesh_processing::parameters::clip_volume(clip_volume)
                );
            } catch (const std::exception &e) {
                // CGAL raises both its assertion failures and the corefinement errors as exceptions,
                // and none of them leaves a usable result behind. Nothing has been written back yet,
                // so the caller's mesh is left exactly as it was.
                LOG(WARNING) << "clipping failed: " << e.what();
                return false;
            }
            internal::append(part, &cut);
        }

        if (dropped != 0)
            LOG(INFO) << dropped << " degenerate faces removed before clipping";

        internal::to_easy3d(cut, input_mesh);

        if (const std::size_t removed = internal::drop_degenerate_faces(input_mesh))
            LOG(INFO) << removed << " degenerate faces removed by clipping";

        return status;
    }


    void Surfacer::split(SurfaceMesh *input_mesh, const Plane3 &input_plane) {
        if (!input_mesh->is_triangle_mesh()) {
            LOG(WARNING) << "only triangle meshes can be split";
            return;
        }

        // See crosses_a_triangle(): a plane the mesh was already cut open along has nothing left to
        // split, and the splitter coinciding with existing border edges is a configuration
        // corefinement asserts on.
        if (!internal::crosses_a_triangle(input_mesh, input_plane, internal::plane_slack(input_mesh))) {
            LOG(INFO) << "the plane does not cut the mesh: nothing to split";
            return;
        }

        const Plane_3 plane(input_plane.a(), input_plane.b(), input_plane.c(), input_plane.d());
        CGALMesh cut;
        std::size_t dropped = 0;
        // One component at a time, for the reason given in clip(): the coincident borders an earlier
        // cut leaves behind are what corefinement trips over, and apart from each other they are
        // ordinary borders.
        for (const auto &component : internal::connected_components(input_mesh)) {
            CGALMesh part;
            internal::to_cgal(input_mesh, component, part);
            dropped += internal::drop_degenerate_faces(&part);
            try {
                CGAL::Polygon_mesh_processing::split(part, plane);
            } catch (const std::exception &e) {
                // Nothing has been written back yet, so the caller's mesh is left as it was.
                LOG(WARNING) << "splitting failed: " << e.what();
                return;
            }
            internal::append(part, &cut);
        }

        if (dropped != 0)
            LOG(INFO) << dropped << " degenerate faces removed before splitting";

        internal::to_easy3d(cut, input_mesh);

        if (const std::size_t removed = internal::drop_degenerate_faces(input_mesh))
            LOG(INFO) << removed << " degenerate faces removed by splitting";
    }


    std::vector<Surfacer::Polyline> Surfacer::slice(SurfaceMesh *input_mesh, const Plane3 &input_plane) {
        std::vector<Plane3> planes(1, input_plane);
        const std::vector< std::vector<Surfacer::Polyline> >& result = slice(input_mesh, planes);
        return result[0];
    }


    std::vector< std::vector<Surfacer::Polyline> > Surfacer::slice(SurfaceMesh *input_mesh, const std::vector<Plane3> &input_planes) {
        std::vector< std::vector<Surfacer::Polyline> > result;
        if (!input_mesh->is_triangle_mesh()) {
            LOG(WARNING) << "only triangle meshes can be clipped";
            return result;
        }

        typedef std::vector<K::Point_3> CGALPolyline;

        auto to_easy3d = [](std::vector<CGALPolyline>& input) -> std::vector<Surfacer::Polyline> {
            std::vector<Surfacer::Polyline> polylines(input.size());
            for (std::size_t i = 0; i < input.size(); ++i) {
                const CGALPolyline &pl = input[i];
                polylines[i].resize(pl.size());
                for (std::size_t j = 0; j < pl.size(); ++j) {
                    const auto &p = pl[j];
                    polylines[i][j] = vec3(
                            static_cast<float>(CGAL::to_double(p.x())),
                            static_cast<float>(CGAL::to_double(p.y())),
                            static_cast<float>(CGAL::to_double(p.z()))
                    );
                }
            }
            return polylines;
        };

        CGALMesh mesh;
        internal::to_cgal(input_mesh, mesh);
        // Slicer constructor from the mesh
        CGAL::Polygon_mesh_slicer<CGALMesh , K> slicer(mesh);

        std::vector< std::vector<CGALPolyline> > CGALResult(input_planes.size());
        for (const auto &input_plane : input_planes) {
            Plane_3 plane(input_plane.a(), input_plane.b(), input_plane.c(), input_plane.d());

            std::vector<CGALPolyline> lines;
            slicer(plane, std::back_inserter(lines));

            result.push_back(to_easy3d(lines));
        }

        return result;
    }


    unsigned int Surfacer::remove_degenerate_faces(SurfaceMesh *mesh, float length_threshold) {
        unsigned int num = mesh->n_faces();

        for (auto e : mesh->edges()) {
            if (mesh->edge_length(e) < length_threshold) {
                auto h = mesh->halfedge(e, 0);
                if (mesh->is_collapse_ok(h)) {
                    mesh->collapse(h);
                }
                else {
                    h = mesh->opposite(h);
                    if (mesh->is_collapse_ok(h)) {
                        mesh->collapse(h);
                    }
                }
            }
        }

        typedef CGAL::Simple_cartesian<double>  Kernel;
        typedef CGAL::Point_3<Kernel>           Point_3;
        typedef CGAL::Triangle_3<Kernel>        Triangle_3;

        auto prop = mesh->get_vertex_property<vec3>("v:point");

        auto has_tiny_edge = [](SurfaceMesh* m, SurfaceMesh::Face f, float thresh) -> bool {
            for (auto h : m->halfedges(f)) {
                if (m->edge_length(m->edge(h)) < thresh)
                    return true;
            }
            return false;
        };

        std::set<SurfaceMesh::Face> to_delete;
        for (auto f : mesh->faces()) {
            if (has_tiny_edge(mesh, f, length_threshold))
                to_delete.insert(f);
        }

        for (auto f : mesh->faces()) {
            std::vector<Point_3> points;
            for (auto v : mesh->vertices(f)) {
                const vec3 &p = prop[v];
                points.emplace_back(Point_3(p.x, p.y, p.z));
            }

            if (points.size() == 3) {
                const Triangle_3 t(points[0], points[1], points[2]);
                if (t.is_degenerate())
                    to_delete.insert(f);
            } else {
                LOG_N_TIMES(3, WARNING) << "only triangular meshes can be processed. " << COUNTER;
            }
        }

        for (auto f : to_delete)
            mesh->delete_face(f);

        mesh->collect_garbage();

        unsigned int diff = num - mesh->n_faces();
        if (diff > 0)
            LOG(INFO) << diff << " degenerate faces deleted";

        return diff;
    }


    void Surfacer::detect_overlapping_faces(
            SurfaceMesh *mesh,
            std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > &duplicate_faces,
            std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > &folding_faces,
            double dist_threshold)
    {
        easy3d::OverlappingFaces rdf;
        return rdf.detect(mesh, duplicate_faces, folding_faces, dist_threshold);
    }


    unsigned int Surfacer::remove_overlapping_faces(SurfaceMesh *mesh, double dist_threshold) {
        OverlappingFaces rdf;
        return rdf.remove(mesh, dist_threshold);
    }


    std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
    Surfacer::detect_self_intersections(SurfaceMesh *mesh) {
#if 1
        SelfIntersection msi;
        return msi.detect(mesh, false);
#else // this ca
        std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > result;
        if (!mesh->is_triangle_mesh()) {
            LOG(WARNING) << "only triangle meshes can be checked";
            return result;
        }

        CGALMesh cgal_mesh;
        internal::to_cgal(mesh, cgal_mesh);

        std::vector<CGALMesh::Face_index> degenerate_faces;
        CGAL::Polygon_mesh_processing::degenerate_faces(cgal_mesh, std::back_inserter(degenerate_faces));

        std::cout << "faces: " << cgal_mesh.number_of_faces() << std::endl;
        for (auto f : degenerate_faces)
            cgal_mesh.remove_face(f);
        cgal_mesh.collect_garbage();
        std::cout << "faces: " << cgal_mesh.number_of_faces() << std::endl;
        std::cout << degenerate_faces.size() << " degenerate_faces deleted" << std::endl;
        internal::to_easy3d(cgal_mesh, mesh);

        std::vector<std::pair<CGALMesh::Face_index, CGALMesh::Face_index> > intersected_tris;
        CGAL::Polygon_mesh_processing::self_intersections<CGAL::Parallel_if_available_tag>(CGAL::faces(cgal_mesh), cgal_mesh, std::back_inserter(intersected_tris));

        result.resize(intersected_tris.size());
        for (std::size_t i=0; i<intersected_tris.size(); ++i) {
            const auto& face_pair = intersected_tris[i];
            result[i] = std::make_pair(
                    SurfaceMesh::Face(face_pair.first),
                    SurfaceMesh::Face(face_pair.second)
            );
        }
        return result;
    #endif
    }


    bool Surfacer::remesh_self_intersections(SurfaceMesh *mesh, bool stitch /* = true */) {
        SelfIntersection msi;
        return msi.remesh(mesh, stitch);
    }

}
