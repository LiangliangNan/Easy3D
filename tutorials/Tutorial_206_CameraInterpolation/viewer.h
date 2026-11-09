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

#ifndef EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
#define EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H

#include <easy3d/viewer/viewer.h>

// This tutorial shows how to interpolate camera frames to
// animate model exploration.

/**
 * \brief A Viewer that records camera keyframes and plays them back as a smooth animation.
 *
 * The base Viewer already owns a KeyFrameInterpolator bound to the camera and can draw/play a
 * camera path; this subclass merely remaps a few friendly keys ('k', 'space', 'd') onto that
 * machinery by overriding key_press_event(). No extra rendering or state is needed, which is
 * why the destructor is defaulted.
 */
class TutorialCameraInterpolation : public easy3d::Viewer {
public:
    explicit TutorialCameraInterpolation(const std::string &title);
    ~TutorialCameraInterpolation() override = default;

protected:
    // Maps 'k' (add keyframe), 'space' (play/stop), and 'd' (delete path) to the interpolator.
    bool key_press_event(int key, int modifiers) override;
};

#endif // EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
