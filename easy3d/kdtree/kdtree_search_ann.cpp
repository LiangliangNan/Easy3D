/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <algorithm>

#include <easy3d/kdtree/kdtree_search_ann.h>
#include <easy3d/core/point_cloud.h>

#include <3rd_party/kd_tree/ANN/ANN.h>


using namespace ANN;

#define get_tree(x) (reinterpret_cast<ANNkd_tree*>(x))


namespace easy3d {

    KdTreeSearch_ANN::KdTreeSearch_ANN()  {
        points_ = nullptr;
        points_num_ = 0;
        tree_ = nullptr;
        k_for_radius_search_ = 32;
        std::cerr << "KdTreeSearch_ANN: k = 32 for radius search" << std::endl;
    }


    KdTreeSearch_ANN::~KdTreeSearch_ANN() {
        if (points_)
            annDeallocPts(points_);

        delete get_tree(tree_);
        annClose();
    }


    void KdTreeSearch_ANN::begin()  {
        points_num_ = 0;

        if (points_)
            annDeallocPts(points_);

        delete get_tree(tree_);
        tree_ = nullptr;
    }


    void KdTreeSearch_ANN::end()  {
        tree_ = new ANNkd_tree(points_, points_num_, 3);
    }


    void KdTreeSearch_ANN::add_point_cloud(PointCloud* cloud)  {
        points_num_ = int(cloud->n_vertices());
        points_ = annAllocPts(points_num_, 3);

        const std::vector<vec3>& pts = cloud->points();
        for (int i = 0; i < points_num_; ++i) {
            const vec3& p = pts[i];
            points_[i][0] = p[0];
            points_[i][1] = p[1];
            points_[i][2] = p[2];
        }
    }


    int KdTreeSearch_ANN::find_closest_point(const vec3& p) const {
        ANNcoord ann_p[3];

        ANNidx closest_pt_ix;
        ANNdist closest_pt_dist;

        ann_p[0] = p[0];
        ann_p[1] = p[1];
        ann_p[2] = p[2];

        get_tree(tree_)->annkSearch(ann_p, 1, &closest_pt_ix, &closest_pt_dist);

        return closest_pt_ix;
    }


    int KdTreeSearch_ANN::find_closest_point(const vec3& p, float& squared_distance) const {
        ANNcoord ann_p[3];
        ann_p[0] = p[0];
        ann_p[1] = p[1];
        ann_p[2] = p[2];

        ANNidx closest_pt_idx;
        ANNdist closest_pt_dist;

        get_tree(tree_)->annkSearch(ann_p, 1, &closest_pt_idx, &closest_pt_dist);
        squared_distance = closest_pt_dist; // ANN uses squared distance internally

        return closest_pt_idx;
    }


    void KdTreeSearch_ANN::find_closest_K_points(
        const vec3& p, int k, std::vector<int>& neighbors
        )  const {
            ANNcoord ann_p[3];
            ann_p[0] = p[0];
            ann_p[1] = p[1];
            ann_p[2] = p[2];

            neighbors.resize(k);
            ANNdistArray closest_pts_dists = new ANNdist[k];	// neighbor distances
            get_tree(tree_)->annkSearch(ann_p, k, neighbors.data(), closest_pts_dists);
            delete [] closest_pts_dists;
    }


    void KdTreeSearch_ANN::find_closest_K_points(
        const vec3& p, int k, std::vector<int>& neighbors, std::vector<float>& squared_distances
        )  const {
            ANNcoord ann_p[3];
            ann_p[0] = p[0];
            ann_p[1] = p[1];
            ann_p[2] = p[2];

            neighbors.resize(k);
            squared_distances.resize(k);
            get_tree(tree_)->annkSearch(ann_p, k, neighbors.data(), squared_distances.data());
    }


    void KdTreeSearch_ANN::find_points_in_radius(
        const vec3& p, float squared_radius, std::vector<int>& neighbors
        )  const {
            ANNcoord ann_p[3];
            ann_p[0] = p[0];
            ann_p[1] = p[1];
            ann_p[2] = p[2];

            ANNidxArray  closest_pts_idx = new ANNidx[k_for_radius_search_];		// near neighbor indices
            ANNdistArray closest_pts_dists = new ANNdist[k_for_radius_search_];		// near neighbor distances
            int n = get_tree(tree_)->annkFRSearch(ann_p, squared_radius, k_for_radius_search_, closest_pts_idx, closest_pts_dists);

            int num = std::min(n, k_for_radius_search_);
            neighbors.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = closest_pts_idx[i];
            }

            delete [] closest_pts_idx;
            delete [] closest_pts_dists;
    }


    void KdTreeSearch_ANN::find_points_in_radius(
        const vec3& p, float squared_radius, std::vector<int>& neighbors, std::vector<float>& squared_distances
        )  const {
            ANNcoord ann_p[3];
            ann_p[0] = p[0];
            ann_p[1] = p[1];
            ann_p[2] = p[2];

            ANNidxArray  closest_pts_idx = new ANNidx[k_for_radius_search_];		// near neighbor indices
            ANNdistArray closest_pts_dists = new ANNdist[k_for_radius_search_];		// near neighbor distances
            int n = get_tree(tree_)->annkFRSearch(ann_p, squared_radius, k_for_radius_search_, closest_pts_idx, closest_pts_dists);

            int num = std::min(n, k_for_radius_search_);
            neighbors.resize(num);
            squared_distances.resize(num);
            for (int i=0; i<num; ++i) {
                neighbors[i] = closest_pts_idx[i];
                squared_distances[i] = closest_pts_dists[i]; // ANN uses squared distance internally
            }

            delete [] closest_pts_idx;
            delete [] closest_pts_dists;
    }


} // namespace easy3d
