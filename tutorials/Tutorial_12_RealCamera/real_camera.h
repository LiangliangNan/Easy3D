/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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

#include <easy3d/viewer.h>


// This RealCamera class visualizes a model from the view point
// given its (intrinsic and extrinsic) camera parameters.
// The camera parameters were recovered using computer vison
// techniques (i.e., SfM in this class).


class RealCamera : public easy3d::Viewer
{
public:
    RealCamera(const std::string& title = "");

protected:
    bool key_press_event(int key, int modifiers) override;

private:
    struct CameraPara {
        int w, h;           // image size
        float fx, fy;       // focal length
        float u0, v0;       // principal point
        float rx, ry, rz;   // rotation in angle-axis format (i.e., the 3 angles in radian)
        float tx, ty, tz;   // the translation
    };

    // converts CameraPara into a 3x4 projection matrix
    easy3d::mat34 compute_projection_matrix(const CameraPara& cam) const;

    // update the view
    void change_view(const CameraPara& cam);
};


#endif // EASY3D_TUTORIAL_REAL_CAMERA_H
