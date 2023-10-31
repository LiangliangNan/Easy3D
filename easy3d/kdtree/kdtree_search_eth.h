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

#ifndef EASY3D_KD_TREE_SEARCH_ETH_H
#define EASY3D_KD_TREE_SEARCH_ETH_H

#include <easy3d/kdtree/kdtree_search.h>


namespace easy3d {

    class PointCloud;

    /**
     * \brief KdTree implementation based on Richard Keiser's KdTree code.
     * \class KdTreeSearch_ETH easy3d/kdtree/kdtree_search_eth.h
     * \see KdTreeSearch_ANN, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN.
     */
    class KdTreeSearch_ETH : public KdTreeSearch {
    public:
        /**
         * \brief Constructor.
         * \param cloud The point cloud for which a KdTree will be constructed.
         */
        explicit KdTreeSearch_ETH(const PointCloud *cloud);

        /**
         * \brief Constructor.
         * \param points The points for which a KdTree will be constructed.
         */
        explicit KdTreeSearch_ETH(const std::vector<vec3>& points);

        ~KdTreeSearch_ETH() override;

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


        /// @name Cylinder range search
        /// @{

        /**
         * \brief Queries the nearest neighbors within a cylinder range.
         * \details Searches for the nearest points whose distances to line segment \p p1 - \p p2 are smaller than
         * \p radius (\b not squared).
         * \param p1 One end point of the query line segment.
         * \param p2 The other end point of the query line segment.
         * \param radius The search range (which is \b not squared).
         * \param neighbors The indices of the neighbors found.
         * \param squared_distances The squared distances between the found neighbors to the query line. The values
         * are stored in accordance with their indices.
         * \param bToLine Specifies how the points found are ordered. If \bToLine=true, the points found are ordered
         * by their distances to the line. Otherwise, they are ordered by their distances to \p p1.
         * \note The range is specified by radius (\b not squared) but \b squared distances are returned.
         */
        int find_points_in_cylinder(
                const vec3 &p1, const vec3 &p2, float radius,
                std::vector<int> &neighbors, std::vector<float> &squared_distances,
                bool bToLine = true
        ) const;

        /**
         * \brief Queries the nearest neighbors within a cylinder range.
         * \details Searches for the nearest points whose distances to line segment \p p1 - \p p2 are smaller than
         * \p radius (\b not squared).
         * \param p1 One end point of the query line segment.
         * \param p2 The other end point of the query line segment.
         * \param radius The search range (which is \b not squared).
         * \param neighbors The indices of the neighbors found.
         * \param bToLine Specifies how the points found are ordered. If \bToLine=true, the points found are ordered
         * by their distances to the line. Otherwise, they are ordered by their distances to \p p1.
         * \note The range is specified by radius (\b not squared) but \b squared distances are returned.
         */
        int find_points_in_cylinder(
                const vec3 &p1, const vec3 &p2, float radius,
                std::vector<int> &neighbors,
                bool bToLine = true
        ) const;
        /// @}


        /// @name Cone range search
        /// @{

        /**
         * \brief Queries the nearest neighbors within a cone.
         * \details Searches for the nearest points with a cone from \p p1 to \p p2.
         * \param eye The view point.
         * \param p1 One end point of a line segment.
         * \param p2 The other end point of a line segment.
         * \param angle_range The maximal angle in radian allowed between \p p1 - \p p and \p p1 - \p p2, where \p p
         * is a candidate point.
         * \param neighbors The indices of the neighbors found.
         * \param squared_distances The squared distances between the found neighbors to the query line segment.
         * The values are stored in accordance with their indices.
         * \param bToLine Specifies how the points found are ordered. If \bToLine=true, the points found are ordered
         * by their distances to the line. Otherwise, they are ordered by their distances to the view point \p eye.
         * \note The angle is specified in \b radian and \b squared distances are returned.
         */
        int find_points_in_cone(
                const vec3 &eye, const vec3 &p1, const vec3 &p2, float angle_range,
                std::vector<int> &neighbors, std::vector<float> &squared_distances,
                bool bToLine = true
        ) const;

        /**
         * \brief Queries the nearest neighbors within a cone.
         * \details Searches for the nearest points with a cone from \p p1 to \p p2.
         * \param eye The view point.
         * \param p1 One end point of a line segment.
         * \param p2 The other end point of a line segment.
         * \param angle_range The maximal angle in radian allowed between \p p1 - \p p and \p p1 - \p p2, where \p p
         * is a candidate point.
         * \param neighbors The indices of the neighbors found.
         * \param bToLine Specifies how the points found are ordered. If \p bToLine=true, the points found are ordered
         * by their distances to the line. Otherwise, they are ordered by their distances to the view point \p eye.
         * \note The angle is specified in \b radian and \b squared distances are returned.
         */
        int find_points_in_cone(
                const vec3 &eye, const vec3 &p1, const vec3 &p2, float angle_range,
                std::vector<int> &neighbors, bool bToLine = true
        ) const;
        /// @}

    protected:
        int points_num_;
        float *points_; // reference of the original point cloud data

        void *tree_;
    };

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_ETH_H


