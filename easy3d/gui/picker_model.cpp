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


#include <easy3d/gui/picker_model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/framebuffer_object.h>


namespace easy3d {

    ModelPicker::ModelPicker(Camera *cam) : Picker(cam) {
        use_gpu_ = true;
    }


    ModelPicker::~ModelPicker() {
        if (fbo_) {
            delete fbo_;
            fbo_ = nullptr;
        }
    }


    Model *ModelPicker::pick(const std::vector<Model *> &models, int x, int y) {
        if (models.empty())
            return nullptr;

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        int width = viewport[2];
        int height = viewport[3];

        // prepare a frame buffer object (fbo), I will do offscreen rendering to the new fbo
        if (!fbo_) {
            fbo_ = new FramebufferObject(width, height, 0);
            fbo_->add_color_buffer();
            fbo_->add_depth_buffer();
        }
        fbo_->ensure_size(width, height);

        //--------------------------------------------------------------------------
        // render the 'scene' into the new FBO.

        // TODO: the performance can be improved. Since the 'scene' is static, we need to render it to the fbo only
        //       once. Then just query. Re-rendering is needed only when the scene is changed/manipulated, or the canvas
        //       size is changed.

        // Bind the offscreen fbo for drawing
        fbo_->bind();

        float color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        states_.clear();
        draw(models);

        // --- Maybe this is not necessary ---------
        glFlush();
        glFinish();
        // -----------------------------------------

        // GLFW (same as Qt) uses upper corner for its origin while GL uses the lower corner.
        int glx = x;
        int gly = camera()->screenHeight() - 1 - y;

        // NOTE: when dealing with OpenGL, we alway work in the highdpi screen space
#if defined(__APPLE__)
        glx = static_cast<int>(glx * 2);
        gly = static_cast<int>(gly * 2);
#endif

        unsigned char c[4];
        fbo_->read_color(c, glx, gly);

        // switch back to the previous fbo
        fbo_->release();

        restore(models);

        // restore the clear color
        glClearColor(color[0], color[1], color[2], color[3]);
        easy3d_debug_gl_error;
        easy3d_debug_frame_buffer_error;

        //--------------------------------------------------------------------------

        // Convert the color back to an integer ID
        int id = rgb::rgba(c[0], c[1], c[2], c[3]);
        if (id >= 0 && id < models.size()) {
            //std::cout << "selected model " << models[id]->name() << std::endl;
            return models[id];
        }

        return nullptr;
    }


    // draw the scene
    void ModelPicker::draw(const std::vector<Model *> &models) {
        for (std::size_t i = 0; i < models.size(); ++i) {
            Model *model = models[i];
            if (!model->is_visible())
                continue;

            // the color used to render this model
            int r, g, b, a;
            rgb::encode(static_cast<int>(i), r, g, b, a);
            const vec4 color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

            if (dynamic_cast<SurfaceMesh *>(model))
                draw(model->triangles_drawable("faces"), color);
            else if (dynamic_cast<PointCloud *>(model))
                draw(model->points_drawable("vertices"), color);
            else if (dynamic_cast<Graph *>(model)) {
                draw(model->points_drawable("vertices"), color);
                draw(model->lines_drawable("edges"), color);
            }
        }
    }


    // draw a drawable
    void ModelPicker::draw(Drawable *drawable, const vec4 &color) {
        // record
        State state;
        state.lighting_ = drawable->lighting();
        state.per_vertex_color_ = drawable->per_vertex_color();
        state.default_color_ = drawable->default_color();
        states_[drawable] = state;

        // change
        drawable->set_lighting(false);
        drawable->set_per_vertex_color(false);
        drawable->set_default_color(color);

        // render
        drawable->draw(camera(), false);
    }


    void ModelPicker::restore(const std::vector<Model *> &models) {
        for (std::size_t i = 0; i < models.size(); ++i) {
            Model *model = models[i];
            if (!model->is_visible())
                continue;
            if (dynamic_cast<SurfaceMesh *>(model))
                restore(model->triangles_drawable("faces"));
            else if (dynamic_cast<PointCloud *>(model))
                restore(model->points_drawable("vertices"));
            else if (dynamic_cast<Graph *>(model)) {
                restore(model->points_drawable("vertices"));
                restore(model->lines_drawable("edges"));
            }
        }
    }


    void ModelPicker::restore(Drawable *drawable) {
        const State &state = states_[drawable];
        drawable->set_lighting(state.lighting_);
        drawable->set_per_vertex_color(state.per_vertex_color_);
        drawable->set_default_color(state.default_color_);
    }

}
