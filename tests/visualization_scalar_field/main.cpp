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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/timer.h>

using namespace easy3d;


int test_scalar_field(int duration) {
    const std::string file_name = resource::directory() + "/data/building.off";

    Viewer viewer("ScalarField");

    // Load a mesh model to the viewer
    SurfaceMesh *model = dynamic_cast<SurfaceMesh *>(viewer.add_model(file_name, false));
    if (!model) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    // Add a TrianglesDrawable to visualize the surface.
    auto drawable = model->renderer()->add_triangles_drawable("faces");

    // By default, Easy3D renders the model using either a uniform color, or a per-face/vertex color given in the
    // model file. In this tutorial, we define a scalar field on the mesh vertices: elevation (here the Z-component
    // of each vertex). The visualization is done by mapping the scalar value to a colormap.
    auto elevation = model->add_vertex_property<float>("v:elevation");
    for (auto v : model->vertices())
        elevation[v] = model->position(v).z;

    drawable->set_scalar_coloring(State::VERTEX, "v:elevation", nullptr, 0.0f, 0.0f);

    // Create texture for coloring the scalar field.
    const std::string texture_file = resource::directory() + "/colormaps/rainbow.png";
    Texture *texture = TextureManager::request(texture_file);
    if (!texture) {
        LOG(ERROR) << "failed to create texture.";
        return EXIT_FAILURE;
    }

    // Use the texture
    drawable->set_texture(texture);

    viewer.set_usage("testing scalar field...");

    Timer<>::single_shot(duration, (Viewer*)&viewer, &Viewer::exit);
    return viewer.run();
}

