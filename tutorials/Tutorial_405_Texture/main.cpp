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
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/texture.h>


using namespace easy3d;


// This example shows how to
//		- create a texture from an image;
//		- render a textured quad using a texture.


int main(int /*argc*/, char** /*argv*/) {

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_405_Texture");
        viewer.camera()->setUpVector(vec3(0, 1, 0));
        viewer.camera()->setViewDirection(vec3(0, 0, -1));

        //--------------- create a mesh (which is a quad) -------------------
#if 1
        SurfaceMesh* mesh = new SurfaceMesh;
        auto texcoord = mesh->add_vertex_property<vec2>("v:texcoord");

        SurfaceMesh::Vertex v0 = mesh->add_vertex(vec3(   0,   0, 0)); texcoord[v0] = vec2(0, 0);
        SurfaceMesh::Vertex v1 = mesh->add_vertex(vec3(1070,   0, 0)); texcoord[v1] = vec2(1, 0);
        SurfaceMesh::Vertex v2 = mesh->add_vertex(vec3(1070, 245, 0)); texcoord[v2] = vec2(1, 1);
        SurfaceMesh::Vertex v3 = mesh->add_vertex(vec3(   0, 245, 0)); texcoord[v3] = vec2(0, 1);
        mesh->add_triangle(v0, v1, v2);
        mesh->add_triangle(v0, v2, v3);
        viewer.add_model(mesh);
#else
        const std::string file = "../../Easy3D/data/room.obj";
        easy3d::Model* mesh = viewer.open(file, true);
#endif

        //---------------- Load texture from an image file -------------------

        const std::string texture_file = "../../Easy3D/images/logo.jpg";
        Texture* tex = Texture::create(texture_file, GL_REPEAT);
        if (!tex) {
            std::cerr << "Error: failed to create texture. Please make sure the file exists and format is correct." << std::endl;
            return EXIT_FAILURE;
        }

        auto drawable = mesh->triangles_drawable("surface");
        drawable->set_texture(tex);

        // -----------------------------------------------------------------

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

