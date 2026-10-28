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
#if EASY3D_TUTORIAL_USES_VULKAN
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#else
#include <easy3d/gui/picker_model.h>
#endif
#include <easy3d/renderer/camera.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/setting.h>


using namespace easy3d;

// \cond

#if EASY3D_TUTORIAL_USES_VULKAN
TutorialModelPicker::TutorialModelPicker(const std::string& title,const VulkanViewerOptions& options)
        : TutorialViewer(title,options) {
#else
TutorialModelPicker::TutorialModelPicker(const std::string &title) : TutorialViewer(title) {
#endif
    // We always want to look at the front of the easy3d logo.
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->setUpVector(vec3(0, 1, 0));

    hint_ = "Press the left button to pick/unpick a model";
}


bool TutorialModelPicker::mouse_press_event(int x, int y, int button, int modifiers) {
#if EASY3D_TUTORIAL_USES_VULKAN
    const auto hit=(button==GLFW_MOUSE_BUTTON_LEFT && modifiers==0) ? pick(x,y) : PickResult{};
    auto* model=hit.model.get();
#else
    ModelPicker picker(camera());
    auto model = picker.pick(models(), x, y);
#endif
    if (model)
        mark(model);

    return TutorialViewer::mouse_press_event(x, y, button, modifiers);
}


void TutorialModelPicker::mark(easy3d::Model *model) {
    for (auto m : models()) {
        if (m.get() == model)
            m->renderer()->set_selected(!m->renderer()->is_selected());

        auto faces = m->renderer()->get_triangles_drawable("faces");
        if (!faces) continue;
        if (m->renderer()->is_selected())
            faces->set_uniform_coloring(vec4(1, 0, 0, 1.0f));
        else
            faces->set_uniform_coloring(setting::surface_mesh_faces_color);
    }
    std::cout << "picked model: " << file_system::simple_name(model->name()) << std::endl;
    update();
}

// \endcond