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

#ifndef EASY3D_ALGO_POINT_CLOUD_RANSAC_H
#define EASY3D_ALGO_POINT_CLOUD_RANSAC_H

#include <set>
#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;

    /**
     * \brief Extract primitives from point clouds using RANSAC.
     * \class PrimitivesRansac easy3d/algo/point_cloud_ransac.h
     * \details Detection requires a per-vertex \c v:normal property. Plane and cylinder parameters are available
     *          through get_planes() and get_cylinders(); other supported types are represented only by the
     *          per-vertex type and index properties.
     * Usage example:
     *  \code
     *      PrimitivesRansac ransac;
     *      ransac.add_primitive_type(PrimitivesRansac::PLANE);
     *      int num = ransac.detect(cloud);
     *  \endcode
     */
    class PrimitivesRansac {
    public:
        /// Primitive types accepted by the detector. UNKNOWN is the unassigned-point label, not a detection type.
        enum PrimType { // Do NOT modify the order!!! Values have to be exactly the same as in RANSAC
            PLANE = 0,      ///< Plane primitive
            SPHERE = 1,     ///< Sphere primitive
            CYLINDER = 2,   ///< Cylinder primitive
            CONE = 3,       ///< Cone primitive
            TORUS = 4,      ///< Torus primitive
            UNKNOWN = -1    ///< Unknown primitive
        };

    public:
        /**
         * \brief Set up the primitive types to be extracted.
         * \details Adds a type to the set of types considered by subsequent detect() calls. Duplicate additions
         *          have no effect. UNKNOWN does not enable a detector.
         * \param t Primitive type to include.
         */
        void add_primitive_type(PrimType t);
        /**
         * \brief Exclude a primitive type from detection.
         * \details Removes the type from the set considered by subsequent detect() calls; removing a type that
         *          is not present has no effect.
         * \param t Primitive type to remove.
         */
        void remove_primitive_type(PrimType t);

        /**
         * \brief Extract primitives from a point cloud.
         * \details Requires a point cloud with at least three vertices, a \c v:normal property, and at least one
         *          configured primitive type. On a valid call, replaces the point cloud's \c v:primitive_type
         *          and \c v:primitive_index properties:
         *      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
         *      - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
         *        primitive_type must be UNKNOWN).
         * \param cloud Input point cloud; must not be null.
         * \param min_support Minimum number of points required to accept a primitive.
         * \param dist_threshold Distance tolerance as a fraction of the scale derived from the point cloud bounds.
         * \param bitmap_resolution Bitmap resolution as a fraction of the same point-cloud scale.
         * \param normal_threshold Cosine threshold for normal deviation.
         * \param overlook_probability Probability parameter passed to the RANSAC detector.
         * \return Number of accepted primitives, or zero when prerequisites are not met.
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
         * \brief Extract primitives from a subset of a point cloud.
         * \details Detects only among \p vertices, whose entries must be valid indices into \p cloud. Requires a
         *          point cloud with at least three vertices, a \c v:normal property, and at least one configured
         *          primitive type. On a valid call, replaces the point cloud's \c v:primitive_type and
         *          \c v:primitive_index properties:
         *          - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
         *          - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus
         *            its primitive_type must be UNKNOWN).
         *          Plane and cylinder parameters can also be queried with get_planes() and get_cylinders().
         * \param cloud Input point cloud; must not be null.
         * \param vertices Indices of the subset of vertices to process.
         * \param min_support Minimum number of points required to accept a primitive.
         * \param dist_threshold Distance tolerance as a fraction of the point cloud's bounding-box-derived scale.
         * \param bitmap_resolution Bitmap resolution as a fraction of the same point-cloud scale.
         * \param normal_threshold Cosine threshold for normal deviation.
         * \param overlook_probability Probability parameter passed to the RANSAC detector.
         * \return Number of accepted primitives, or zero when prerequisites are not met.
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
        
        /**
         * \brief Information about a plane primitive.
         */
        struct PlanePrim {
            int primitive_index;       ///< the index of this plane (w.r.t. the entire list of detected primitives)
            std::vector<int> vertices; ///< the vertex indices (w.r.t. the point cloud) of this plane
            Plane3 plane;   ///< the plane representation
            vec3  position; ///< the position of the plane (represented by a point on the plane)
            vec3  normal;   ///< the normal of the plane
        };
        /**
         * \brief Get the accumulated plane parameters.
         * \return Reference to the detector-owned plane list, including each primitive's cloud vertex indices.
         *         A full-cloud detection clears the list before detecting; subset detections append to it.
         */
        const std::vector<PlanePrim>& get_planes() const { return plane_primitives_; }

        /**
         * \brief Information about a cylinder primitive.
         */
        struct CylinderPrim {
            int primitive_index;       ///< the index of this cylinder (w.r.t. the entire list of detected primitives)
            std::vector<int> vertices; ///< the vertex indices (w.r.t. the point cloud) of this cylinder
            float radius;       ///< the radius of the cylinder
            vec3  position;     ///< the position of the cylinder (represented by the center of the bottom circle)
            vec3  direction;    ///< the direction of the cylinder
        };
        /**
         * \brief Get the accumulated cylinder parameters.
         * \return Reference to the detector-owned cylinder list, including each primitive's cloud vertex indices.
         *         A full-cloud detection clears the list before detecting; subset detections append to it.
         */
        const std::vector<CylinderPrim>& get_cylinders() const { return cylinder_primitives_; }

    private:
        std::set<PrimType>      types_;

        std::vector<PlanePrim>      plane_primitives_;
        std::vector<CylinderPrim>   cylinder_primitives_;
    };

}


#endif  //  EASY3D_ALGO_POINT_CLOUD_RANSAC_H
