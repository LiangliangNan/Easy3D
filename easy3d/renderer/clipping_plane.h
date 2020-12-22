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
     * \brief An implementation of clipping plane for visualization of model interiors and cross sections.
     *
     * \class ClippingPlane easy3d/renderer/clipping_plane.h
     */
    class ClippingPlane {
    public:
        ClippingPlane();
        virtual ~ClippingPlane();

        bool is_enabled() const { return enabled_; }
        void set_enabled(bool b) { enabled_ = b; }

        void fit_scene(const vec3 &center, float radius);

        vec3 center() const;
        vec3 normal() const;

        Plane3 plane0() const;    // clipping plane0
        Plane3 plane1() const;    // clipping plane1

        bool is_visible() const { return visible_; }
        void set_visible(bool v) { visible_ = v; }

        const vec4& color() const { return color_; }
        void set_color(const vec4& c) { color_ = c; }

        bool cross_section() const { return cross_section_; }
        void set_cross_section(bool b) { cross_section_ = b; }

        // relative to scene bounding box. Default value is 0.01;
        float cross_section_width() { return cross_section_width_; }
        void set_cross_section_width(float w) { cross_section_width_ = w; }

        // \param plane_clipping_discard Controls clipping plane behavior.
        //  - true: completely discard a vertex in vertex shader
        //  - false: standard plane clipping
        void set_program(ShaderProgram *program, bool plane_clipping_discard = false);

        // arrow = true: also draw an arrow in the center of the plane.
        void draw(Camera* cam) const;

        virtual ManipulatedFrame *manipulated_frame();
        virtual const ManipulatedFrame *manipulated_frame() const;

    protected:
        ManipulatedFrame *manipulated_frame_;
        bool enabled_;
        bool visible_;
        vec4 color_;
        bool cross_section_;
        float cross_section_width_;
        float size_;
    };

}

#endif // EASY3D_RENDERER_CLIPPING_PLANE_H
