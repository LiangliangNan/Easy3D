/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EASY3D_TUTORIAL_REAL_CAMERA_H
#define EASY3D_TUTORIAL_REAL_CAMERA_H

#include <easy3d/viewer/viewer.h>

// This RealCamera class visualizes a model from the view point
// given the camera's intrinsic and extrinsic parameters. These
// camera parameters can (usually) be recovered using camera
// calibration or structure from motion techniques.

namespace easy3d {
    class Camera;
}

class RealCamera : public easy3d::Viewer
{
public:
    RealCamera(const std::string& title,
               const std::string& bundler_file,
               const std::string& cloud_file);
    
protected:
    bool key_press_event(int key, int modifiers) override;
    
private:
    struct CameraPara {
        int w, h;           // image size
        float fx, fy;       // focal length
        float cx, cy;       // principal point
        float rx, ry, rz;   // rotation in angle-axis format
        float tx, ty, tz;   // the translation
    };
    std::vector<CameraPara> views_;
    std::size_t current_view_;
    
    bool read_bundler_file(const std::string& file_name);
    
    // K [R T] -> easy3d camera representation
    bool KRT_to_camera(std::size_t view_index, int method, easy3d::Camera* c);
    
    void create_cameras_drawable(float scale = 1.0f);
};


#endif // EASY3D_TUTORIAL_REAL_CAMERA_H
