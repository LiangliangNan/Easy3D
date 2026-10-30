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

namespace {
const char* const usage_hint = "Press 'k' to add key frames\n"
            "Press 'space' to start/stop the animation\n"
            "Press 'd' to delete the camera path";
}

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/core/model.h>
#include <algorithm>


using namespace easy3d;

// \cond

#if EASY3D_TUTORIAL_USES_VULKAN
TutorialCameraInterpolation::TutorialCameraInterpolation(const std::string& title,const VulkanViewerOptions& options)
    : Viewer(title,options),interpolator_(new KeyFrameInterpolator(camera()->frame())) {
    hint_=usage_hint;
}
TutorialCameraInterpolation::~TutorialCameraInterpolation() {delete interpolator_;}
bool TutorialCameraInterpolation::key_press_event(int key,int modifiers) {
    if (key==KEY_K && modifiers==0) {
        if (!interpolator_->add_keyframe(*camera()->frame())) return false;
        camera()->setSceneRadius(std::max(camera()->sceneRadius(),distance(camera()->sceneCenter(),camera()->position())));
        update();return true;
    }
    if (key==KEY_SPACE && modifiers==0) {
        if (interpolator_->is_interpolation_started()) interpolator_->stop_interpolation();
        else {interpolator_->start_manual_interpolation();next_frame_=std::chrono::steady_clock::now();}
        update();return true;
    }
    if (key==KEY_D && modifiers==0) {
        interpolator_->delete_path();
        path_renderer_=vulkan::KeyFrameInterpolatorRenderer();
        Box3 box;for (const auto& model:models()) box.grow(model->bounding_box());
        if (box.is_valid()) camera()->setSceneBoundingBox(box.min_point(),box.max_point());
        set_overlays({});update();return true;
    }
    return Viewer::key_press_event(key,modifiers);
}
void TutorialCameraInterpolation::pre_draw() {
    if (interpolator_->is_interpolation_started()) {
        const auto now=std::chrono::steady_clock::now();
        if (now>=next_frame_) {
            interpolator_->advance_interpolation();
            next_frame_=now+std::chrono::milliseconds(interpolator_->interpolation_period());
        }
    }
    std::vector<graphics::DrawCommand> commands;
    if (!interpolator_->is_interpolation_started()) {
        auto cameras=path_renderer_.draw_cameras(*interpolator_,*camera(),camera()->sceneRadius()*.05f);
        auto path=path_renderer_.draw_path(*interpolator_,*camera());
        if (cameras.geometry) commands.push_back(cameras);
        if (path.geometry) commands.push_back(path);
    }
    set_overlays(std::move(commands));
}
#else
TutorialCameraInterpolation::TutorialCameraInterpolation(const std::string& title)
    : Viewer(title)
{
    interpolator_ = new KeyFrameInterpolator(camera_->frame());
    // update the viewer when the interpolation finishes
    easy3d::connect(&interpolator_->interpolation_stopped, (Viewer*)this, &Viewer::update);

    hint_ = usage_hint;
}


TutorialCameraInterpolation::~TutorialCameraInterpolation()
{
    delete interpolator_;
}


bool TutorialCameraInterpolation::key_press_event(int key, int modifiers)
{
    if (key == KEY_K && modifiers == 0) {
        easy3d::Frame *frame = camera()->frame();
        if (interpolator_->add_keyframe(*frame)) {
            float dist = distance(camera_->sceneCenter(), frame->position());
            if (dist > camera_->sceneRadius())
                camera_->setSceneRadius(dist);
            std::cout << "Key frame added" << std::endl;
            return true;
        }
        else
            return false;
    }
    else if (key == KEY_SPACE && modifiers == 0) {
        if (interpolator_->is_interpolation_started()) {
            interpolator_->stop_interpolation();
            std::cout << "Animation stopped" << std::endl;
        }
        else {
            interpolator_->start_interpolation();
            if (interpolator_->is_interpolation_started())
                std::cout << "Animation started" << std::endl;
        }
        return true;
    }
    else if (key == KEY_D && modifiers == 0) {
        interpolator_->delete_path();
        // update scene bounding box
        Box3 box;
        for (auto m : models_)
            box.grow(m->bounding_box());
        camera_->setSceneBoundingBox(box.min_point(), box.max_point());
        std::cout << "path deleted"<< std::endl;
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


bool TutorialCameraInterpolation::draw() {
    if (!Viewer::draw()) return false;

    // shown only when it is not animating
    if (!interpolator_->is_interpolation_started()) {
        interpolator_->draw_cameras(camera(), camera()->sceneRadius() * 0.05f);
        interpolator_->draw_path(camera());
    }
    return true;
}

// \endcond
#endif // EASY3D_TUTORIAL_USES_VULKAN
