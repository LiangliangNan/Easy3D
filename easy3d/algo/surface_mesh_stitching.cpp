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

#include <easy3d/algo/surface_mesh_stitching.h>

#include <algorithm>
#include <cassert>

#include <easy3d/core/surface_mesh.h>

#include <3rd_party/kdtree/ANN/ANN.h>


using namespace ANN;

#define get_tree(x) (reinterpret_cast<ANNkd_tree*>(x))


namespace easy3d {

    SurfaceMeshStitching::SurfaceMeshStitching(SurfaceMesh *mesh)
            : mesh_(mesh), coordinates_(nullptr), tree_(nullptr), k_for_radius_search_(4) {
        for (auto h : mesh_->halfedges()) {
            if (mesh_->is_border(h))
                border_edges_.push_back(h);
        }

        if (border_edges_.empty())
            return;

        coordinates_ = annAllocPts(border_edges_.size(), 6);
        for (std::size_t i = 0; i < border_edges_.size(); ++i)
            assign_edge_coordinate(coordinates_[i], border_edges_[i]);

        tree_ = new ANNkd_tree(coordinates_, border_edges_.size(), 6);
    }


    SurfaceMeshStitching::~SurfaceMeshStitching() {
        if (coordinates_)
            annDeallocPts(coordinates_);

        delete get_tree(tree_);
        annClose();
    }


    bool SurfaceMeshStitching::lexicographically_smaller(const vec3 &p0, const vec3 &p1) const {
        if (p0.x > p1.x)
            return false;
        else if (p0.x < p1.x)
            return true;
        else {
            if (p0.y > p1.y)
                return false;
            else if (p0.y < p1.y)
                return true;
            else {
                if (p0.z > p1.z)
                    return false;
                else if (p0.z < p1.z)
                    return true;
                else
                    return false;
            }
        }
        assert(false);
        return true;
    }


    void SurfaceMeshStitching::assign_edge_coordinate(float *coords, SurfaceMesh::Halfedge h) const {
        const auto vs = mesh_->source(h);
        const auto vt = mesh_->target(h);
        const vec3 &s = mesh_->position(vs);
        const vec3 &t = mesh_->position(vt);
        if (lexicographically_smaller(s, t)) {
            for (std::size_t j = 0; j < 3; ++j) {
                coords[j] = s[j];
                coords[j + 3] = t[j];
            }
        } else {
            for (std::size_t j = 0; j < 3; ++j) {
                coords[j] = t[j];
                coords[j + 3] = s[j];
            }
        }
    }


    void SurfaceMeshStitching::borders_in_range(
            SurfaceMesh::Halfedge h, float squared_dist_threshold,
            std::vector<SurfaceMesh::Halfedge> &neighbors
    ) const {
        ANNcoord ann_p[6];
        assign_edge_coordinate(ann_p, h);

        std::vector<int> pts_indices(k_for_radius_search_);               // neighbor indices
        std::vector<float> pts_squared_distances(k_for_radius_search_);     // neighbor (squared) distances
        const int n = get_tree(tree_)->annkFRSearch(ann_p, squared_dist_threshold, k_for_radius_search_,
                                                    pts_indices.data(), pts_squared_distances.data());

        const int num = std::min(n, k_for_radius_search_);
        neighbors.reserve(num);
        for (int i = 0; i < num; ++i) {
            auto idx = pts_indices[i];
            if (border_edges_[idx] != h) {  // exclude it self
                neighbors.push_back(border_edges_[idx]);
            }
        }
    }


    float SurfaceMeshStitching::squared_distance(SurfaceMesh::Halfedge h1, SurfaceMesh::Halfedge h2) const {
        auto s1 = mesh_->source(h1);
        auto t1 = mesh_->target(h1);
        auto s2 = mesh_->source(h2);
        auto t2 = mesh_->target(h2);
        return std::max(
                distance2(mesh_->position(s1), mesh_->position(t2)),
                distance2(mesh_->position(s2), mesh_->position(t1)));
    }


    SurfaceMesh::Halfedge
    SurfaceMeshStitching::matched_border(SurfaceMesh::Halfedge h, float squared_dist_threshold) const {
        std::vector<SurfaceMesh::Halfedge> neighbors;
        borders_in_range(h, squared_dist_threshold, neighbors);

        float min_sd = squared_dist_threshold;
        SurfaceMesh::Halfedge best_match;

        for (int i = 0; i < neighbors.size(); ++i) {
            auto h2 = neighbors[i];
            float sd = squared_distance(h, h2);
            if (sd < min_sd) {
                min_sd = sd;
                best_match = h2;
            }
        }

        return best_match;
    }


    void SurfaceMeshStitching::apply(float dist_threshold) {
        auto scheduled = mesh_->add_halfedge_property<bool>("h::scheduled::SurfaceMeshStitching::apply", false);
        std::vector<std::pair<SurfaceMesh::Halfedge, SurfaceMesh::Halfedge> > to_stitch;

        const float squared_dist_threshold = dist_threshold * dist_threshold;
        for (auto h : border_edges_) {
            if (!scheduled[h]) {
                auto h2 = matched_border(h, squared_dist_threshold);
                if (h2.is_valid() && !scheduled[h2]) {
                    to_stitch.emplace_back(std::make_pair(h, h2));
                    scheduled[h] = true;
                    scheduled[h2] = true;
                }
            }
        }

        std::size_t count = 0;
        if (!to_stitch.empty()) {
            for (const auto &ep : to_stitch) {
                if (mesh_->is_stitch_ok(ep.first, ep.second)) {
                    mesh_->stitch(ep.first, ep.second);
                    ++count;
                }
            }
            mesh_->collect_garbage();

            if (count > 0)
                LOG(INFO) << count << " (out of " << to_stitch.size() << ") pairs of edges stitched";
            else
                LOG(WARNING) << "none of the " << to_stitch.size() << " edge pairs can be stitched";
        } else {
            LOG(WARNING) << "no coincident edges can be found for stitching";
        }

        mesh_->remove_halfedge_property(scheduled);
    }


} // namespace easy3d
