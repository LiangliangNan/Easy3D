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

#ifndef EASY3D_TESTS_REAL_CAMERA_H
#define EASY3D_TESTS_REAL_CAMERA_H

#include <easy3d/viewer/viewer.h>

// This class visualizes a model from the view point given the camera's intrinsic and extrinsic parameters.
// These camera parameters can (usually) be recovered using camera calibration or structure from motion techniques.

namespace easy3d {
    class Camera;
    class Texture;
    class LinesDrawable;
}

class RealCamera : public easy3d::Viewer
{
public:
    RealCamera(const std::string& title,
               const std::string& bundler_file,
               const std::string& cloud_file);

    void change_view();

private:
    void show_next_view();
    
private:
    struct CameraPara {
        int w, h;           // image size
        float fx, fy;       // focal length
        float cx, cy;       // principal point
        easy3d::mat3 R;     // rotation
        easy3d::vec3 t;     // the translation
    };
    std::vector<CameraPara> views_;
    int current_view_;
    
    bool read_bundler_file(const std::string& file_name);
    
    // K [R T] -> easy3d camera representation
    bool KRT_to_camera(int view_index, easy3d::Camera* c, bool ground_truth);

    void load_image();

    void post_draw() override;

private:
    easy3d::Texture* texture_;
    bool need_update_;
};


#endif // EASY3D_TESTS_REAL_CAMERA_H
