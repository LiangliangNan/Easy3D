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
 * \example{lineno} Tutorial_205_RealCamera/main.cpp
 *
 * \brief Rendering a scene through a real calibrated camera and converting between pixels and 3D rays.
 *
 * <b>Overview</b>
 * This tutorial reconstructs the viewpoint of a real photograph. Given camera parameters
 * recovered by calibration or structure-from-motion (here read from a Bundler file), it places
 * the Easy3D camera so the 3D point cloud is seen exactly as in the original image, shows that
 * image as an overlay, and lets you move the cursor to project between image pixels and 3D
 * rays/points. The viewer subclass (TutorialRealCamera, shown after this file) does all the work.
 *
 * <b>The technique</b>
 * A pinhole camera is described by intrinsics K (focal length fx/fy, principal point cx/cy,
 * skew) and extrinsics [R|t] (rotation and translation from world to camera coordinates). A 3D
 * world point p projects to the image as x = K [R|t] p, with the result divided by its depth
 * (the perspective divide). Inverting this maps an image pixel back to a 3D ray through the
 * camera center. Two conventions must be reconciled: computer vision uses X right, Y down,
 * Z forward, while OpenGL/Easy3D uses X right, Y up, Z toward the viewer, so Y and Z are flipped
 * when converting. A common pitfall is treating t as the camera position; the true world-space
 * camera center is C = -R^{-1} t = -R^T t. The vertical field of view follows from the focal
 * length and image height: fov = 2 atan(h / (2 fy)). See Camera::set_from_calibration() and the
 * helper functions in TutorialRealCamera for the exact math.
 *
 * <b>Easy3D API</b>
 * - Viewer: subclassed to load the cloud, draw the camera frusta, and overlay the source image.
 * - Camera: set_from_calibration() builds the pose and projection from K, R, t; setPosition(),
 *   setOrientation(), and setFieldOfView() are used for the equivalent "ground truth" pose.
 * - LinesDrawable: draws the camera frusta and the back-projected 3D ray.
 * - PointCloud: the reconstructed 3D points loaded from the .ply file.
 *
 * <b>Running it</b>
 * Press 'space' to cycle through the recovered views (the window resizes to each image's aspect
 * ratio); '1'/'2' toggle calibration vs. ground-truth pose; 'h' shows/hides the camera frusta.
 * Move the cursor over the overlaid photo to see the corresponding 3D ray; move it over the 3D
 * scene to see where that point lands in the image.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_205_RealCamera/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_205_RealCamera/viewer.cpp
 *
 * \see Camera, TutorialRealCamera, LinesDrawable, Tutorial_206_CameraInterpolation
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The Bundler output holds the recovered camera intrinsics/extrinsics (we use only those).
    const std::string bundler_file = resource::directory() + "/data/fountain/bundle.out";
    // The reconstructed 3D point cloud that the cameras observed.
    const std::string cloud_file = resource::directory() + "/data/fountain/pointcloud.ply";

    // The subclass loads both files, builds the camera poses, and sets up the overlays.
    TutorialRealCamera viewer(EXAMPLE_TITLE, bundler_file, cloud_file);

    // Enter the interactive render loop; returns only when the window is closed.
    return viewer.run();
}
