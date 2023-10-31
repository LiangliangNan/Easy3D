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

#include <easy3d/kdtree/kdtree_search_flann.h>
#include <easy3d/core/point_cloud.h>

#include <3rd_party/kdtree/FLANN/flann.hpp>


#define get_tree(x) (reinterpret_cast<const flann::Index< flann::L2<float> > *>(x))

namespace easy3d {

    KdTreeSearch_FLANN::KdTreeSearch_FLANN(const PointCloud *cloud) : KdTreeSearch(cloud) {
        //checks_ = 32;
        checks_ = flann::FLANN_CHECKS_AUTOTUNED;

        // prepare data
        points_num_ = int(cloud->n_vertices());
        const std::vector<vec3>& points = cloud->points();
        points_ = const_cast<float *>(points[0].data());

        // create tree
        flann::Matrix<float> dataset(points_, points_num_, 3);
        // construct a single kd-tree optimized for searching lower dimensionality data
        auto tree = new flann::Index< flann::L2<float> >(dataset, flann::KDTreeSingleIndexParams());
        tree->buildIndex();
        tree_ = tree;
    }


    KdTreeSearch_FLANN::KdTreeSearch_FLANN(const std::vector<vec3>& points) : KdTreeSearch(points) {
        //checks_ = 32;
        checks_ = flann::FLANN_CHECKS_AUTOTUNED;

        // prepare data
        points_num_ = int(points.size());
        points_ = const_cast<float*>(points[0].data());

        // create tree
        flann::Matrix<float> dataset(points_, points_num_, 3);
        // construct a single kd-tree optimized for searching lower dimensionality data
        auto tree = new flann::Index< flann::L2<float> >(dataset, flann::KDTreeSingleIndexParams());
        tree->buildIndex();
        tree_ = tree;
    }


    KdTreeSearch_FLANN::~KdTreeSearch_FLANN() {
        delete get_tree(tree_);
    }


    void KdTreeSearch_FLANN::set_checks(int chk) {
        checks_ = chk;
    }


    int KdTreeSearch_FLANN::find_closest_point(const vec3& p, float& squared_distance) const {
        flann::Matrix<float> query(const_cast<float*>(p.data()), 1, 3);

        std::vector< std::vector<int> >		indices;
        std::vector< std::vector<float> >	dists;

        get_tree(tree_)->knnSearch(query, indices, dists, 1, flann::SearchParams(checks_));

        squared_distance = dists[0][0];
        return indices[0][0];
    }


    int KdTreeSearch_FLANN::find_closest_point(const vec3& p) const {
        float dist = 0;
        return find_closest_point(p, dist);
    }


    void KdTreeSearch_FLANN::find_closest_k_points(
        const vec3& p, int k, std::vector<int>& neighbors, std::vector<float>& squared_distances
        )  const
    {
        flann::Matrix<float> query(const_cast<float*>(p.data()), 1, 3);

        std::vector< std::vector<int> >		indices;
        std::vector< std::vector<float> >	dists;

        get_tree(tree_)->knnSearch(query, indices, dists, k, flann::SearchParams(checks_));

        neighbors = indices[0];
        squared_distances = dists[0];
    }


    void KdTreeSearch_FLANN::find_closest_k_points(
        const vec3& p, int k, std::vector<int>& neighbors
    )  const
    {
        std::vector<float> squared_distances;
        return find_closest_k_points(p, k, neighbors, squared_distances);
    }


    void KdTreeSearch_FLANN::find_points_in_range(
        const vec3& p, float squared_radius, std::vector<int>& neighbors, std::vector<float>& squared_distances
        )  const {
        flann::Matrix<float> query(const_cast<float*>(p.data()), 1, 3);

        std::vector< std::vector<int> >		indices;
        std::vector< std::vector<float> >	dists;

        get_tree(tree_)->radiusSearch(query, indices, dists, squared_radius, flann::SearchParams(checks_));

        size_t num = indices[0].size();
        neighbors.resize(num);
        squared_distances.resize(num);
        for (size_t i=0; i<num; ++i) {
            neighbors[i] = indices[0][i];
            squared_distances[i] = dists[0][i];
        }
    }


    void KdTreeSearch_FLANN::find_points_in_range(
        const vec3& p, float squared_radius, std::vector<int>& neighbors
    )  const
    {
        std::vector<float> squared_distances;
        return find_points_in_range(p, squared_radius, neighbors, squared_distances);
    }


} // namespace easy3d
