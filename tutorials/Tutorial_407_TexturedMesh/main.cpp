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



#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/setting.h>

using namespace easy3d;


// This example shows how to
//		- load a textured mesh from an obj file using tinyobjloader;
//		- create drawables for rendering the texture mesh.


bool load_obj(const std::string& file_name, SurfaceMesh* mesh);


int main(int /*argc*/, char** /*argv*/) {
    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_407_TextureDMesh");
        viewer.camera()->setUpVector(vec3(0, 1, 0));
        viewer.camera()->setViewDirection(vec3(0, 0, -1));

        //----------------------- Load mesh model from a file ------------------------

        const std::string& file_name = easy3d::setting::resource_directory() + "/data/pine/scrubPine.obj";

        SurfaceMesh* mesh = new SurfaceMesh;
        if (!load_obj(file_name, mesh)) {
            std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
            return EXIT_FAILURE;
        }

        viewer.add_model(mesh, true, false);

        // -------------------------------------------------------------------------------

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

