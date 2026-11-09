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

#include "viewer.h"
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_306_ImageViewer/main.cpp
 *
 * \brief A minimal 2D image viewer built on the Easy3D Viewer, with zoom, fit, and file loading.
 *
 * <b>Overview</b>
 * This tutorial turns the 3D Viewer into a simple image viewer: it displays a photograph in
 * screen space (no 3D camera involved), supports mouse-wheel zoom, 'f' to fit the image to
 * the window, Ctrl+O to open another image, and drag-and-drop of image files. It demonstrates
 * that the Viewer's overlay mechanism can render flat 2D content on both the OpenGL and
 * Vulkan backends. All the logic lives in the TutorialImageViewer class (shown after this
 * file); main() only picks the initial image and starts the viewer.
 *
 * <b>The technique</b>
 * An image is displayed by texture-mapping it onto a screen-space quad: four vertices whose
 * positions are framebuffer pixel coordinates and whose UVs cover the full [0,1] image
 * square. An orthographic "screen-space projection" maps pixel coordinates directly to clip
 * space, so the quad appears exactly where we place it, independent of any 3D camera. Zooming
 * simply rescales the quad; "fit to screen" chooses the largest scale that keeps the whole
 * image inside the window (the minimum of the width and height ratios, preserving aspect
 * ratio). The quad is rebuilt only when something changes (new image, zoom, resize) — a
 * "dirty" flag avoids redundant GPU uploads every frame. This is the same textured-quad idea
 * as Tutorial_305_Texture, but driven in pixel coordinates instead of world coordinates.
 *
 * <b>Easy3D API</b>
 * - Viewer: subclassed as TutorialImageViewer; the overrides used are key_press_event(),
 *   mouse_scroll_event(), drop_event(), post_resize(), and pre_draw().
 * - ImageIO::load(): decodes an image file into an RGBA byte buffer.
 * - graphics::Image / graphics::Sampler: an immutable GPU image plus its sampling state
 *   (linear filtering here, for smooth zooming).
 * - graphics::DrawCommand + Viewer::set_overlays(): submit the textured quad as a screen
 *   overlay; graphics::screen_space_projection() provides the pixel-to-clip-space matrix.
 *
 * <b>Running it</b>
 * The window opens showing data/fountain/images/0000.jpg fitted to the window. Scroll the
 * mouse wheel to zoom in/out, press 'f' to fit the image again, press Ctrl+O to open a
 * different image via a file dialog, or drag an image file onto the window. If the initial
 * image cannot be loaded, the constructor throws and the program terminates with an error.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_306_ImageViewer/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_306_ImageViewer/viewer.cpp
 *
 * \see Viewer, TutorialImageViewer, Tutorial_305_Texture
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // the image file.
    const std::string image_file = resource::directory() + "/data/fountain/images/0000.jpg";

    // The custom viewer loads the image in its constructor (throwing on failure) and fits
    // it to the window, so nothing else needs to be set up here.
    TutorialImageViewer viewer(EXAMPLE_TITLE, image_file);
    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
