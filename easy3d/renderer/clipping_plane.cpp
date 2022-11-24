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

#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/constraint.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/util/setting.h>


namespace easy3d {


    ClippingPlane::ClippingPlane()
            : manipulator_(nullptr), enabled_(false), visible_(true),
              color_(setting::clipping_plane_color), cross_section_(false),
              cross_section_width_(0.001f), scene_radius_(1.0f)
    {
        manipulator_ = new Manipulator;
        static LocalConstraint constraint;
        //constraint.setTranslationConstraintType(AxisPlaneConstraint::AXIS);
        constraint.setTranslationConstraint(AxisPlaneConstraint::AXIS, vec3(0, 0, 1));
        manipulator_->frame()->setConstraint(&constraint);
    }


    ClippingPlane::~ClippingPlane() {
        delete manipulator_;
    }


    ClippingPlane* ClippingPlane::instance() {
        static ClippingPlane cp;
        return &cp;
    }


    void ClippingPlane::fit_scene(const vec3 &center, float radius) {
        scene_radius_ = radius;
        manipulator_->frame()->setPosition(center); // we change position only (and keep the orientation)
    }


    Plane3 ClippingPlane::plane0() const {
        return Plane3(center(), normal());
    }


    Plane3 ClippingPlane::plane1() const {
        const vec3 &n = normal();
        return Plane3(center() + cross_section_width_ * scene_radius_ * n, -n);
    }


    vec3 ClippingPlane::center() const {
        return manipulator_->frame()->position();
    }


    vec3 ClippingPlane::normal() const {
        const mat4 &CS = manipulator_->frame()->matrix();
        return transform::normal_matrix(CS) * vec3(0, 0, 1);
    }


    void ClippingPlane::set_program(ShaderProgram *program) const {
        if (enabled_) {
            glEnable(GL_CLIP_DISTANCE0);
            if (cross_section_)
                glEnable(GL_CLIP_DISTANCE1);
            else
                glDisable(GL_CLIP_DISTANCE1);
        } else {
            glDisable(GL_CLIP_DISTANCE0);
            glDisable(GL_CLIP_DISTANCE1);
        }

        program->set_uniform("clippingPlaneEnabled", enabled_);
        easy3d_debug_log_gl_error
        program->set_uniform("crossSectionEnabled", cross_section_);
        easy3d_debug_log_gl_error

#if 0
        program->set_uniform("clippingPlane0", plane0());
        program->set_uniform("clippingPlane1", plane1());
#else   // strange GLSL behavior (maybe a bug of my GPU driver)?
        const vec3& n = normal();
        const Plane3 plane_0(center(), -n);
        program->set_uniform("clippingPlane0", plane_0);
        const Plane3 plane_1(center() - cross_section_width_ * scene_radius_ * n, n);
        program->set_uniform("clippingPlane1", plane_1);
#endif
        easy3d_debug_log_gl_error
    }


    void ClippingPlane::set_discard_primitives(ShaderProgram *program, bool plane_clipping_discard) const {
        // don't discard vertices when visualizing cross-sections
        program->set_uniform("planeClippingDiscard", plane_clipping_discard && !cross_section_);
    }


    void ClippingPlane::draw(Camera *cam) const {
        if (!enabled_ || !visible_)
            return;

        // To use the standard drawable to visualize the clipping plane, I have to temporally disable clipping plane
        // to avoid the clipping plane being clipped by itself.
        bool status = enabled_;
        const_cast<ClippingPlane *>(this)->set_enabled(false);

        const float size = scene_radius_;
        std::vector<vec3> corners = {
                vec3(-size, -size, 0),
                vec3(size, -size, 0),
                vec3(size, size, 0),
                vec3(-size, size, 0)
        };
        static std::vector<unsigned int> face_indices = {0, 1, 2, 0, 2, 3};
        static std::vector<unsigned int> wire_indices = {0, 1, 1, 2, 2, 3, 3, 0};

        std::vector<vec3> points(4);
        const mat4 &m = manipulator_->frame()->matrix();
        for (std::size_t i = 0; i < 4; ++i)
            points[i] = m * corners[i];

        // draw the wireframe of the clipping plane
        LinesDrawable wireframe;
        wireframe.update_vertex_buffer(points);
        wireframe.update_element_buffer(wire_indices);
        wireframe.set_line_width(1.0f);
        wireframe.set_uniform_coloring(color_);
        wireframe.draw(cam);
        easy3d_debug_log_gl_error

        // draw the face of the clipping plane
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        TrianglesDrawable face;
        face.update_vertex_buffer(points);
        face.update_element_buffer(face_indices);
        face.set_uniform_coloring(color_);
        face.set_distinct_back_color(false);
        face.draw(cam);
        glDisable(GL_BLEND);
        easy3d_debug_log_gl_error

        const_cast<ClippingPlane *>(this)->set_enabled(status);
    }

}
