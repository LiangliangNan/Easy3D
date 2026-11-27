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


#include <easy3d/gui/picker_model.h>

#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <stdexcept>


namespace easy3d {

    ModelPicker::ModelPicker(const Camera *cam) : Picker(cam) {}


    Model *ModelPicker::pick(const std::vector< std::shared_ptr<Model> > &models, int x, int y) {
        const int width = camera()->screen_width(), height = camera()->screen_height();
        if (models.empty() || x < 0 || y < 0 || x >= width || y >= height)
            return nullptr;
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
        const auto id = render_device().sample_pixel(width, height, commands, x, y).id.object;
        return id < models.size() ? models[id].get() : nullptr;
    }

}
