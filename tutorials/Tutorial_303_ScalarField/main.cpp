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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_303_ScalarField/main.cpp
 *
 * \brief Visualizing a scalar field on a mesh by mapping values to colors with a colormap.
 *
 * <b>Overview</b>
 * This tutorial renders a scalar field defined on the vertices of a surface mesh: the
 * elevation (Z coordinate) of each vertex of a building model. The values are turned into
 * colors through a colormap texture, producing the familiar "heatmap" look used to inspect
 * simulation results, measured quantities, or any per-vertex/per-face attribute.
 *
 * <b>The technique</b>
 * A scalar field assigns one number to each mesh element (here: one float per vertex).
 * To visualize it, the numbers are first normalized to the field's [min, max] range and then
 * used as a 1D texture coordinate into a colormap — a thin image whose pixels encode a color
 * ramp (e.g. blue for low values through green/yellow to red for high ones). The GPU samples
 * that ramp per fragment, and because the scalar is a vertex attribute the color interpolates
 * smoothly across each triangle. This "pseudocolor" mapping is the standard way to make
 * invisible quantities (temperature, pressure, error, height, ...) visible at a glance.
 * Easy3D implements the ramp as an ordinary Texture: a default colormap is created
 * automatically, and you can substitute any image (the commented block below loads
 * "colormaps/rainbow.png"). Optional clamp percentages trim outliers from the range so a few
 * extreme values do not wash out the color contrast; here both clamps are 0, i.e. the full
 * data range is used.
 *
 * <b>Easy3D API</b>
 * - SurfaceMesh: add_vertex_property<float>() attaches a named per-vertex attribute
 *   ("v:elevation"); position(v) gives a vertex's coordinates.
 * - DrawableTriangles: set_scalar_coloring(State::VERTEX, "v:elevation", ...) tells the "faces"
 *   drawable to color the surface from that property via a colormap texture. State::VERTEX
 *   means the scalar lives on vertices (State::FACE would use per-face values).
 * - TextureManager::request(): loads/caches a Texture from an image file (used in the
 *   optional colormap-customization block).
 *
 * <b>Running it</b>
 * The building is displayed colored by height: low parts take the low end of the colormap and
 * high parts the high end. Navigate with the standard Viewer controls ('h' for help). To try
 * a different colormap, uncomment the block that requests "rainbow.png" before running.
 *
 * \see DrawableTriangles, SurfaceMesh, Texture, TextureManager, Tutorial_304_VectorField, Tutorial_305_Texture
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    const std::string file_name = resource::directory() + "/data/building.off";

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

    // Load a mesh model to the viewer. add_model() also creates the default drawables
    // (including the "faces" triangles drawable we configure below).
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file_name));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // By default, Easy3D renders the model using either a uniform color, or a per-face/vertex color
    // if the file contains such information. In this tutorial, we define a scalar field on the mesh
    // vertices: elevation (i.e., the Z-component of each vertex). The visualization is done by mapping
    // the scalar value to a colormap.
    // Properties are Easy3D's way of attaching arbitrary per-element data to a mesh; the "v:"
    // prefix marks this one as a vertex property.
    auto elevation = mesh->add_vertex_property<float>("v:elevation");
    for (auto v : mesh->vertices())
        elevation[v] = mesh->position(v).z;

    // Tell the drawable to use the scalar property "v:elevation" to color the model.
    // State::VERTEX: the scalar is defined per vertex (colors interpolate across faces).
    // nullptr: use the automatically created default colormap texture.
    // The two 0.0f arguments disable lower/upper range clamping, so the full min-max range
    // of the data is mapped onto the colormap.
    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
    drawable->set_scalar_coloring(State::VERTEX, "v:elevation", nullptr, 0.0f, 0.0f);

    // A colormap is just a 1D texture, so any image can serve as one. The optional block
    // below replaces the default colormap with the bundled "rainbow" ramp.
    //    // A default scalar texture is automatically created.
    //    // However, you can choose a more suitable one using the code below:
    //    const std::string texture_file = resource::directory() + "/colormaps/rainbow.png";
    //    Texture *texture = TextureManager::request(texture_file);
    //    if (!texture) {
    //        std::cerr << "failed to create texture" << std::endl;
    //        return EXIT_FAILURE;
    //    }
    //    drawable->set_texture(texture); // Use the texture

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
