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
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/constraint.h>
#include <easy3d/renderer/transform.h>
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


}
