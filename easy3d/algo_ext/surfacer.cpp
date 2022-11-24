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

        CGALMesh mesh;
        internal::to_cgal(input_mesh, mesh);

        Plane_3 plane(input_plane.a(), input_plane.b(), input_plane.c(), input_plane.d());
        bool status = CGAL::Polygon_mesh_processing::clip(
                mesh,
                plane,
                CGAL::Polygon_mesh_processing::parameters::clip_volume(clip_volume)
        );

        internal::to_easy3d(mesh, input_mesh);
        return status;
    }


    void Surfacer::split(SurfaceMesh *input_mesh, const Plane3 &input_plane) {
        if (!input_mesh->is_triangle_mesh()) {
            LOG(WARNING) << "only triangle meshes can be clipped";
            return;
        }

        CGALMesh mesh;
        internal::to_cgal(input_mesh, mesh);

        Plane_3 plane(input_plane.a(), input_plane.b(), input_plane.c(), input_plane.d());
        CGAL::Polygon_mesh_processing::split(mesh, plane);

        internal::to_easy3d(mesh, input_mesh);
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


    unsigned int Surfacer::remove_overlapping_faces(SurfaceMesh *mesh, bool folding_faces, double dist_threshold) {
        OverlappingFaces rdf;
        return rdf.remove(mesh, folding_faces, dist_threshold);
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
