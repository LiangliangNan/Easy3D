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
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_403_PointSelection/main.cpp
 *
 * \brief Selecting a subset of a point cloud by sketching a rectangle or a lasso with the mouse.
 *
 * <b>Overview</b>
 * This tutorial implements region selection on a point cloud: while holding Shift, the user
 * drags out a rectangle (key 'R') or a free-form lasso (key 'L'), and every point projecting
 * inside the region becomes selected (colored red). Dragging with the right button instead
 * deselects. Where Tutorial_401/402 pick a single element under the cursor, this tutorial
 * selects many elements at once — the standard "marquee/lasso" interaction of 3D editors.
 *
 * <b>The technique</b>
 * Region selection asks a different question than picking: not "what is the nearest element
 * at this pixel?" but "which elements project inside this 2D region?". Note that this is a
 * through-selection: points hidden behind other points are still selected, because selection
 * is defined by projection, not by visibility. Testing every point against the polygon on the
 * CPU would be O(n) per gesture with a serial point-in-polygon test; instead Easy3D evaluates
 * all points on the GPU in parallel (a vertex-shader/compute pass): each point is transformed
 * by the model-view-projection matrix, tested against the region and the clipping planes, and
 * the answer is packed into a bitmask — one bit per point, 32 points per 32-bit word — so only
 * a compact result is read back. The request carries the current selection as an input bitmask
 * so a deselect gesture can clear bits, and the returned words become the new selection.
 * While the gesture is in progress, the sketched region is drawn as a screen-space overlay:
 * the lasso interior is triangulated with the Tessellator (a polygon may be concave or
 * self-overlapping, so a simple fan would not do) and the outline is built from thin quads.
 *
 * <b>Easy3D API</b>
 * - Viewer::select_points() with graphics::PointSelectionRequest: the GPU region-selection
 *   query (polygon, point positions, MVP, clipping planes, input/output bitmasks).
 * - Viewer::point_resource(): the immutable point-position snapshot shared by rendering and
 *   selection, so the query sees exactly the points that are drawn.
 * - PointCloud vertex properties ("v:select", "v:color"): store the selection state and the
 *   per-point colors; PointsDrawable::set_coloring() / update_color_buffer() display them.
 * - Viewer::set_overlays() + graphics::screen_space_projection(): draw the selection region
 *   in framebuffer-pixel coordinates on top of the scene; Tessellator triangulates the lasso.
 *
 * <b>Running it</b>
 * Hold Shift and drag with the left button to select points, or with the right button to
 * deselect. Press 'L' for lasso mode (default), 'R' for rectangle mode, and Escape to cancel
 * an ongoing gesture. Selected points turn red.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_403_PointSelection/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_403_PointSelection/viewer.cpp
 *
 * \see Viewer, PointCloud, PointsDrawable, Tessellator, Tutorial_401_ModelPicker
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A point cloud stored in Easy3D's binary format.
    const std::string file = resource::directory() + "/data/polyhedron.bin";

    // Our custom viewer: a Viewer subclass that turns Shift+drag into region selection.
    TutorialPointSelection viewer(EXAMPLE_TITLE);

    // Load the cloud; the second argument (true) centers and scales it to fit the scene.
    auto model = viewer.add_model(file, true);
    if (!model) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // "vertices" is the standard points drawable of a point cloud. Enlarging the points
    // makes the selection feedback (red points) easier to see.
    auto drawable = model->renderer()->get_points_drawable("vertices");
    // drawable->set_impostor_type(PointsDrawable::SPHERE);
    drawable->set_point_size(3.0f);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
