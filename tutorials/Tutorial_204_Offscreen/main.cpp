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
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>

/**
 * \example{lineno} Tutorial_204_Offscreen/main.cpp
 *
 * \brief Rendering a model straight to an image file with no window, using the OffScreen renderer.
 *
 * <b>Overview</b>
 * Sometimes you want a picture of a scene without any user interaction — for batch processing,
 * automated thumbnails, or headless servers with no display. OffScreen provides the same scene
 * management and rendering pipeline as Viewer but draws into an offscreen framebuffer and
 * writes the result to an image file instead of presenting it in a window. This tutorial loads
 * the Stanford bunny, gives it a flat color, and saves a PNG.
 *
 * <b>The technique</b>
 * Offscreen rendering means the GPU draws into a framebuffer object (a texture-backed render
 * target) rather than the default on-screen framebuffer. The geometry, camera, and shading are
 * identical to interactive rendering; only the destination differs. After the draw, the pixels
 * are read back from the GPU and encoded to an image file. OffScreen subclasses Viewer
 * (privately) so it reuses add_model(), the camera, and the draw path, but it never opens a
 * window or runs an event loop — render() performs a single capture. Optional supersampling
 * (the samples argument) and scaling let you trade speed for image quality.
 *
 * <b>Easy3D API</b>
 * - OffScreen: construct it with a pixel width/height; add_model() loads geometry; render()
 *   draws the scene and saves it to the given filename, returning true on success.
 * - Renderer / TrianglesDrawable: get_triangles_drawable("faces") fetches the surface drawable;
 *   set_uniform_coloring() assigns a constant material color.
 *
 * <b>Running it</b>
 * This example is non-interactive: it prints "surface rendering saved to 'offscreen.png'" and
 * exits, leaving offscreen.png in the working directory. No window appears.
 *
 * \see OffScreen, Viewer, TrianglesDrawable, Tutorial_201_Viewer_default
 */

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    easy3d::initialize();

    // Create a headless renderer with an 800 x 600 pixel canvas (no window is opened).
    easy3d::OffScreen renderer(800, 600);

    // Load the bunny mesh; add_model() returns nullptr if the file is missing or invalid.
    const std::string filename = easy3d::resource::directory() + "/data/bunny.ply";
    auto *mesh = renderer.add_model(filename);
    if (!mesh) {
        std::cerr << "model not found or failed to load: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // Give the surface a flat light-blue material so the saved image has a uniform color.
    auto *faces = mesh->renderer()->get_triangles_drawable("faces", true);
    if (faces)
        faces->set_uniform_coloring(easy3d::vec4(0.7f, 0.85f, 1, 1));

    // Render one frame offscreen and write it to disk; render() returns true on success.
    if (renderer.render("offscreen.png")) {
        std::cout << "surface rendering saved to 'offscreen.png'" << std::endl;
        return EXIT_SUCCESS;
    }

    std::cerr << "offscreen rendering failed" << std::endl;
    return EXIT_FAILURE;
}
