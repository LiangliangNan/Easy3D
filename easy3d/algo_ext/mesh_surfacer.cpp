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
#include <easy3d/algo_ext/self_intersection.h>
#include <easy3d/algo_ext/mesh_surfacer.h>
#include <easy3d/util/logging.h>


#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>


namespace easy3d {


    MeshSurfacer::MeshSurfacer()
    {
    }


    MeshSurfacer::~MeshSurfacer() {
    }


    bool MeshSurfacer::orient(std::vector<vec3> &input_points, std::vector<Polygon> &input_polygons) {
        typedef CGAL::Exact_predicates_inexact_constructions_kernel          K;
        typedef CGAL::Surface_mesh<K::Point_3>                               Polyhedron;

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

        Polyhedron mesh;
        CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, polygons, mesh);

        if(CGAL::is_closed(mesh))
            CGAL::Polygon_mesh_processing::orient_to_bound_a_volume(mesh);

        input_points.clear();
        input_polygons.clear();
        for (auto v : mesh.vertices()) {
            const K::Point_3 &p = mesh.point(v);
            input_points.emplace_back(
                    vec3(CGAL::to_double(p.x()),
                         CGAL::to_double(p.y()),
                         CGAL::to_double(p.z())
                    )
            );
        }

        for (auto f : mesh.faces()) {
            auto h = mesh.halfedge(f);
            Polygon plg;
            for (auto v : mesh.vertices_around_face(h))
                plg.push_back(v.idx());
            input_polygons.push_back(plg);
        }

        return status;
    }


    bool MeshSurfacer::orient(SurfaceMesh* mesh) {
        if (!mesh)
            return false;

        std::vector<vec3> points = mesh->points();
        std::vector<Polygon> polygons(mesh->n_faces());
        for (auto f : mesh->faces()) {
            auto& plg = polygons[f.idx()];
            for (auto v : mesh->vertices(f))
                plg.push_back(v.idx());
        }

        bool status = orient(points, polygons);
        mesh->clear();

        for (const auto& p : points)
            mesh->add_vertex(p);

        for (const auto& plg : polygons) {
            std::vector<SurfaceMesh::Vertex> vertices;
            for (auto id : plg)
                vertices.push_back(SurfaceMesh::Vertex(id));
            mesh->add_face(vertices);
        }

        return status;
    }


    std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
    MeshSurfacer::detect_duplicated_faces(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        easy3d::DuplicatedFaces rdf;
        return rdf.detect(mesh, exact, dist_threshold);
    }


    unsigned int MeshSurfacer::remove_duplicated_faces(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        DuplicatedFaces rdf;
        return rdf.remove(mesh, exact, dist_threshold);
    }


    std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
    MeshSurfacer::detect_self_intersections(SurfaceMesh* mesh)
    {
        SelfIntersection msi;
        return msi.detect(mesh, false);
    }


    bool MeshSurfacer::remesh_self_intersections(SurfaceMesh* mesh, bool stitch /* = true */)
    {
        SelfIntersection msi;
        return msi.remesh(mesh, stitch);
    }

}
