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


#ifndef EASY3D_ALGO_RANSAC_H
#define EASY3D_ALGO_RANSAC_H


#include <set>
#include <vector>


namespace easy3d {

    class PointCloud;

    // usage:
    //    PrimitivesRansac ransac;
    //    ransac.add_primitive_type(PrimitivesRansac::PLANE);
    //    int num = ransac.detect(cloud);

    class PrimitivesRansac
    {
    public:
        enum PrimType { // values have to be exactly the same as in RANSAC
            PLANE = 0,
            SPHERE = 1,
            CYLINDER = 2,
            CONE = 3,
            TORUS = 4
        };

    public:
        // add/remove the primitive type to be extracted
        void add_primitive_type(PrimType t);
        void remove_primitive_type(PrimType t);

        // extract primitive from the entire point cloud.
        // returns the nummber of extracted primitives.
        // result will store as properties:
        //      - "v:segment_label" (-1 indicating unknow)
        //      - "v:segment_index" (0, 1, 2...)
        int detect(
            PointCloud* cloud,
            unsigned int min_support = 1000,	// the minimal number of points required for a primitive
            float dist_thresh = 0.005f,	// relative to the bounding box width. NOTE: Internally the distance threshold is taken as 3 * distance_threshold!!!
            float bitmap_reso = 0.02f,	// relative to the bounding box width. NOTE: This threshold is NOT multiplied internally!
            float normal_thresh = 0.8f,	// the cos of the maximal normal deviation
            float overlook_prob = 0.001f	// the probability with which a primitive is overlooked
            );

        // extract primitive from a subset of a point cloud.
        // result will store as properties:
        //      - "v:segment_label" (-1 indicating unknown)
        //      - "v:segment_index" (-1 indicating unknown)
        int detect(
            PointCloud* cloud,
            const std::vector<int>& vertitces,
            unsigned int min_support = 1000,	// the minimal number of points required for a primitive
            float dist_thresh = 0.005f,	// relative to the bounding box width. NOTE: Internally the distance threshold is taken as 3 * distance_threshold!!!
            float bitmap_reso = 0.02f,	// relative to the bounding box width. NOTE: This threshold is NOT multiplied internally!
            float normal_thresh = 0.8f,	// the cos of the maximal normal deviation
            float overlook_prob = 0.001f	// the probability with which a primitive is overlooked
            );

    private:
        std::set<PrimType>	types_;
    };

}


#endif  //  EASY3D_ALGO_RANSAC_H
