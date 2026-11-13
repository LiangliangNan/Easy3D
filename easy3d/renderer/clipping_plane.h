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

#ifndef EASY3D_RENDERER_CLIPPING_PLANE_H
#define EASY3D_RENDERER_CLIPPING_PLANE_H

#include <easy3d/core/types.h>

namespace easy3d {

    class Camera;
    class ShaderProgram;
    class Manipulator;

    /**
     * \brief An implementation of a manipulable clipping plane for visualization of model interiors and cross sections.
     * \details It also provides functions for visualizing the clipping plane itself.
     *
     * \class ClippingPlane easy3d/renderer/clipping_plane.h
     */
    class ClippingPlane {
    public:
        /// Create the process-wide clipping-plane controller.
        ClippingPlane();

        /// Destroy the clipping-plane controller.
        virtual ~ClippingPlane();

        /// Return the process-wide clipping-plane instance.
        /// \return The singleton clipping plane.
        static ClippingPlane* instance();

        /// Return whether clipping is enabled.
        /// \return True when clipping is active.
        bool is_enabled() const { return enabled_; }
        /// Enable or disable clipping.
        /// \param b True to enable clipping.
        void set_enabled(bool b) { enabled_ = b; }

        /**
         * \brief Center the clipping plane on a scene bounding sphere.
         * \param center Scene center in world coordinates.
         * \param radius Scene radius in world units.
         */
        void fit_scene(const vec3 &center, float radius);
        /// radius used by the plane guide and relative cross-section width.
        /// \return Scene radius in world units.
        float scene_radius() const { return scene_radius_; }

        /// Return the clipping plane center in world coordinates.
        /// \return Plane center.
        vec3 center() const;
        /// Return the clipping plane normal in world coordinates.
        /// \return Plane normal.
        vec3 normal() const;

        /// Return the first clipping halfspace.
        /// \return First clipping plane.
        Plane3 plane0() const;
        /// Return the second halfspace used to define a cross section.
        /// \return Second clipping plane.
        Plane3 plane1() const;

        /// Return whether the plane guide is visible.
        /// \return True when the guide is shown.
        bool is_visible() const { return visible_; }
        /// Show or hide the plane guide without changing clipping.
        /// \param v True to show the guide.
        void set_visible(bool v) { visible_ = v; }

        /// Return the plane-guide color.
        /// \return Color in RGBA.
        const vec4& color() const { return color_; }
        /// Set the plane-guide color.
        /// \param c Color in RGBA.
        void set_color(const vec4& c) { color_ = c; }

        /// Return whether cross-section mode is enabled.
        /// \return True when cross-section mode is enabled.
        bool cross_section() const { return cross_section_; }
        /// Enable or disable cross-section mode.
        /// \param b True to enable cross-section mode.
        void set_cross_section(bool b) { cross_section_ = b; }

        /// returns the thickness of the cross section.
        /// \details The thickness is defined relative to the scene bounding box. Default value is 0.01.
        /// \return Relative cross-section thickness.
        float cross_section_width() const { return cross_section_width_; }
        /// Set cross-section thickness as a relative fraction of the scene radius.
        /// \param w Relative thickness.
        void set_cross_section_width(float w) { cross_section_width_ = w; }

        /// sets up the shader program for rendering.
        /// \param program Shader program to configure.
        void set_program(ShaderProgram *program) const;
        /// Configure whether a primitive is rejected when a vertex is clipped.
        /// \param program The shader program.
        /// \param plane_clipping_discard Controls clipping plane behavior.
        ///     - true: discard a primitive when any vertex is clipped.
        ///     - false: use standard geometric plane clipping.
        void set_discard_primitives(ShaderProgram *program, bool plane_clipping_discard) const;

        /// Draw the plane guide using the supplied camera.
        /// \param cam Camera used to compute the guide's screen-space representation.
        void draw(const Camera* cam) const;

        /// Return the manipulator attached to the clipping plane.
        /// \return Non-owning manipulator pointer.
        virtual Manipulator *manipulator() { return manipulator_; }
        /// Return the manipulator attached to the clipping plane.
        /// \return Non-owning manipulator pointer.
        virtual const Manipulator *manipulator() const { return manipulator_; }

    protected:
        Manipulator *manipulator_;
        bool enabled_;
        bool visible_;
        vec4 color_;
        bool cross_section_;
        float cross_section_width_;
        float scene_radius_;
    };

}

#endif // EASY3D_RENDERER_CLIPPING_PLANE_H
