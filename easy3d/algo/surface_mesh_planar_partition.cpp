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


#include <easy3d/algo/surface_mesh_planar_partition.h>

#include <set>
#include <stack>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    SurfaceMeshPlanarPartition::SurfaceMeshPlanarPartition(SurfaceMesh *mesh)
            : mesh_(mesh) {
        mesh_->update_vertex_normals();
        vertex_normal_ = mesh_->get_vertex_property<vec3>("v:normal");
    }


    SurfaceMeshPlanarPartition::~SurfaceMeshPlanarPartition(void) {
    }


    bool SurfaceMeshPlanarPartition::can_grow(SurfaceMesh::Vertex v) const {
        for (auto f : mesh_->faces(v)) {
            if (planar_segments_[f] == -1)
                return true;
        }
        return false;
    }


    float SurfaceMeshPlanarPartition::max_squared_dist_to_plane(SurfaceMesh::Face f, const Plane3& plane) const {
        float max_sd = -FLT_MAX;
        for (auto v : mesh_->vertices(f)) {
            float sd = plane.squared_ditance(mesh_->position(v));
            if (sd > max_sd)
                max_sd = sd;
        }
        return max_sd;
    }


    void SurfaceMeshPlanarPartition::propagate(SurfaceMesh::Vertex v, int id, float max_allowed_squared_deviation) {
        std::stack<SurfaceMesh::Face> stack;
        for (auto f : mesh_->faces(v))
            stack.push(f);

        const Plane3 plane(mesh_->position(v), vertex_normal_[v]);

        while (!stack.empty()) {
            SurfaceMesh::Face top = stack.top();
            stack.pop();

            if (planar_segments_[top] == -1 && max_squared_dist_to_plane(top, plane) <= max_allowed_squared_deviation) {
                planar_segments_[top] = id;

                for (auto h : mesh_->halfedges(top)) {
                    auto op = mesh_->opposite(h);
                    if (!mesh_->is_border(op)) {
                        auto f = mesh_->face(op);
                        if (planar_segments_[f] == -1) {
                            stack.push(f);
                        }
                    }
                }
            }
        }
    }


    void SurfaceMeshPlanarPartition::apply(SurfaceMesh::FaceProperty<int> segments) {
        planar_segments_ = segments;

        // we use (1 - max_abs_curvature) as a metric to measure the planarity.
        const unsigned int iter_smooth = 5;
        const bool two_ring = true;
        SurfaceMeshCurvature analyzer(mesh_);
        analyzer.analyze_tensor(iter_smooth, two_ring);
        auto curvature = mesh_->vertex_property<float>("v:curvature");
        float max_curvature = -FLT_MAX;
        for (auto v : mesh_->vertices()) {
            float curv = analyzer.max_abs_curvature(v);
            max_curvature = std::max(max_curvature, curv);
            curvature[v] = curv;
        }

        // functor for comparing two vertices w.r.t. their planarity
        class VertexCmp {
        public:
            VertexCmp(const SurfaceMesh::VertexProperty<float>& planarity) : planarity_(planarity) {}
            bool operator()(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) const {
                return ((planarity_[v0] == planarity_[v1]) ? (v0.idx() > v1.idx()) : (planarity_[v0] > planarity_[v1]));
            }
        private:
            const SurfaceMesh::VertexProperty<float> planarity_;
        };
        // priority queue using planarity as sorting criterion
        typedef std::set<SurfaceMesh::Vertex, VertexCmp> PriorityQueue;

        auto planarity = mesh_->vertex_property<float>("v:planarity");
        for (auto v : mesh_->vertices())
            planarity[v] = max_curvature - curvature[v];
        mesh_->remove_vertex_property(curvature);

        PriorityQueue priority_queue(planarity);
        for (auto v :mesh_->vertices())
            priority_queue.insert(v);

        auto locked = mesh_->vertex_property<bool>("v:locked", false);
        std::vector<SurfaceMesh::Vertex> seeds;

        float dist = mesh_->bounding_box().diagonal() * 0.005f;
        float max_allowed_sq_dist = dist * dist;

        planar_segments_ = segments;
        planar_segments_.vector().assign(mesh_->n_faces(), -1);
        int id = 0;
        for (auto v : priority_queue) {
            if (can_grow(v)) {
                locked[v] = true;
                seeds.push_back(v);
                propagate(v, id, max_allowed_sq_dist);
                ++id;
            }
        }
        LOG(INFO) << "accumulated " << id << " planar segments";
    }

}