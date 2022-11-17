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
#include <easy3d/gui/picker_model.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>


using namespace easy3d;


ModelPickerViewer::ModelPickerViewer(const std::string &title)
        : Viewer(title) {
    // We always want to look at the front of the easy3d logo.
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->setUpVector(vec3(0, 1, 0));
}


bool ModelPickerViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    ModelPicker picker(camera());
    auto model = picker.pick(models(), x, y);
    if (model)
        mark(model);

    return Viewer::mouse_press_event(x, y, button, modifiers);
}


void ModelPickerViewer::mark(easy3d::Model *model) {
    for (auto m : models()) {
        if (m == model)
            m->renderer()->set_selected(!m->renderer()->is_selected());

        auto faces = m->renderer()->get_triangles_drawable("faces");
        if (m->renderer()->is_selected())
            faces->set_uniform_coloring(vec4(1, 0, 0, 1.0f));
        else
            faces->set_uniform_coloring(setting::surface_mesh_faces_color);
    }
    std::cout << "picked model: " << file_system::simple_name(model->name()) << std::endl;
    update();
}


void ModelPickerViewer::post_draw() {
    Viewer::post_draw();

    // draw usage hint
    if (texter_ && texter_->num_fonts() >=2) {
        const float font_size = 20.0f;
        const float offset = 20.0f * dpi_scaling();
        float y_pos = 50.0f;
        texter_->draw("Click (using the mouse) on a model to pick/unpick it.", offset, y_pos * dpi_scaling(), font_size, 1);
        y_pos += font_size;
        texter_->draw("Close the application when you finish the test.", offset, y_pos * dpi_scaling(), font_size, 1);
    }
}