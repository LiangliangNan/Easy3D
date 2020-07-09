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

#ifndef EASY3D_KD_TREE_SEARCH_ANN_H
#define EASY3D_KD_TREE_SEARCH_ANN_H

#include <easy3d/kdtree/kdtree_search.h>


namespace easy3d {

    class PointCloud;

    class KdTreeSearch_ANN : public KdTreeSearch  {
    public:
        KdTreeSearch_ANN();
        virtual ~KdTreeSearch_ANN();


        //______________ tree construction __________________________

        virtual void begin() ;
        virtual void add_point_cloud(PointCloud* cloud) ;
        virtual void end() ;

        //________________ closest point ____________________________

        // NOTE: *squared* distance is returned
        virtual int find_closest_point(const vec3& p, float& squared_distance) const ;

        virtual int find_closest_point(const vec3& p) const ;

        //_________________ K-nearest neighbors ____________________

        // NOTE: *squared* distances are returned
        virtual void find_closest_k_points(
            const vec3& p, int k,
            std::vector<int>& neighbors, std::vector<float>& squared_distances
            ) const ;

        virtual void find_closest_k_points(
            const vec3& p, int k,
            std::vector<int>& neighbors
            ) const ;

        //___________________ radius search __________________________

        // Liangliang: it seems ANN's annkFRSearch() needs to specify k.
        /* "It does two things. First, it computes the k nearest neighbors within the radius bound.
            Second, it returns the total number of points lying within the radius bound. It is
            permitted to set k = 0, in which case it only answers a range counting query. */
        void set_k_for_radius_search(int k) { k_for_radius_search_ = k; }

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius and *squared* distances are returned
        virtual void find_points_in_range(
            const vec3& p, float squared_radius,
            std::vector<int>& neighbors, std::vector<float>& squared_distances
        ) const;

        // fixed-radius search. Search for all points in the range.
        // NOTE: the range must be *squared* radius
        virtual void find_points_in_range(
            const vec3& p, float squared_radius,
            std::vector<int>& neighbors
        ) const;

    protected:
        int		points_num_;

        float**	points_; // a copy of the point cloud data (due to different data structure);

        void*	tree_;
        int		k_for_radius_search_;
    } ;

} // namespace easy3d

#endif  // EASY3D_KD_TREE_SEARCH_ANN_H


