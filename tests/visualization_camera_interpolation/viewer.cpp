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
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/core/model.h>


using namespace easy3d;


CameraIntrepolation::CameraIntrepolation(const std::string& title)
    : Viewer(title)
{
    interpolator_ = new KeyFrameInterpolator(camera_->frame());
    // update the viewer when the interpolation finishes
    easy3d::connect(&interpolator_->interpolation_stopped, (Viewer*)this, &Viewer::update);
}


CameraIntrepolation::~CameraIntrepolation()
{
    delete interpolator_;
}


bool CameraIntrepolation::key_press_event(int key, int modifiers)
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
            std::cout << "Animation stopped." << std::endl;
        }
        else {
            interpolator_->start_interpolation();
            if (interpolator_->is_interpolation_started())
                std::cout << "Animation started." << std::endl;
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
        std::cout << "Path deleted"<< std::endl;
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void CameraIntrepolation::draw() const {
    Viewer::draw();

    // shown only when it is not animating
    if (!interpolator_->is_interpolation_started()) {
        interpolator_->draw_cameras(camera(), camera()->sceneRadius() * 0.05f);
        interpolator_->draw_path(camera());
    }
}


void CameraIntrepolation::post_draw() {
    Viewer::post_draw();

    // draw usage hint
    if (texter_ && texter_->num_fonts() >=2) {
        const float font_size = 20.0f;
        const float offset = 20.0f * dpi_scaling();
        float y_pos = 50.0f;
        texter_->draw("Press 'K' to add current view as a keyframe.", offset, y_pos * dpi_scaling(), font_size, 1);
        y_pos += font_size;
        texter_->draw("Drag the left cursor to change the view and then add another keyframe.", offset, y_pos * dpi_scaling(), font_size, 1);
        y_pos += font_size;
        texter_->draw("After creating at least 3 keyframes, press 'Space' to start/stop the animation.", offset, y_pos * dpi_scaling(), font_size, 1);
        y_pos += font_size;
        texter_->draw("Press 'D' to delete the camera path.", offset, y_pos * dpi_scaling(), font_size, 1);
        y_pos += font_size;
        texter_->draw("Close the application when you finish the test.", offset, y_pos * dpi_scaling(), font_size, 1);
    }
}