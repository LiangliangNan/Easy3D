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

#include <easy3d/viewer/clipping_plane.h>
#include <easy3d/viewer/manipulated_frame.h>
#include <easy3d/viewer/transform.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/constraint.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include "setting.h"


namespace easy3d {


    ClippingPlane::ClippingPlane()
            : manipulated_frame_(0), size_(1.0f), enabled_(false), cross_section_(false),
              cross_section_width_(0.001f) {
    }


    ClippingPlane::~ClippingPlane() {
        if (manipulated_frame_)
            delete manipulated_frame_;
    }


    ManipulatedFrame *ClippingPlane::manipulated_frame() {
        if (manipulated_frame_ == 0) {
            manipulated_frame_ = new ManipulatedFrame;

            static LocalConstraint constaint;
            //constaint.setTranslationConstraintType(AxisPlaneConstraint::AXIS);
            constaint.setTranslationConstraint(AxisPlaneConstraint::AXIS, vec3(0, 0, 1));
            manipulated_frame_->setConstraint(&constaint);
        }
        return manipulated_frame_;
    }


    const ManipulatedFrame *ClippingPlane::manipulated_frame() const {
        return const_cast<ClippingPlane *>(this)->manipulated_frame();
    }


    void ClippingPlane::fit_scene(const vec3 &center, float radius) {
        size_ = radius;
        //manipulated_frame()->setPositionAndOrientation(center_, quat());
        manipulated_frame()->setPosition(center); // keep the orientation
    }


    Plane3 ClippingPlane::plane0() const {
        return Plane3(center(), normal());
    }


    Plane3 ClippingPlane::plane1() const {
        const vec3 &n = normal();
        return Plane3(center() + cross_section_width_ * size_ * n, -n);
    }


    vec3 ClippingPlane::center() const {
        return manipulated_frame()->position();
    }


    vec3 ClippingPlane::normal() const {
        const mat4 &CS = manipulated_frame()->matrix();
        return transform::normal_matrix(CS) * vec3(0, 0, 1);
    }


    void ClippingPlane::set_program(ShaderProgram *program) {
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
        easy3d_debug_log_gl_error;
        program->set_uniform("crossSectionEnabled", cross_section_);
        easy3d_debug_log_gl_error;
        program->set_uniform("clippingPlane0", plane0());
        easy3d_debug_log_gl_error;
        program->set_uniform("clippingPlane1", plane1());
        easy3d_debug_log_gl_error;
    }


    void ClippingPlane::draw(Camera *cam) const {
        if (!enabled_)
            return;

        // To use the standard drawable to visualize the clipping plane, I have to temporally disable clipping plane
        // to avoid the clipping plane being clipped by itself.
        bool status = enabled_;
        const_cast<ClippingPlane*>(this)->set_enabled(false);

        std::vector<vec3> corners = {
                vec3(-size_, -size_, 0),
                vec3(size_, -size_, 0),
                vec3(size_, size_, 0),
                vec3(-size_, size_, 0)
        };
        static std::vector<unsigned int> face_indices = {0, 1, 2, 0, 2, 3};
        static std::vector<unsigned int> wire_indices = {0, 1, 1, 2, 2, 3, 3, 0};

        std::vector<vec3> points(4);
        const mat4 &m = manipulated_frame()->matrix();
        for (std::size_t i = 0; i < 4; ++i)
            points[i] = m * corners[i];

        // draw the wireframe of the clipping plane
        LinesDrawable wireframe;
        wireframe.update_vertex_buffer(points);
        wireframe.update_index_buffer(wire_indices);
        wireframe.set_line_width(1.0f);
        wireframe.set_default_color(vec4(1, 0, 0, 1));
        wireframe.draw(cam, false);  easy3d_debug_log_gl_error;

        // draw the face of the clipping plane
        //glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        TrianglesDrawable face;
        face.update_vertex_buffer(points);
        face.update_index_buffer(face_indices);
        face.set_default_color(vec4(1, 0, 0, 0.2));
        face.draw(cam, false);
        glDisable(GL_BLEND);    easy3d_debug_log_gl_error;

        const_cast<ClippingPlane*>(this)->set_enabled(status);
    }

}