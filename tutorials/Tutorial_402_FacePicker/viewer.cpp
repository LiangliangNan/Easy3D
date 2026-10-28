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
#include <easy3d/gui/picker_surface_mesh.h>
#endif
#include <easy3d/renderer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>


using namespace easy3d;

// \cond

#if EASY3D_TUTORIAL_USES_VULKAN
TutorialFacePicker::TutorialFacePicker(const std::string& title,const VulkanViewerOptions& options)
        : TutorialViewer(title,options) {
#else
TutorialFacePicker::TutorialFacePicker(const std::string &title) : TutorialViewer(title) {
#endif
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));

    hint_ = "Click the left button on a face to pick it";
}


bool TutorialFacePicker::mouse_press_event(int x, int y, int button, int modifiers) {
#if EASY3D_TUTORIAL_USES_VULKAN
    if (button==GLFW_MOUSE_BUTTON_LEFT && modifiers==0) {
        const auto hit=pick(x,y);
        for (const auto& m:models()) for (const auto& d:m->renderer()->triangles_drawables()) d->set_highlight(false);
        auto* model=dynamic_cast<SurfaceMesh*>(hit.model.get());
        auto* drawable=dynamic_cast<TrianglesDrawable*>(hit.drawable.get());
        if (model && drawable && hit.face_index>=0) {
            const auto ranges=model->get_face_property<std::pair<int,int>>("f:triangle_range");
            if (ranges) {
                drawable->set_highlight_range(ranges[SurfaceMesh::Face(hit.face_index)]);
                drawable->set_highlight(true);
                std::cout << "picked face f" << hit.face_index << std::endl;
            }
        }
        update();
    }
#else
    auto model = dynamic_cast<SurfaceMesh *>(current_model());
    if (model) {
        SurfaceMeshPicker picker(camera());
        auto face = picker.pick_face(model, x, y);
        if (face.is_valid())  // picked a face
            std::cout << "picked face " << face << std::endl;

        // highlight the picked face in the model 
        // always treat the model as a general polygonal mesh.
        auto drawable = model->renderer()->get_triangles_drawable("faces");
        auto triangle_range = model->get_face_property<std::pair<int, int> >("f:triangle_range");
        if (triangle_range && face.is_valid()) {
            const auto& range = triangle_range[face];
            drawable->set_highlight_range(range);
            drawable->set_highlight(true);
        }
        else {
            drawable->set_highlight_range(std::make_pair(-1, -1));
            drawable->set_highlight(false);
        }

        LOG_IF(!triangle_range, ERROR) << "face property 'f:triangle_range' not defined";
    }

#endif
    return TutorialViewer::mouse_press_event(x, y, button, modifiers);
}


#if EASY3D_TUTORIAL_USES_VULKAN
Model* TutorialFacePicker::add_model(const std::string& file_name,bool create) {
#else
Model* TutorialFacePicker::add_model(const std::string& file_name) {
#endif
#if EASY3D_TUTORIAL_USES_VULKAN
    auto model = TutorialViewer::add_model(file_name,create);
#else
    auto model = TutorialViewer::add_model(file_name,true);
#endif
    if (!model) return nullptr;

    // make sure the edges are visible
    model->renderer()->get_lines_drawable("edges")->set_visible(true);

    return model;
}

// \endcond