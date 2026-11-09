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
 * \example{lineno} Tutorial_402_FacePicker/main.cpp
 *
 * \brief Selecting an individual face of a surface mesh by clicking on it with the mouse.
 *
 * <b>Overview</b>
 * This tutorial refines the model-level picking of Tutorial_401_ModelPicker down to a single
 * face: clicking a face highlights it and prints its index. The loaded model (P.off) is a
 * quad mesh, which makes the tutorial more interesting — the GPU can only rasterize
 * triangles, so picking must map the hit triangle back to the original polygonal face.
 *
 * <b>The technique</b>
 * Easy3D picks with a GPU primitive-ID pass (see Tutorial_401_ModelPicker): the scene is
 * re-rendered off screen with each triangle writing an encoded index instead of a color,
 * and the pixel under the cursor is read back. Viewer::pick() returns that triangle index
 * and, when provenance is available, the original face index. The subtlety here is that a
 * SurfaceMesh may contain non-triangular faces: for unified rendering, every polygon is
 * internally triangulated, and the mesh stores a "f:triangle_range" face property mapping
 * each original face to the [first, last] range of triangles it produced. Highlighting must
 * therefore be expressed in triangle indices: we look up the picked face's triangle range
 * and pass it to the drawable's highlight state, so the whole polygon lights up, not just
 * the one triangle that happened to be under the cursor.
 *
 * <b>Easy3D API</b>
 * - Viewer::pick() / Viewer::PickResult: the GPU picking query; face_index identifies the
 *   original SurfaceMesh face under the cursor (-1 when nothing was hit).
 * - SurfaceMesh: the polygonal mesh data structure; get_face_property() retrieves the
 *   "f:triangle_range" mapping used to convert faces to triangle ranges.
 * - TrianglesDrawable (State): set_highlight() / set_highlight_range() mark a range of
 *   triangles to be drawn in the highlight color.
 * - LinesDrawable: the "edges" drawable is made visible so face boundaries can be seen.
 *
 * <b>Running it</b>
 * Left-click a face to highlight it (the console prints "picked face f<index>"). Clicking
 * another face moves the highlight; clicking the background clears it. Dragging still
 * rotates the camera because unhandled events are forwarded to the base Viewer.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_402_FacePicker/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_402_FacePicker/viewer.cpp
 *
 * \see Viewer, SurfaceMesh, TrianglesDrawable, Tutorial_401_ModelPicker
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A quad mesh: its faces are polygons, not triangles, so picking must map the hit
    // triangle back to the original face (see the "f:triangle_range" discussion above).
    const std::string file_name = resource::directory() + "/data/quad_mesh/P.off";

    // Our custom viewer: a Viewer subclass that highlights the face under a left click.
    TutorialFacePicker viewer(EXAMPLE_TITLE);

    // This calls the overridden add_model(), which loads the mesh and makes its edges
    // visible so the individual faces are easy to see and aim at.
    if (!viewer.add_model(file_name)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
