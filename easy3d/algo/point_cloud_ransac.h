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

#ifndef EASY3D_ALGO_POINT_CLOUD_RANSAC_H
#define EASY3D_ALGO_POINT_CLOUD_RANSAC_H


#include <set>
#include <vector>


namespace easy3d {

    class PointCloud;

    /// \brief Extract primitives from point clouds using RANSAC.   \par
    /// Usage example:
    ///     \code
    ///     PrimitivesRansac ransac;
    ///     ransac.add_primitive_type(PrimitivesRansac::PLANE);
    ///     int num = ransac.detect(cloud);
    ///     \endcode

    class PrimitivesRansac {
    public:
        enum PrimType { // values have to be exactly the same as in RANSAC
            PLANE = 0,
            SPHERE = 1,
            CYLINDER = 2,
            CONE = 3,
            TORUS = 4,
            UNKNOWN = -1
        };

    public:
        /// \brief Setup the primitive types to be extracted. This is done by adding the interested primitive type one by one.
        void add_primitive_type(PrimType t);

        /// \brief Extract primitives from the entire point cloud. \par
        /// Returns the number of extracted primitives.
        /// The extracted primitives are stored as properties:
        ///      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
        ///      - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
        ///        primtive_type must be UNKNOWN.
        int detect(
                PointCloud *cloud,
                unsigned int min_support = 1000,    // the minimal number of points required for a primitive
                float dist_thresh = 0.005f,    // relative to the bounding box width. NOTE: Internally the distance threshold is taken as 3 * distance_threshold!!!
                float bitmap_reso = 0.02f,    // relative to the bounding box width. NOTE: This threshold is NOT multiplied internally!
                float normal_thresh = 0.8f,    // the cos of the maximal normal deviation
                float overlook_prob = 0.001f    // the probability with which a primitive is overlooked
        );

        /// \brief Extract primitives from a subset of a point cloud.
        /// Returns the number of extracted primitives.
        /// The extracted primitives are stored as properties:
        ///      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
        ///      - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
        ///        primtive_type must be UNKNOWN.
        int detect(
                PointCloud *cloud,
                const std::vector<int> &vertitces,
                unsigned int min_support = 1000,    // the minimal number of points required for a primitive
                float dist_thresh = 0.005f,    // relative to the bounding box width. NOTE: Internally the distance threshold is taken as 3 * distance_threshold!!!
                float bitmap_reso = 0.02f,    // relative to the bounding box width. NOTE: This threshold is NOT multiplied internally!
                float normal_thresh = 0.8f,    // the cos of the maximal normal deviation
                float overlook_prob = 0.001f    // the probability with which a primitive is overlooked
        );

    private:
        std::set<PrimType> types_;
    };


    /**
     * \example Tutorial_603_Cloud_PlaneExtraction
     * The source file containing the main() function:
     * \include Tutorial_603_Cloud_PlaneExtraction/main.cpp
     * The header file of the class:
     * \include Tutorial_603_Cloud_PlaneExtraction/viewer.h
     * The source file of the class:
     * \include Tutorial_603_Cloud_PlaneExtraction/viewer.cpp
     */

}


#endif  //  EASY3D_ALGO_POINT_CLOUD_RANSAC_H
