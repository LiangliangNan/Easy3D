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

#ifndef EASY3D_KD_TREE_SEARCH_H
#define EASY3D_KD_TREE_SEARCH_H


#include <vector>
#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;

    /**
     * \brief Base class for nearest neighbor search using KdTree.
     * \class KdTreeSearch easy3d/kdtree/kdtree_search.h
     * \see KdTreeSearch_ANN, KdTreeSearch_ETH, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN
     *
     * \details Easy3D has a collection of KdTree implementations, including [ANN](http://www.cs.umd.edu/~mount/ANN/),
     * ETH, [FLANN](https://github.com/mariusmuja/flann), and [NanoFLANN](https://github.com/jlblancoc/nanoflann) and
     * tried to get the best performance of each implementation. Runtime tests (on Windows, using a Visual C++ 2008,
     * release build) indicated that the ETH implementation has the best performance. Bellow is the statistic summary
     * of the test on two point clouds.
     *
     * \arg Build:  time for constructing the kd-tree.
     * \arg Single: time for querying closest vertex (for each point in the point cloud).
     * \arg KNN:    time for querying K(=16) closest vertex.
     * \arg Radius: time for querying closest vertex within a radius.
     *
     * Test 1: 362, 271 points (the Stanford bunny). radius = 0.001
     * \code
     * --------------------------------------------------------------------------------------
     *       Build         |       Single        |        KNN         |       Radius
     * --------------------|---------------------|--------------------|---------------------
     * ANN    ETH   FLANN  |  ANN   ETH   FLANN  |  ANN   ETH   FLANN |  ANN   ETH   FLANN
     * --------------------|---------------------|--------------------|---------------------
     * 0.14   0.05   0.12  |  0.17  0.11   0.71  |  1.33  1.0   1.48  |  1.32  1.01  1.51
     * --------------------------------------------------------------------------------------
     * \endcode
     *
     * Test 2: 4, 116, 466 points (an noisy urban scan). radius = 0.03
     * \code
     * --------------------------------------------------------------------------------------
     *       Build         |       Single        |        KNN          |       Radius
     * --------------------|---------------------|---------------------|---------------------
     * ANN    ETH   FLANN  |  ANN   ETH   FLANN  |  ANN   ETH   FLANN  |  ANN    ETH   FLANN
     * --------------------|---------------------|---------------------|---------------------
     * 2.2   0.76   1.88   |  2.61  1.84  11.8   |  20.8  13.5  22.0   |  8.75  4.79   15.1
     * --------------------------------------------------------------------------------------
     *\endcode
     *
     * \attention KdTreeSearch_FLANN and KdTreeSearch_NanoFLANN are thread-safe. Others seem not (not tested yet).
     */

    class KdTreeSearch {
    public:
        /**
         * \brief Constructor.
         * \param cloud The point cloud for which a KdTree will be constructed.
         */
        explicit KdTreeSearch(const PointCloud *cloud);

        /**
         * \brief Constructor.
         * \param points The points for which a KdTree will be constructed.
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
         * \return The index of the nearest neighbor found (the same as in the original point cloud).
         */
        virtual int find_closest_point(const vec3 &p, float &squared_distance) const = 0;

        /**
         * \brief Queries the closest point for a given point.
         * \param p The query point.
         * \return The index of the nearest neighbor found (the same as in the original point cloud).
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
         */
        virtual void find_closest_k_points(const vec3 &p, int k, std::vector<int> &neighbors,
                                           std::vector<float> &squared_distances) const = 0;

        /**
         * \brief Queries the K nearest neighbors for a given point.
         * \param p The query point.
         * \param k The number of required neighbors.
         * \param neighbors The indices of the neighbors found (the same as in the original point cloud).
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
         */
        virtual void find_points_in_range(const vec3 &p, float squared_radius, std::vector<int> &neighbors,
                                          std::vector<float> &squared_distances) const = 0;

        /**
         * \brief Queries the nearest neighbors within a fixed range.
         * \param p The query point.
         * \param squared_radius The search range (which is required to be \b squared).
         * \param neighbors The indices of the neighbors found (the same as in the original point cloud).
         */
        virtual void find_points_in_range(const vec3 &p, float squared_radius, std::vector<int> &neighbors) const = 0;
        /// @}
    };

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_H


