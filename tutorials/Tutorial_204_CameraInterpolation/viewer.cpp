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

#include "viewer.h"
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/core/model.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


CameraIntrepolation::CameraIntrepolation(const std::string& title)
    : Viewer(title)
{
    interpolator_ = new KeyFrameInterpolator(camera_->frame());
}


CameraIntrepolation::~CameraIntrepolation()
{
    delete interpolator_;
}


std::string CameraIntrepolation::usage() const {
    return ("------------ Camera Interpolation usage ---------- \n"
            "Press 'K' to add key frames\n"
            "Press 'Space' to start/stop the animation\n"
            "Press 'D' to delete the camera path\n"
            "-------------------------------------------------- \n");
}


bool CameraIntrepolation::key_press_event(int key, int modifiers)
{
    if (key == GLFW_KEY_K && modifiers == 0) {
        easy3d::Frame *frame = camera()->frame();
        interpolator_->add_keyframe(*frame);
        float dist = distance(camera_->sceneCenter(), frame->position());
        if (dist > camera_->sceneRadius())
            camera_->setSceneRadius(dist);
        std::cout << "Key frame added" << std::endl;
        return true;
    }
    else if (key == GLFW_KEY_SPACE && modifiers == 0) {
        if (interpolator_->is_interpolation_started()) {
            interpolator_->stop_interpolation();
            std::cout << "Animation stopped." << std::endl;
        }
        else {
            interpolator_->start_interpolation();
            std::cout << "Animation started." << std::endl;
        }
        return true;
    }
    else if (key == GLFW_KEY_D && modifiers == 0) {
        interpolator_->delete_path();
        // update scene bounding box
        Box3 box;
        for (auto m : models_)
            box.add_box(m->bounding_box());
        camera_->setSceneBoundingBox(box.min(), box.max());
        std::cout << "path deleted"<< std::endl;
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void CameraIntrepolation::draw() const {
    Viewer::draw();

    // shown only when it is not animating
    if (!interpolator_->is_interpolation_started())
        interpolator_->draw_path(camera(), camera()->sceneRadius() * 0.05f);
}
