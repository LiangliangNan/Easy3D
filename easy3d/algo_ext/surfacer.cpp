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
#include <easy3d/algo_ext/surfacer.h>

#include <easy3d/algo_ext/duplicate_faces.h>
#include <easy3d/algo_ext/self_intersection.h>
#include <easy3d/util/logging.h>


#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel          K;
typedef CGAL::Surface_mesh<K::Point_3>                               CGALMesh;



namespace easy3d {

    namespace details {

        void easy3d_to_cgal(SurfaceMesh *input, CGALMesh &output) {
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
                    vts.push_back(CGALMesh::Vertex_index(v.idx()));
                output.add_face(vts);
            }
        }

        void cgal_to_easy3d(CGALMesh &input, SurfaceMesh *output) {
            if (input.has_garbage())
                input.collect_garbage();

            output->clear();
            for (auto v : input.vertices()) {
                const K::Point_3 &p = input.point(v);
                output->add_vertex(
                        vec3(CGAL::to_double(p.x()),
                             CGAL::to_double(p.y()),
                             CGAL::to_double(p.z())
                        )
                );
            }

            for (auto f : input.faces()) {
                std::vector<SurfaceMesh::Vertex> vts;
                auto h = input.halfedge(f);
                for (auto v : input.vertices_around_face(h))
                    vts.push_back(SurfaceMesh::Vertex(v.idx()));
                output->add_face(vts);
            }
        }


        void easy3d_mesh_to_polygon_soup(SurfaceMesh *mesh,
                                         std::vector<vec3> &points,
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


        void polygon_soup_to_easy3d_mesh(const std::vector<vec3> &points,
                                         const std::vector<Surfacer::Polygon> &polygons,
                                         SurfaceMesh *mesh) {
            mesh->clear();
            for (auto p : points)
                mesh->add_vertex(p);

            for (const auto& plg : polygons) {
                std::vector<SurfaceMesh::Vertex> vts;
                for (auto v : plg)
                    vts.push_back(SurfaceMesh::Vertex(v));
                mesh->add_face(vts);
            }
        }
    }





    void Surfacer::merge_reversible_connected_components(SurfaceMesh* input) {
        CGALMesh mesh;
        details::easy3d_to_cgal(input, mesh);

        typedef boost::property_map<CGALMesh, CGAL::dynamic_face_property_t<std::size_t> >::type Fccmap;
        Fccmap fccmap = CGAL::get(CGAL::dynamic_face_property_t<std::size_t>(), mesh);
        int count_oirg = CGAL::Polygon_mesh_processing::connected_components(mesh, fccmap);
        if (count_oirg <= 1) {
            LOG(INFO) << "model has only one connected components, so nothing to merge";
            return; // nothing to be merged
        }

        // iteratively merge the connected components until the no more connected components can be merged
        int count_prev = count_oirg;
        int count_now = -1;
        do {
            CGAL::Polygon_mesh_processing::merge_reversible_connected_components(mesh);
            count_now = CGAL::Polygon_mesh_processing::connected_components(mesh, fccmap);
            if (count_now == count_prev)
                break;
            count_prev = count_now;
        } while (true);

        details::cgal_to_easy3d(mesh, input);
        if (count_now != count_oirg)
            LOG(INFO) << count_oirg << " connected components merged into " << count_now;
        else
            LOG(INFO) << "model has " << count_oirg << " connected components but nothing could be merged";
    }


    void Surfacer::reverse_orientation(SurfaceMesh *mesh) {
        const std::vector<vec3> points = mesh->points();

        typedef std::vector<SurfaceMesh::Vertex> Polygon;
        std::vector<Polygon> polygons(mesh->n_faces());
        for (auto f : mesh->faces()) {
            auto& plg = polygons[f.idx()];
            for (auto v : mesh->vertices(f))
                plg.push_back(v);
            std::reverse(plg.begin(), plg.end());
        }

        mesh->clear();

        for (const auto& p : points)
            mesh->add_vertex(p);

        for (const auto& plg : polygons)
            mesh->add_face(plg);
    }


    bool Surfacer::orient_polygon_soup(std::vector<vec3> &input_points, std::vector<Polygon> &input_polygons) {
        std::vector<K::Point_3> points;
        std::vector< std::vector<std::size_t> > polygons(input_polygons.size());

        // helper class for identifying coincident vertices
        class CmpVec {
        public:
            CmpVec(float _eps = FLT_MIN) : eps_(_eps) {}
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

        CmpVec comp(FLT_MIN);
        std::map<vec3, int, CmpVec>            vMap(comp);
        std::map<vec3, int, CmpVec>::iterator  vMapIt;

        std::map<int, int> index_map;

        for (std::size_t i=0; i<input_points.size(); ++i) {
            const vec3& p = input_points[i];

            // has the point been referenced before?
            vMapIt = vMap.find(p);
            if (vMapIt == vMap.end()) {
                // No : add vertex and remember idx/vector mapping
                vMap[p] = points.size();
                index_map[i] = points.size();
                points.push_back(K::Point_3(p.x, p.y, p.z));
            }
            else {
                // Yes : get index from map
                index_map[i] = vMapIt->second;
            }
        }
        for (std::size_t i=0; i<input_polygons.size(); ++i) {
            for (auto id : input_polygons[i])
                polygons[i].push_back(index_map[id]);
        }

        bool status = CGAL::Polygon_mesh_processing::orient_polygon_soup(points, polygons);

        if(!CGAL::Polygon_mesh_processing::is_polygon_soup_a_polygon_mesh(polygons)) {
            LOG(WARNING) << "the polygons after orientation do not define a valid polygon mesh";
            return false;
        }

        // convert to easy3d
        input_points.resize(points.size());
        input_polygons.resize(polygons.size());

        for (std::size_t i=0; i<points.size(); ++i) {
            const auto& p = points[i];
            input_points[i] = vec3(
                    CGAL::to_double(p.x()),
                    CGAL::to_double(p.y()),
                    CGAL::to_double(p.z())
            );
        }

        for (std::size_t i=0; i<polygons.size(); ++i) {
            const auto &plg_new = polygons[i];

            auto &plg = input_polygons[i];
            plg.resize(plg_new.size());
            for (std::size_t j = 0; j < plg_new.size(); ++j)
                plg[j] = plg_new[j];
        }

        return status;
    }


    int Surfacer::stitch_borders(SurfaceMesh* input) {
        CGALMesh mesh;
        details::easy3d_to_cgal(input, mesh);
        int num = CGAL::Polygon_mesh_processing::stitch_borders(mesh);
        details::cgal_to_easy3d(mesh, input);
        LOG(INFO) << num << " pairs of halfedges have been stitched";
        return num;
    }


    bool Surfacer::merge_reversible_connected_components_2(SurfaceMesh* mesh) {
        if (!mesh)
            return false;

        std::vector<vec3> points;
        std::vector<Polygon> polygons;
        details::easy3d_mesh_to_polygon_soup(mesh, points, polygons);

        bool status = orient_polygon_soup(points, polygons);
        details::polygon_soup_to_easy3d_mesh(points, polygons, mesh);

        return status;
    }


    std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
    Surfacer::detect_duplicate_faces(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        easy3d::DuplicateFaces rdf;
        return rdf.detect(mesh, exact, dist_threshold);
    }


    unsigned int Surfacer::remove_duplicate_faces(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        DuplicateFaces rdf;
        return rdf.remove(mesh, exact, dist_threshold);
    }


    std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
    Surfacer::detect_self_intersections(SurfaceMesh* mesh)
    {
        SelfIntersection msi;
        return msi.detect(mesh, false);
    }


    bool Surfacer::remesh_self_intersections(SurfaceMesh* mesh, bool stitch /* = true */)
    {
        SelfIntersection msi;
        return msi.remesh(mesh, stitch);
    }

}
