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


#include "viewer.h"
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>


namespace easy3d {

    FigureMaker::FigureMaker(int rows, int cols, const std::string &title)
        : MultiViewer(rows, cols, title)
    {
    }

    Model* FigureMaker::add_model(const std::string& file_name, bool create_default_drawables) {
        auto model = Viewer::add_model(file_name, create_default_drawables);
        if (model) {
            if (views_[0][0].models.empty())
                assign(0, 0, model);
            else {
                set_layout(num_rows_, num_cols_ + 1);
                assign(num_rows_ - 1, num_cols_ - 1, model);
            }
            std::cout << "model [" << num_cols_ - 1 << "]: " << file_name << std::endl;
        }

        return model;
    }

    bool FigureMaker::key_press_event(int key, int modifiers) {
        if (key == KEY_D)
            set_division_visible(!division_visible_);
        else if (key == KEY_L) {
            for (const auto model : models_) {
                const auto& tri_drawables = model->renderer()->triangles_drawables();
                for (auto d : tri_drawables)    d->set_lighting(!d->lighting());
                const auto& pts_drawables = model->renderer()->points_drawables();
                for (auto d : pts_drawables)    d->set_lighting(!d->lighting());
                const auto& lin_drawables = model->renderer()->lines_drawables();
                for (auto d : lin_drawables)    d->set_lighting(!d->lighting());
            }
        }
        else
            return Viewer::key_press_event(key, modifiers);

        update();
        return false;
    }

}
