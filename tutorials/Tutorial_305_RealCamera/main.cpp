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

#include "real_camera.h"


// This example shows how to
//        - render a model from the view point given the camera's intrinsic and
//        extrinsic parameters. The camera parameters were recovered using
//        commonly used computer vison techniques (i.e., calibration, or SfM).

int main(int /*argc*/, char** /*argv*/) {
    // Create the viewer.
    
    // the bunder file (We use only the camera instric and extrinsic parameters).
    const std::string bundler_file = "../../Easy3D-Mapple/data/fountain/bundle.out";
    
    // the point cloud file.
    const std::string cloud_file = "../../Easy3D-Mapple/data/fountain/pointcloud.ply";
    
    RealCamera viewer("Tutorial_12_RealCamera",
                      bundler_file,
                      cloud_file);
    
    viewer.resize(960, 800);
    
    // Run the viewer
    viewer.run();
    
    return EXIT_SUCCESS;
}

