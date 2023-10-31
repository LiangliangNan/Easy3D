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

#ifndef EASY3D_KD_TREE_SEARCH_ANN_H
#define EASY3D_KD_TREE_SEARCH_ANN_H

#include <easy3d/kdtree/kdtree_search.h>


namespace easy3d {

    class PointCloud;

    /**
     * \brief KdTree implementation based on [ANN](http://www.cs.umd.edu/~mount/ANN/).
     * \class KdTreeSearch_ANN easy3d/kdtree/kdtree_search_ann.h
     * \see KdTreeSearch_ETH, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN.
     */
    class KdTreeSearch_ANN : public KdTreeSearch {
    public:
        /**
         * \brief Constructor.
         * \param cloud The point cloud for which a KdTree will be constructed.
         */
        explicit KdTreeSearch_ANN(const PointCloud *cloud);

        /**
         * \brief Constructor.
         * \param points The points for which a KdTree will be constructed.
         */
        explicit KdTreeSearch_ANN(const std::vector<vec3>& points);

        ~KdTreeSearch_ANN() override;

        /// \name Closest point query
        /// @{

        /**
         * \brief Queries the closest point for a given point.
         * \param p The query point.
         * \param squared_distance The squared distance between the query point and its closest neighbor.
         * \note A \b squared distance is returned by the second argument \p squared_distance.
         * \return The index of the nearest neighbor found.
         */
        int find_closest_point(const vec3 &p, float &squared_distance) const override;

        /**
         * \brief Queries the closest point for a given point.
         * \param p The query point.
         * \return The index of the nearest neighbor found.
         */
        int find_closest_point(const vec3 &p) const override;
        /// @}

        /// \name K nearest neighbors search
        /// @{

        /**
         * \brief Queries the K nearest neighbors for a given point.
         * \param p The query point.
         * \param k The number of required neighbors.
         * \param neighbors The indices of the neighbors found.
         * \param squared_distances The squared distances between the query point and its K nearest neighbors.
         * The values are stored in accordance with their indices.
         * \note The \b squared distances are returned by the argument \p squared_distances.
         */
        void find_closest_k_points(
                const vec3 &p, int k,
                std::vector<int> &neighbors, std::vector<float> &squared_distances
        ) const override;

        /**
         * \brief Queries the K nearest neighbors for a given point.
         * \param p The query point.
         * \param k The number of required neighbors.
         * \param neighbors The indices of the neighbors found.
         */
        void find_closest_k_points(
                const vec3 &p, int k,
                std::vector<int> &neighbors
        ) const override;
        /// @}

        /// @name Fixed radius search
        /// @{

        /**
         * \brief Specifies the K for fixed range search.
         * \details ANN's annkFRSearch() needs to specify k. ANN's fixed range search is done in two steps.
         * First, it computes the k nearest neighbors within the radius bound. Second, it returns the total number
         * of points lying within the radius bound. It is permitted to set k = 0, in which case it only answers a range
         * counting query.
         */
        void set_k_for_radius_search(int k) { k_for_radius_search_ = k; }

        /**
         * \brief Queries the nearest neighbors within a fixed range.
         * \param p The query point.
         * \param squared_radius The search range (which is required to be \b squared).
         * \param neighbors The indices of the neighbors found.
         * \param squared_distances The squared distances between the query point and the neighbors found.
         * The values are stored in accordance with their indices.
         * \note The \b squared distances are returned by the argument \p squared_distances.
         */
        void find_points_in_range(
                const vec3 &p, float squared_radius,
                std::vector<int> &neighbors, std::vector<float> &squared_distances
        ) const override;

        /**
         * \brief Queries the nearest neighbors within a fixed range.
         * \param p The query point.
         * \param squared_radius The search range (which is required to be \b squared).
         * \param neighbors The indices of the neighbors found.
         */
        void find_points_in_range(
                const vec3 &p, float squared_radius,
                std::vector<int> &neighbors
        ) const override;
        /// @}

#ifndef DOXYGEN
    protected:
        int points_num_;

        float **points_; // a copy of the point cloud data (due to different data structure);

        void *tree_;
        int k_for_radius_search_;
#endif
    };

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_ANN_H


