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

#ifndef EASY3D_RENDERER_CLIPPING_PLANE_H
#define EASY3D_RENDERER_CLIPPING_PLANE_H

#include <easy3d/core/types.h>


namespace easy3d {

    class ManipulatedFrame;
    class Camera;
    class ShaderProgram;

    /**
     * \brief An implementation of a manipulable clipping plane for visualization of model interiors and cross sections.
     * \details It also provides functions for visualizing the clipping plane itself.
     *
     * \class ClippingPlane easy3d/renderer/clipping_plane.h
     */
    class ClippingPlane {
    public:
        ClippingPlane();
        virtual ~ClippingPlane();

        /// returns whether the clipping plane has been enabled.
        bool is_enabled() const { return enabled_; }
        /// enables/disables the clipping plane.
        void set_enabled(bool b) { enabled_ = b; }

        /// puts the clipping plane at the center of the scene.
        void fit_scene(const vec3 &center, float radius);

        /// the center of the clipping plane. \sa normal()
        vec3 center() const;
        /// the normal direction of the clipping plane. \sa center().
        vec3 normal() const;

        /// the clipping plane
        Plane3 plane0() const;
        /// the other clipping plane. plane0() and plane1() together define a cross section.
        Plane3 plane1() const;

        /// returns whether the clipping plane is visible.
        bool is_visible() const { return visible_; }
        /// shows/hides the clipping plane.
        void set_visible(bool v) { visible_ = v; }

        /// returns the color of the clipping plane.
        const vec4& color() const { return color_; }
        /// sets the color of the clipping plane.
        void set_color(const vec4& c) { color_ = c; }

        /// returns whether cross section is enabled.
        bool cross_section() const { return cross_section_; }
        /// enables/disables cross section.
        void set_cross_section(bool b) { cross_section_ = b; }

        /// returns the thickness of the cross section.
        /// \details The thickness is defined relative to the scene bounding box. Default value is 0.01.
        float cross_section_width() { return cross_section_width_; }
        /// sets the thickness of the cross section.
        void set_cross_section_width(float w) { cross_section_width_ = w; }

        /// setups the shader program for rendering.
        /// \param program The shader program.
        /// \param plane_clipping_discard Controls clipping plane behavior.
        ///     - true: completely discard a vertex in vertex shader
        ///     - false: standard plane clipping
        void set_program(ShaderProgram *program, bool plane_clipping_discard = false);

        /// draws the clipping plane itself.
        void draw(Camera* cam) const;

        /// returns the manipulated frame bound to the clipping plane.
        virtual ManipulatedFrame *manipulated_frame();
        /// returns the manipulated frame bound to the clipping plane.
        virtual const ManipulatedFrame *manipulated_frame() const;

    protected:
        ManipulatedFrame *manipulated_frame_;
        bool enabled_;
        bool visible_;
        vec4 color_;
        bool cross_section_;
        float cross_section_width_;
        float scene_radius_;
    };

}

#endif // EASY3D_RENDERER_CLIPPING_PLANE_H
