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
#include <easy3d/renderer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_305_Texture/main.cpp
 *
 * \brief Loading an image as a texture and displaying it on a textured quad with UV coordinates.
 *
 * <b>Overview</b>
 * This tutorial shows the basic texture-mapping pipeline in Easy3D: load an image file into a
 * Texture, build a small mesh consisting of a single quad, assign per-vertex texture
 * coordinates (UVs), and tell the mesh's drawable to sample the texture. The result is the
 * Easy3D logo displayed on a flat rectangle — the "hello world" of texturing.
 *
 * <b>The technique</b>
 * Texture mapping wraps a 2D image around geometry. Each vertex carries a texture coordinate
 * (u, v) in the normalized range [0, 1] x [0, 1], addressing a position in the image
 * independent of its pixel resolution. During rasterization the GPU interpolates the UVs
 * across each triangle and samples the texture at the interpolated coordinate, so the image
 * appears "painted" onto the surface. Here the quad's four corners get the four corners of
 * the UV square — (0,0), (1,0), (1,1), (0,1) — which maps the whole image onto the quad
 * exactly once. To avoid distortion, the quad's world-space width/height are set to the
 * image's pixel dimensions, preserving its aspect ratio. The camera is aimed straight at the
 * XY plane (looking down -Z with Y up) so the quad is seen face-on, like an image viewer.
 * This is the same mechanism used for full 3D texture mapping (see Tutorial_308_TexturedMesh)
 * and for the colormap trick in Tutorial_303_ScalarField.
 *
 * <b>Easy3D API</b>
 * - TextureManager::request(): loads an image file into a cached Texture (same file requested
 *   twice returns the same object).
 * - SurfaceMesh: add_vertex_property<vec2>("v:texcoord") stores the per-vertex UVs;
 *   add_vertex()/add_quad() build the single-quad geometry.
 * - DrawableTriangles: set_texture_coloring(State::VERTEX, "v:texcoord", tex) switches the
 *   drawable to textured rendering using that UV property and texture.
 * - Camera: setUpVector()/setViewDirection() orient the view to face the quad.
 *
 * <b>Running it</b>
 * The window shows the Easy3D logo on a quad, viewed face-on. Standard Viewer navigation
 * applies ('h' for help); rotating reveals that the image lives on a flat piece of geometry.
 *
 * \see Texture, TextureManager, DrawableTriangles, SurfaceMesh, Tutorial_303_ScalarField, Tutorial_308_TexturedMesh
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

    // Aim the camera straight at the XY plane: Y is up and we look along -Z, so the quad
    // (which lies in the XY plane) is seen face-on, like a 2D image.
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, -1));

    //----------------------- Load texture from an image file ------------------------

    // TextureManager loads the image and caches the resulting GPU texture by file path.
    const std::string texture_file = resource::directory() + "/images/logo.jpg";
    Texture *tex = TextureManager::request(texture_file);
    if (!tex) {
        std::cerr << "failed to create texture. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    //--------------- create a mesh (which contains a single quad) -------------------

    auto mesh = new SurfaceMesh;
    // The UV property: one 2D texture coordinate per vertex, in the normalized [0,1] range.
    auto texcoord = mesh->add_vertex_property<vec2>("v:texcoord");

    // Use the image's pixel dimensions as the quad's size so the displayed aspect ratio
    // matches the image (no stretching).
    const auto w = static_cast<float>(tex->width());
    const auto h = static_cast<float>(tex->height());
    // create a quad face having an aspect ratio the same as the texture image
    // The four corners walk around the quad counter-clockwise; each is paired with the
    // matching corner of the UV square so the full image maps onto the quad exactly once.
    auto v0 = mesh->add_vertex(vec3(0, 0, 0));
    texcoord[v0] = vec2(0, 0); // bottom-left of the image
    auto v1 = mesh->add_vertex(vec3(w, 0, 0));
    texcoord[v1] = vec2(1, 0); // bottom-right
    auto v2 = mesh->add_vertex(vec3(w, h, 0));
    texcoord[v2] = vec2(1, 1); // top-right
    auto v3 = mesh->add_vertex(vec3(0, h, 0));
    texcoord[v3] = vec2(0, 1); // top-left
    mesh->add_quad(v0, v1, v2, v3);

    // add the model to the viewer and create the default drawable "faces".
    // The 'true' flag centers/scales the quad to fit the scene.
    viewer.add_model(std::shared_ptr<SurfaceMesh>(mesh), true);

    // set the texture of the default drawable "faces".
    // set_texture_coloring() switches the drawable from plain coloring to texture sampling:
    // State::VERTEX says the UVs come from the per-vertex property "v:texcoord".
    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
    drawable->set_texture(tex);
    drawable->set_texture_coloring(easy3d::State::VERTEX, "v:texcoord", tex);

    // -------------------------------------------------------------------------------

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
