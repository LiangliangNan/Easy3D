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

#include <memory>

namespace easy3d {
    class KeyFrameInterpolator;
    class LinesDrawable;
    class TrianglesDrawable;
}

// This tutorial shows how to interpolate camera frames to
// animate model exploration.

/**
 * \brief A Viewer that records camera keyframes and plays them back as a smooth fly-through.
 * \details The base Viewer knows nothing about keyframe interpolation. This subclass owns a
 * KeyFrameInterpolator bound to the viewer's camera frame, records a keyframe on 'k', starts and
 * stops playback on 'space', deletes the path on 'd', and shows the path and keyframe cameras.
 * Both guides are scene drawables: the scene pass hides the back side of each glyph behind its
 * front side and the models occlude the guides, on both backends. See the tutorial's main.cpp
 * for the technique.
 */
class TutorialCameraInterpolation : public easy3d::Viewer {
public:
    explicit TutorialCameraInterpolation(const std::string &title);
    ~TutorialCameraInterpolation() override;

protected:
    bool key_press_event(int key, int modifiers) override;
    void pre_draw() override;

private:
    easy3d::KeyFrameInterpolator *interpolator_;

    // The keyframe cameras and the path as scene drawables, plus what they were built from:
    // they are rebuilt only when the keyframes, the projection or the guide sizes change.
    std::shared_ptr<easy3d::TrianglesDrawable> cameras_;
    std::shared_ptr<easy3d::LinesDrawable> path_;
    std::size_t cameras_keyframes_ = 0;
    std::size_t path_keyframes_ = 0;
    float cameras_width_ = 0.0f;
    float cameras_field_of_view_ = 0.0f;
    float cameras_aspect_ = 0.0f;
};


#endif // EASY3D_TUTORIAL_CAMERA_INTERPOLATION_H
