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

#ifndef EASY3D_ALGO_POINT_CLOUD_RANSAC_H
#define EASY3D_ALGO_POINT_CLOUD_RANSAC_H


#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;

    /**
     * \brief Extract primitives from point clouds using RANSAC.
     * \class PrimitivesRansac easy3d/algo/point_cloud_ransac.h
     * Usage example:
     *  \code
     *      PrimitivesRansac ransac;
     *      ransac.add_primitive_type(PrimitivesRansac::PLANE);
     *      int num = ransac.detect(cloud);
     *  \endcode
     */
    class PrimitivesRansac {
    public:
        enum PrimType { // Do NOT modify the order!!! Values have to be exactly the same as in RANSAC
            PLANE = 0,
            SPHERE = 1,
            CYLINDER = 2,
            CONE = 3,
            TORUS = 4,
            UNKNOWN = -1
        };

    public:
        /**
         * \brief Setup the primitive types to be extracted.
         * \details This is done by adding the interested primitive types one by one.
         */
        void add_primitive_type(PrimType t);
        /**
         * \brief Exclude a primitive types to be extracted.
         * \details This is done by removing the primitive type from the existing list.
         */
        void remove_primitive_type(PrimType t);

        /**
         * \brief Extract primitives from a point cloud.
         * \details The extracted primitives are stored as properties:
         *      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
         *      - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
         *        primitive_type must be UNKNOWN.
         * \param cloud The input point cloud.
         * \param min_support The minimal number of points required for a primitive.
         * \param dist_threshold The distance threshold, defined relative to the bounding box's max dimension.
         * \param bitmap_resolution The bitmap resolution, defined relative to the bounding box width.
         * \param normal_threshold The cosine of the maximal normal deviation.
         * \param overlook_probability The probability with which a primitive is overlooked.
         * \return The number of extracted primitives.
         */
        int detect(
                PointCloud *cloud,
                unsigned int min_support = 1000,
                float dist_threshold = 0.005f,
                float bitmap_resolution = 0.02f,
                float normal_threshold = 0.8f,
                float overlook_probability = 0.001f
        );


        /**
         * \brief Extract primitives from a subset of a point cloud..
         * \details The extracted primitives are stored as properties:
         *      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
         *      - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
         *        primitive_type must be UNKNOWN.
         * \param cloud The input point cloud.
         * \param vertices The indices of the subset of the input point cloud.
         * \param min_support The minimal number of points required for a primitive.
         * \param dist_threshold The distance threshold, defined relative to the bounding box's max dimension.
         * \param bitmap_resolution The bitmap resolution, defined relative to the bounding box width.
         * \param normal_threshold The cosine of the maximal normal deviation.
         * \param overlook_probability The probability with which a primitive is overlooked.
         * \return The number of extracted primitives.
         */
        int detect(
                PointCloud *cloud,
                const std::vector<int> &vertices,
                unsigned int min_support = 1000,
                float dist_threshold = 0.005f,
                float bitmap_resolution = 0.02f,
                float normal_threshold = 0.8f,
                float overlook_probability = 0.001f
        );

        //Todo: implement storing parameters for spheres, toruses, and cones.
        
        // In addition to the primitive information (primitive type and index stored as per-vertex properties, 
        // the parameters of the detected primititives can be queried using the perspective functions.
        struct PlanePrim {
            int primitive_index;            // the index of this plane (w.r.t. the entire list of detected primitives)
            std::vector<int> vertices;      // the vertex indices (w.r.t. the point cloud) of this plane
            Plane3 plane;                   // the plane equation
            vec3  position; 
            vec3  normal;
        };
        const std::vector<PlanePrim>& get_planes() const { return plane_primitives_; }

        struct CylinderPrim {
            int primitive_index;            // the index of this cylinder w.r.t. the entire list of detected primitives
            std::vector<int> vertices;      // the vertex indices (w.r.t. the point cloud) of this cylinder
            float radius;
            vec3  position;
            vec3  direction;
        };
        const std::vector<CylinderPrim>& get_cylinders() const { return cylinder_primitives_; }

    private:
        std::set<PrimType>      types_;

        std::vector<PlanePrim>      plane_primitives_;
        std::vector<CylinderPrim>   cylinder_primitives_;
    };

}


#endif  //  EASY3D_ALGO_POINT_CLOUD_RANSAC_H
