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
#include "animation.h"
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_311_Animation/main.cpp
 *
 * \brief Animating a mesh with per-frame dynamic vertex-buffer updates, portable across Vulkan and OpenGL.
 *
 * <b>Overview</b>
 * This tutorial makes the Stanford bunny breathe: every frame its vertices are recomputed on
 * the CPU (stretched along Z by a slowly growing factor) and republished to the GPU through a
 * dynamic vertex buffer. No Viewer subclass is needed — the default Viewer's animation hook
 * (animation_func_) is enough. The small BunnyAnimation helper class below holds the animation
 * state; main() wires it into the viewer.
 *
 * <b>The technique</b>
 * An interactive viewer is an event loop: when animation is enabled it keeps requesting frames,
 * and before each frame it invokes a user callback on the UI thread. That callback is the
 * animation step — here it deforms the geometry and returns true to signal that the scene
 * changed and should be redrawn. The key rendering concept is the dynamic buffer: geometry that
 * changes every frame must be re-uploaded efficiently. In Easy3D the drawable keeps an editable
 * CPU copy of the vertex data; update_vertex_buffer(points, true) marks it dynamic and publishes
 * a new snapshot, and each backend (OpenGL or Vulkan) uploads that snapshot before rendering the
 * next frame — so the same animation code runs unchanged on both APIs. Note the animation always
 * recomputes positions from the original rest-pose vertices instead of modifying last frame's
 * result; repeatedly scaling an already-scaled mesh would accumulate floating-point roundoff
 * and drift. The callback is frame-based, not a fixed-rate clock, so the animation speed follows
 * the render rate.
 *
 * <b>Easy3D API</b>
 * - Viewer::set_animation(true): enables the continuous frame loop (F2 pauses/resumes it).
 * - Viewer::animation_func_: std::function<bool(Viewer*)> called on the UI thread before each
 *   renderable frame; returning true requests a redraw.
 * - TrianglesDrawable: update_vertex_buffer(points, dynamic) uploads positions (dynamic = true
 *   for per-frame updates); update_element_buffer() uploads the triangle connectivity once.
 * - resource::bunny_vertices / bunny_indices: built-in bunny geometry, so no file is needed.
 *
 * <b>Running it</b>
 * The bunny slowly stretches along the Z axis until the scale reaches 1.5, then snaps back to
 * its original shape and the cycle repeats. Press F2 to pause/resume the animation; the usual
 * navigation controls work while it runs.
 *
 * The animation helper class:
 * \include{lineno} Tutorial_311_Animation/animation.h
 *
 * \see Viewer, TrianglesDrawable, Tutorial_312_MultiThread
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    //-------------------------------------------------------------

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

    //-------------------------------------------------------------

    // We use the points and indices of the "bunny".
    const std::vector<vec3> &points = resource::bunny_vertices;
    // Each consecutive 3 indices represent a triangle.
    const std::vector<unsigned int> &indices = resource::bunny_indices;

    //-------------------------------------------------------------
    // Create a TrianglesDrawable to visualize the surface of the "bunny".
    auto surface = new TrianglesDrawable("faces");

    // Upload the vertex positions of the surface to the GPU.
    // The second argument is 'true' for dynamic update of the buffer data.
    surface->update_vertex_buffer(points, true);

    // Upload the vertex indices of the surface to the GPU. The indices represent how the vertices are connected to
    // form triangles.
    surface->update_element_buffer(indices);
    // Add the drawable to the viewer
    viewer.add_drawable(std::shared_ptr<TrianglesDrawable>(surface));

    // allow animation
    // With animation enabled, the viewer keeps rendering frames and calls animation_func_
    // before each one. F2 pauses/resumes the loop.
    viewer.set_animation(true);

    // Define an animation function to specify how vertex positions are updated.
    // In this trivial example, we stretch the model along the Z-axis.
    // The lambda captures the raw drawable pointer (the viewer holds the owning shared_ptr,
    // so it stays alive) and the animation state; it runs on the UI thread, so touching the
    // drawable here is safe.
    BunnyAnimation animation;
    viewer.animation_func_ = [surface, &animation](Viewer *) {
        // Keep editable geometry on the CPU. Updating the drawable publishes a
        // new snapshot; each backend uploads it before rendering the next frame.
        return animation.update(*surface);
    };

    // run the viewer
    return viewer.run();
}
