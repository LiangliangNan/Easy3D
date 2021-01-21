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


#include <easy3d/renderer/manipulator.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/setting.h>


namespace easy3d {

    Manipulator::Manipulator(Model *model)
            : model_(model), indicator_(nullptr) {
        frame_ = new ManipulatedFrame;
        if (model_) {
            model_->set_manipulator(this);
            frame_->setPositionAndOrientation(model->bounding_box().center(), quat());
        }
    }


    Manipulator::~Manipulator() {
        delete frame_;
        delete indicator_;
    }


    void Manipulator::reset() {
        if (model_) {
            const vec3 &center = model_->bounding_box(true).center();
            frame()->setPositionAndOrientation(center, quat());

            if (indicator_) {
                delete indicator_;
                indicator_ = nullptr;
            }
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


    void Manipulator::draw_frame(Camera *cam) const {
        if (!model_)
            return;

        ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
        }
        if (!program)
            return;

        if (!indicator_) {
            float radius = model_->bounding_box(false).max_range() * 0.7f;
            const vec3 &center = model_->bounding_box().center();

            std::vector<vec3> points, colors;
            std::vector<unsigned int> indices;

            std::vector<vec3> points_xoy;   // xoy
            opengl::prepare_circle(radius, 50, points_xoy, indices);

            auto trans = mat4::translation(center);
            for (auto &p : points_xoy) {
                points.push_back(trans * p);
                colors.push_back(vec3(0, 0, 1));
            }
            colors.resize(points_xoy.size(), vec3(0, 0, 1));

            auto rot_x = mat4::rotation(vec3(1, 0, 0), M_PI * 0.5f);
            for (std::size_t i = 0; i < points_xoy.size(); ++i) {
                points.push_back(trans * rot_x * points_xoy[i]);
                colors.push_back(vec3(0, 1, 0));
                indices.push_back(points_xoy.size() + i);
                indices.push_back(points_xoy.size() + (i + 1) % points_xoy.size());
            }

            auto rot_y = mat4::rotation(vec3(0, 1, 0), M_PI * 0.5f);
            for (std::size_t i = 0; i < points_xoy.size(); ++i) {
                points.push_back(trans * rot_y * points_xoy[i]);
                colors.push_back(vec3(1, 0, 0));
                indices.push_back(points_xoy.size() * 2 + i);
                indices.push_back(points_xoy.size() * 2 + (i + 1) % points_xoy.size());
            }

            auto drawable = new LinesDrawable("frame");
            const_cast<Manipulator*>(this)->indicator_ = drawable;
            drawable->update_vertex_buffer(points);
            drawable->update_color_buffer(colors);
            drawable->update_element_buffer(indices);
            drawable->set_property_coloring(State::VERTEX);
        }

        program->bind();
        program->set_uniform("MVP", cam->modelViewProjectionMatrix())
                ->set_uniform("MANIP", matrix())
                ->set_uniform("per_vertex_color", true)
                ->set_uniform("clippingPlaneEnabled", false)
                ->set_uniform("selected", false);

        indicator_->gl_draw();
        program->release();
    }

}