/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_KD_TREE_SEARCH_H
#define EASY3D_KD_TREE_SEARCH_H


#include <vector>
#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;

    /**
     * \brief Base interface for nearest-neighbor and spatial range queries over 3D points.
     * \class KdTreeSearch easy3d/kdtree/kdtree_search.h
     * \see KdTreeSearch_ANN, KdTreeSearch_ETH, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN
     *
     * \details Implementations differ in supported queries, approximation controls, result ordering, and
     *          concurrency guarantees. Unless a specific implementation documents otherwise, do not assume
     *          concurrent queries or mutation of the indexed data are safe.
     *
     *          The point cloud or vector supplied to a constructor must contain at least one point and must
     *          remain alive and unchanged for the lifetime of the search object. The index refers to the source
     *          data rather than providing a general-purpose mutable copy.
     */

    class KdTreeSearch {
    public:
        /**
         * \brief Constructor.
         * \param cloud Non-empty point cloud to index; it must outlive this search object and remain unchanged.
         */
        explicit KdTreeSearch(const PointCloud *cloud);

        /**
         * \brief Constructor.
         * \param points Non-empty points to index; the vector must outlive this search object and remain unchanged.
         */
        explicit KdTreeSearch(const std::vector<vec3>& points);

        virtual ~KdTreeSearch() = default;

        /// \name Closest point query
        /// @{

        /**
         * \brief Queries the closest point for a given point.
         * \param p The query point.
         * \param squared_distance The squared distance between the query point and its closest neighbor.
         * \note A \b squared distance is returned by the second argument \p squared_distance.
         * \return Index of the nearest point in the source data, or -1 if the implementation finds no point.
         */
        virtual int find_closest_point(const vec3 &p, float &squared_distance) const = 0;

        /**
         * \brief Queries the closest point for a given point.
         * \param p The query point.
         * \return Index of the nearest point in the source data, or -1 if the implementation finds no point.
         */
        virtual int find_closest_point(const vec3 &p) const = 0;
        /// @}

        /// \name K nearest neighbors search
        /// @{

        /**
         * \brief Queries the K nearest neighbors for a given point.
         * \param p The query point.
         * \param k The number of required neighbors.
         * \param neighbors The indices of the neighbors found (the same as in the original point cloud).
         * \param squared_distances The squared distances between the query point and its K nearest neighbors.
         * The values are stored in accordance with their indices.
         * \note The \b squared distances are returned by the argument \p squared_distances.
         *       Results and distances are implementation-dependent if fewer than \p k points are available.
         */
        virtual void find_closest_k_points(const vec3 &p, int k, std::vector<int> &neighbors,
                                           std::vector<float> &squared_distances) const = 0;

        /**
         * \brief Queries the K nearest neighbors for a given point.
         * \param p The query point.
         * \param k The number of required neighbors.
         * \param neighbors The indices of the neighbors found (the same as in the original point cloud).
         * \note Results are implementation-dependent if fewer than \p k points are available.
         */
        virtual void find_closest_k_points(const vec3 &p, int k, std::vector<int> &neighbors) const = 0;
        /// @}

        /// @name Fixed radius search
        /// @{

        /**
         * \brief Queries the nearest neighbors within a fixed range.
         * \param p The query point.
         * \param squared_radius The search range (which is required to be \b squared).
         * \param neighbors The indices of the neighbors found (the same as in the original point cloud).
         * \param squared_distances The squared distances between the query point and the neighbors found.
         * The values are stored in accordance with their indices.
         * \note The \b squared distances are returned by the argument \p squared_distances.
         * \note Result ordering is implementation-dependent.
         */
        virtual void find_points_in_range(const vec3 &p, float squared_radius, std::vector<int> &neighbors,
                                          std::vector<float> &squared_distances) const = 0;

        /**
         * \brief Queries the nearest neighbors within a fixed range.
         * \param p The query point.
         * \param squared_radius The search range (which is required to be \b squared).
         * \param neighbors The indices of the neighbors found (the same as in the original point cloud).
         * \note Result ordering is implementation-dependent.
         */
        virtual void find_points_in_range(const vec3 &p, float squared_radius, std::vector<int> &neighbors) const = 0;
        /// @}
    };

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_H

