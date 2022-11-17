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


#include <easy3d/renderer/manipulator.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>


namespace easy3d {

    Manipulator::Manipulator(Model *model)
            : model_(model), drawable_model_bbox_(nullptr) {
        frame_ = new ManipulatedFrame;
        if (model_) {
            model_->set_manipulator(this);
            frame_->setPositionAndOrientation(model->bounding_box().center(), quat());
        }
    }


    Manipulator::~Manipulator() {
        delete frame_;
        delete drawable_model_bbox_;
    }


    void Manipulator::reset() {
        if (model_) {
            const vec3 &center = model_->bounding_box(true).center();
            frame()->setPositionAndOrientation(center, quat());
        }
    }


    mat4 Manipulator::matrix() const {
        if (model_) {
            const mat4 &mat = frame()->matrix();

            // ------- the real transformation w.r.t. the center of the object -------
            // NOTE: The total transformation of the *frame* contains three parts:
            //  (1) an extra translation that moves the frame to the center of the object (for display).
            //  (2) a pure rotation
            //  (3) a pure translation (this is the real translation should be applied to the object).  
            // So we have to compensate the extra translation.
            const vec3 &center = model_->bounding_box().center();
            return mat * mat4::translation(-center);
        } else
            return mat4::identity();
    }


    void Manipulator::draw_frame(const Camera *cam) const {
        if (!model_ || !cam)
            return;

        if (!drawable_model_bbox_) {
            const_cast<Manipulator*>(this)->drawable_model_bbox_ = new LinesDrawable;
        }

        const Box3 box = model_->bounding_box(false);
        const vec3 &center = box.center();
        auto manip = matrix() * mat4::translation(center) * mat4::scale(box.range(0), box.range(1), box.range(2), 1.0f);
        shape::draw_box_wire(drawable_model_bbox_, cam->modelViewProjectionMatrix(), manip, true);
    }

}