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

#include <easy3d/kdtree/kdtree_search_nanoflann.h>
#include <easy3d/core/point_cloud.h>

#include <3rd_party/kdtree/nanoflann/nanoflann.hpp>


using namespace nanoflann;

namespace easy3d {

    struct PointSet {
        explicit PointSet(const std::vector<vec3>* points) : pts(points) {}
        const std::vector<vec3>*  pts;

        // Must return the number of data points
        inline size_t kdtree_get_point_count() const { return pts->size(); }

        // Returns the dim'th component of the idx'th point in the class:
        // Since this is inlined and the "dim" argument is typically an immediate value, the
        //  "if/else's" are actually solved at compile time.
        inline float kdtree_get_pt(const size_t idx, const size_t dim) const {
            return pts->at(idx)[dim];
        }

        // Optional bounding-box computation: return false to default to a standard bbox computation loop.
        // Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
        // Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
        template <class BBOX>
        bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
    };


    struct KdTree : public KDTreeSingleIndexAdaptor< L2_Simple_Adaptor<float, PointSet>, PointSet, 3, int> {
        explicit KdTree(PointSet* pset) : KDTreeSingleIndexAdaptor< L2_Simple_Adaptor<float, PointSet>, PointSet, 3, int >(3, *pset, KDTreeSingleIndexAdaptorParams(10)){
            pset_ = pset;
        }
        ~KdTree() { delete pset_; }

        PointSet* pset_;
    };

    #define get_tree(x) (reinterpret_cast<const KdTree *>(x))



    KdTreeSearch_NanoFLANN::KdTreeSearch_NanoFLANN(const PointCloud *cloud) : KdTreeSearch(cloud) {
        // prepare data
        points_ = const_cast< std::vector<vec3>* >(&cloud->points());

        // create tree
        auto pset = new PointSet(points_);
        auto tree = new KdTree(pset);
        tree->buildIndex();
        tree_ = tree;
    }


    KdTreeSearch_NanoFLANN::KdTreeSearch_NanoFLANN(const std::vector<vec3>& points) : KdTreeSearch(points) {
        // prepare data
        points_ = const_cast<std::vector<vec3>*>(&points);

        // create tree
        auto pset = new PointSet(points_);
        auto tree = new KdTree(pset);
        tree->buildIndex();
        tree_ = tree;
    }


    KdTreeSearch_NanoFLANN::~KdTreeSearch_NanoFLANN() {
        delete get_tree(tree_);
    }


    int KdTreeSearch_NanoFLANN::find_closest_point(const vec3& p, float& squared_distance) const {
        std::size_t index;

        nanoflann::KNNResultSet<float> result_set(1);
        result_set.init(&index, &squared_distance);

        get_tree(tree_)->findNeighbors(result_set, p, nanoflann::SearchParams(10));
        return static_cast<int>(index);
    }


    int KdTreeSearch_NanoFLANN::find_closest_point(const vec3& p) const {
        float dist = 0;
        return find_closest_point(p, dist);
    }


    void KdTreeSearch_NanoFLANN::find_closest_k_points(
        const vec3& p, int k, std::vector<int>& neighbors, std::vector<float>& squared_distances
    )  const
    {
        std::vector<int> indices(k);
        std::vector<float>	sqr_distances(k);

        nanoflann::KNNResultSet<float, int> result_set(k);
        result_set.init(&indices[0], &sqr_distances[0]);
        get_tree(tree_)->findNeighbors(result_set, p, nanoflann::SearchParams(10));

        neighbors = std::vector<int>(indices.begin(), indices.end());
        squared_distances = sqr_distances;
    }


    void KdTreeSearch_NanoFLANN::find_closest_k_points(
        const vec3& p, int k, std::vector<int>& neighbors
    )  const
    {
        std::vector<float> squared_distances;
        return find_closest_k_points(p, k, neighbors, squared_distances);
    }


    void KdTreeSearch_NanoFLANN::find_points_in_range(
        const vec3& p, float squared_radius, std::vector<int>& neighbors, std::vector<float>& squared_distances
    )  const {
        std::vector<std::pair<int , float> >   matches;
        nanoflann::SearchParams params;
        params.sorted = false;
        const std::size_t num = get_tree(tree_)->radiusSearch(p, squared_radius, matches, params);

        neighbors.resize(num);
        squared_distances.resize(num);
        for (std::size_t i = 0; i < num; ++i) {
            const std::pair<std::size_t, float>& e = matches[i];
            neighbors[i] = static_cast<int>(e.first);
            squared_distances[i] = e.second;
        }
    }


    void KdTreeSearch_NanoFLANN::find_points_in_range(
        const vec3& p, float squared_radius, std::vector<int>& neighbors
    )  const
    {
        std::vector<float> sqr_distances;
        return find_points_in_range(p, squared_radius, neighbors, sqr_distances);
    }


} // namespace easy3d
