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

#include "transparency.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/bunny.h>
#include <easy3d/util/logging.h>


using namespace easy3d;

// This example shows how to render a surface mesh with transparency effect using the following techniques
//		  - Average Color Blending
//		  - Dual Depth Peeling


// create a surface mesh from the vertices and face indices.
SurfaceMesh* bunny_mesh() {
    SurfaceMesh* mesh = new SurfaceMesh;
    mesh->set_name("bunny");

    for (const auto& p : data::bunny_vertices)
        mesh->add_vertex(p);

    for (std::size_t i=0; i<data::bunny_indices.size(); i+=3)
        mesh->add_triangle(SurfaceMesh::Vertex(data::bunny_indices[i]),
                           SurfaceMesh::Vertex(data::bunny_indices[i+1]),
                           SurfaceMesh::Vertex(data::bunny_indices[i+2]));
    return mesh;
}



int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize(argv[0]);

    try {
        // Create the viewer.
        TutorialTransparency viewer("Tutorial_505_Transparency");

        viewer.add_model(bunny_mesh(), true, false);

        // Run the viewer
        viewer.run();

        return EXIT_SUCCESS;

    } catch (const std::runtime_error &e) {
        LOG(ERROR) << "Caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

