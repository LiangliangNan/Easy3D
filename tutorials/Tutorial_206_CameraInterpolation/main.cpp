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

#include "viewer.h"
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example Tutorial_206_CameraInterpolation
 * This example shows how to
 *      - create an exploration path using the key frame interpolator,
 *      - play the path as an animation.
 *
 * The base Viewer does not own a KeyFrameInterpolator: the subclass in viewer.cpp creates one bound
 * to the camera's frame, records the current view as a keyframe on 'k', starts/stops the timer-driven
 * playback on 'space', clears the path on 'd', and draws the path and keyframe cameras.
 *
 * \include{lineno} Tutorial_206_CameraInterpolation/main.cpp
 * The header file of the class:
 * \include{lineno} Tutorial_206_CameraInterpolation/viewer.h
 * The source file of the class:
 * \include{lineno} Tutorial_206_CameraInterpolation/viewer.cpp
 *
 * \see KeyFrameInterpolator, Camera, Frame, Viewer, Tutorial_205_RealCamera
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A simple model to fly around.
    const std::string &file_name = resource::directory() + "/data/sphere.obj";

    // Create the viewer subclass that adds keyframe/animation shortcuts.
    TutorialCameraInterpolation viewer(EXAMPLE_TITLE);

    // Load and add the model; the second argument (true) centers and scales it to fit.
    if (!viewer.add_model(file_name, true)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive render loop; returns only when the window is closed.
    return viewer.run();
}
