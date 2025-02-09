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

#ifndef EASY3D_RENDERER_FRUSTUM_H
#define EASY3D_RENDERER_FRUSTUM_H


#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    /**
     * \brief A Frustum description for perspective projection.
     * \class Frustum easy3d/renderer/frustum.h
     */
    class Frustum
    {
    public:
        /**
         * \brief Types of projection.
         */
        enum ProjectionType {
            PERSPECTIVE,    ///< perspective projection
            ORTHO           ///< orthographic projection
        };

    public:
        /**
         * \brief Constructor with projection type.
         * \param type The type of projection.
         */
        explicit Frustum(ProjectionType type);

        /**
         * \brief Sets the orientation of the frustum.
         * \param pos The position of the frustum.
         * \param at The point the frustum is looking at.
         * \param up The up direction of the frustum.
         */
        void orient(const vec3 &pos, const vec3& at, const vec3& up);

        /**
         * \brief Sets the perspective projection parameters.
         * \param fovy The field of view in the y direction, in radians.
         * \param aspect The aspect ratio of the frustum.
         * \param znear The near clipping plane.
         * \param zfar The far clipping plane.
         */
        void set_perspective(float fovy, float aspect, float znear, float zfar);
        /**
         * \brief Sets the frustum parameters.
         * \param frustum_width The width of the frustum.
         * \param frustum_height The height of the frustum.
         * \param znear The near clipping plane.
         * \param zfar The far clipping plane.
         */
        void set_frustum(float frustum_width, float frustum_height, float znear, float zfar);
        /**
         * \brief Sets the orthographic projection parameters.
         * \param xmin The minimum x value.
         * \param xmax The maximum x value.
         * \param ymin The minimum y value.
         * \param ymax The maximum y value.
         * \param znear The near clipping plane.
         * \param zfar The far clipping plane.
         */
        void set_ortho(float xmin, float xmax, float ymin, float ymax, float znear, float zfar);

        /**
         * \brief Returns the view matrix.
         * \return The view matrix.
         */
        mat4 view_matrix() const;
        /**
         * \brief Returns the projection matrix.
         * \return The projection matrix.
         */
        mat4 projection_matrix() const;

        /**
         * \brief Returns the center point of the near plane in world space.
         * \return The center point of the near plane.
         */
        vec3  near_center() const;
        /**
         * \brief Returns the center point of the far plane in world space.
         * \return The center point of the far plane.
         */
        vec3  far_center() const;

        /**
         * \brief Returns the width of the frustum at the near plane.
         * \return The width of the near plane.
         */
        float near_width() const;	// frustum width at near plane
        /**
         * \brief Returns the height of the frustum at the near plane.
         * \return The height of the near plane.
         */
        float near_height() const;	// frustum height at near plane

        /**
         * \brief Returns the width of the frustum at the far plane.
         * \return The width of the far plane.
         */
        float far_width() const;	// frustum width at far plane
        /**
         * \brief Returns the height of the frustum at the far plane.
         * \return The height of the far plane.
         */
        float far_height() const;	// frustum height at far plane

        /**
         * \brief Returns the position of the frustum in world space.
         * \return The position of the frustum.
         */
        const vec3& position() const { return pos_; }
        /**
         * \brief Returns the up vector of the frustum in world space.
         * \return The up vector.
         */
        vec3 up_vector() const;
        /**
         * \brief Returns the right vector of the frustum in world space.
         * \return The right vector.
         */
        vec3 right_vector() const;

        /**
         * \brief Returns the distance to the near clipping plane.
         * \return The near clipping plane distance.
         */
        float near_distance() const { return near_; }
        /**
         * \brief Returns the distance to the far clipping plane.
         * \return The far clipping plane distance.
         */
        float far_distance() const { return far_; }

        /**
         * \brief Computes the 8 corner points of the frustum in world space.
         * \return A vector containing the 8 corner points.
         */
        std::vector<vec3> vertices() const;

    private:
        ProjectionType type_; ///< The type of projection

        vec3 pos_; ///< The position of the frustum
        vec3 at_;  ///< The point the frustum is looking at
        vec3 up_;  ///< The up direction of the frustum

        float near_, far_; ///< Near and far clipping planes
        float xmin_, xmax_, ymin_, ymax_; ///< Orthographic projection parameters
        float fovy_, ar_;  ///< Perspective projection parameters
    };

}


#endif // EASY3D_RENDERER_FRUSTUM_H
