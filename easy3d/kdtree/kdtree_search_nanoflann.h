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

#ifndef EASY3D_KD_TREE_SEARCH_NANOFLANN_H
#define EASY3D_KD_TREE_SEARCH_NANOFLANN_H

#include <easy3d/kdtree/kdtree_search.h>

namespace easy3d {

    class PointCloud;

    /**
     * \brief KdTree implementation based on [NanoFLANN](https://github.com/jlblancoc/nanoflann).
     * \class KdTreeSearch_NanoFLANN easy3d/kdtree/kdtree_search_nanoflann.h
     * \see KdTreeSearch_ANN, KdTreeSearch_ETH, and KdTreeSearch_FLANN.
     */
    class KdTreeSearch_NanoFLANN : public KdTreeSearch {
    public:
        /**
         * \brief Constructor.
         * \param cloud The point cloud for which a KdTree will be constructed.
         */
        explicit KdTreeSearch_NanoFLANN(const PointCloud *cloud);

        /**
         * \brief Constructor.
         * \param points The points for which a KdTree will be constructed.
         */
        explicit KdTreeSearch_NanoFLANN(const std::vector<vec3>& points);

        ~KdTreeSearch_NanoFLANN() override;

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

    protected:
        std::vector<vec3> *points_; // reference of the original point cloud data
        void *tree_;
    };

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_NANOFLANN_H


