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

#ifndef EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
#define EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H

#include <easy3d/viewer/viewer.h>
#if EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/renderer/backends/vulkan/key_frame_interpolator.h>
#include <chrono>
#endif


namespace easy3d {
    class KeyFrameInterpolator;
}

// This tutorial shows how to interpolate camera frames to
// animate model exploration.

class TutorialCameraInterpolation : public easy3d::Viewer
{
public:
#if EASY3D_TUTORIAL_USES_VULKAN
    explicit TutorialCameraInterpolation(const std::string& title,const easy3d::VulkanViewerOptions& options = {});
#else
    explicit TutorialCameraInterpolation(const std::string& title);
#endif
    ~TutorialCameraInterpolation() override;
    
protected:
    bool key_press_event(int key, int modifiers) override;
#if EASY3D_TUTORIAL_USES_VULKAN
    void pre_draw() override;
#else
    bool draw() override;
#endif

protected:
    easy3d::KeyFrameInterpolator* interpolator_;
#if EASY3D_TUTORIAL_USES_VULKAN
private:
    easy3d::vulkan::KeyFrameInterpolatorRenderer path_renderer_;
    std::chrono::steady_clock::time_point next_frame_;
#endif
};


#endif // EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
