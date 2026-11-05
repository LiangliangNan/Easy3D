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


#include <easy3d/gui/picker_model.h>

#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/render_device.h>
#else
#include <easy3d/renderer/backends/opengl/query_state.h>
#include <easy3d/renderer/framebuffer_object.h>
#endif
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <stdexcept>


namespace easy3d {

    ModelPicker::ModelPicker(const Camera *cam) : Picker(cam) {
        use_gpu_if_supported_ = true;
    }


    Model *ModelPicker::pick(const std::vector< std::shared_ptr<Model> > &models, int x, int y) {
        const int width = camera()->screenWidth(), height = camera()->screenHeight();
        if (models.empty() || x < 0 || y < 0 || x >= width || y >= height)
            return nullptr;
#ifdef EASY3D_HAS_VULKAN
        if (models.size() >= UINT32_MAX)
            throw std::length_error("too many models for picking");
        std::vector<graphics::DrawCommand> commands;
        for (size_t i = 0; i < models.size(); ++i) {
            const auto* renderer = models[i] ? models[i]->renderer() : nullptr;
            if (!renderer || !renderer->is_visible())
                continue;
            for (const auto& drawable : renderer->triangles_drawables())
                if (drawable->is_visible()) commands.push_back(draw_command(drawable, unsigned(i)));
            for (const auto& drawable : renderer->lines_drawables())
                if (drawable->is_visible() && !dynamic_cast<const LinesDrawable2D*>(drawable.get()))
                    commands.push_back(draw_command(drawable, unsigned(i)));
            for (const auto& drawable : renderer->points_drawables())
                if (drawable->is_visible()) commands.push_back(draw_command(drawable, unsigned(i)));
        }
        if (commands.empty())
            return nullptr;
        const auto frame = render_device().render(width, height, commands);
        const auto id = frame.ids[size_t(y) * width + x].object;
        return id < models.size() ? models[id].get() : nullptr;
#else
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int framebuffer_width = viewport[2];
        const int framebuffer_height = viewport[3];
        if (framebuffer_width <= 0 || framebuffer_height <= 0)
            return nullptr;
        opengl::QueryState state;
        state.prepare(framebuffer_width, framebuffer_height);
        setup_framebuffer(framebuffer_width, framebuffer_height);
        if (!fbo_->bind())
            return nullptr;
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw(models);

        int gl_x, gl_y;
        screen_to_opengl(x, y, gl_x, gl_y, framebuffer_width, framebuffer_height);
        unsigned char rgba[4];
        if (!fbo_->read_color(rgba, gl_x, gl_y))
            return nullptr;
        const int id = color::encode(rgba[0], rgba[1], rgba[2], rgba[3]);
        return id >= 0 && size_t(id) < models.size() ? models[id].get() : nullptr;
#endif
    }

#ifndef EASY3D_HAS_VULKAN
    // draw the scene
    void ModelPicker::draw(const std::vector< std::shared_ptr<Model> > &models) {
        for (std::size_t i = 0; i < models.size(); ++i) {
            const auto* renderer = models[i] ? models[i]->renderer() : nullptr;
            if (!renderer || !renderer->is_visible())
                continue;

            // the color used to render this model
            int r, g, b, a;
            color::decode(static_cast<int>(i), r, g, b, a);
            const vec4 color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f);

            for (auto d : renderer->triangles_drawables()) {
                if (d->is_visible())    draw(d.get(), color);
            }
            for (auto d : renderer->lines_drawables()) {
                if (d->is_visible())    draw(d.get(), color);
            }
            for (auto d : renderer->points_drawables()) {
                if (d->is_visible())    draw(d.get(), color);
            }
        }
    }


    // draw a drawable
    void ModelPicker::draw(Drawable *drawable, const vec4 &color) {
        if (dynamic_cast<const LinesDrawable2D*>(drawable))
            return;
        const State original = drawable->state();
        drawable->set_lighting(false);
        drawable->set_selected(false);
        drawable->set_highlight_range({-1, -1});
        drawable->set_uniform_coloring(color);

        try {
            drawable->draw(camera());
        } catch (...) {
            drawable->set_state(original);
            throw;
        }
        drawable->set_state(original);
    }

#endif

}
