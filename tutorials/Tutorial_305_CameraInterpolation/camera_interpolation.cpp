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

#include "camera_interpolation.h"
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/manipulated_camera_frame.h>
#include <easy3d/viewer/key_frame_interpolator.h>
#include <easy3d/viewer/model.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


CameraIntrepolation::CameraIntrepolation(const std::string& title)
    : Viewer(title)
{
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
        easy3d::Frame* frame = camera()->frame();
        camera()->keyFrameInterpolator()->addKeyFrame(*frame);
        std::cout << "Key frame added" << std::endl;

        // update scene bounding box to make sure the path is within the view frustum
        float old_radius = camera()->sceneRadius();
        float candidate_radius = distance(camera()->sceneCenter(), frame->position());
        camera()->setSceneRadius(std::max(old_radius, candidate_radius));

        return true;
    }
    else if (key == GLFW_KEY_SPACE && modifiers == 0) {
        if (camera()->keyFrameInterpolator()->interpolationIsStarted()) {
            camera()->keyFrameInterpolator()->stopInterpolation();
            std::cout << "Animation stopped." << std::endl;
        }
        else {
            camera()->keyFrameInterpolator()->startInterpolation();
            std::cout << "Animation started." << std::endl;
        }
        return true;
    }
    else if (key == GLFW_KEY_D && modifiers == 0) {
        camera()->keyFrameInterpolator()->deletePath();

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
    if (!camera()->keyFrameInterpolator()->interpolationIsStarted())
        camera()->draw_paths();
}
