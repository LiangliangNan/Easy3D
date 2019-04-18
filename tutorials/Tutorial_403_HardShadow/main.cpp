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

#include "hard_shadow.h"
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/drawable.h>


// This example shows how to
//		- renders a scene with hard shadow using the shadow map technique


int main(int /*argc*/, char** /*argv*/) {
    // Create the viewer.
    TutorialHardShadow viewer("Tutorial_403_HardShadow");

    const std::string file = "../../Easy3D-Mapple/data/room.obj";
    easy3d::Model* model = viewer.open(file);
    if (model) {
        auto drawable = model->triangles_drawable("surface");
        drawable->set_default_color(easy3d::vec3(0.9f, 0.9f, 0.9f));
    }
    else
        std::cerr << "Error: failed load model." << std::endl;

	// Run the viewer
    viewer.run();

    return EXIT_SUCCESS;
}

