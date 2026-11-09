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

#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/algo/text_mesher.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_310_TextMesher/main.cpp
 *
 * \brief Turning text strings into 3D surface meshes (or 2D outline contours) with TextMesher.
 *
 * <b>Overview</b>
 * This tutorial converts the strings "Easy3D" and "Makes 3D Easy!" — in two different fonts —
 * into real geometry that can be rendered, processed, or exported like any other mesh. Unlike
 * Tutorial_309_TextRendering, which draws text as 2D image overlays, here the letters become
 * actual triangles in a SurfaceMesh. A compile-time switch (\#if 1 / \#else) selects between the
 * extruded 3D meshes and the flat 2D glyph contours drawn as colored lines.
 *
 * <b>The technique</b>
 * TrueType fonts describe each glyph as closed 2D contours made of line segments and quadratic
 * Bezier curves. To mesh a string, the mesher (1) scales and places each glyph's contours at the
 * requested position and font size, (2) flattens the curves into polylines (the "quality"
 * constructor parameter controls how finely), (3) triangulates the outlines — letters like "O"
 * or "a" have holes, so this is polygon-with-holes triangulation, typically done via constrained
 * Delaunay methods — and (4) extrudes the filled 2D region along the Z axis by the given height
 * to obtain a solid 3D letter with front, back, and side walls. With collision_free = true, overlapping
 * characters are united so the result has no self-intersections. Contour orientation matters:
 * outer boundaries and hole boundaries wind in opposite directions, which is exactly what the
 * alternative branch below visualizes (clockwise vs counter-clockwise contours in different
 * colors).
 *
 * <b>Easy3D API</b>
 * - TextMesher: constructed from a .ttf font file; set_font() switches fonts; generate() has
 *   three overloads — new SurfaceMesh, append to an existing mesh, or append 2D contours
 *   (std::vector<std::vector<Polygon2>>, one contour list per character).
 * - SurfaceMesh, Viewer, Renderer: the generated mesh is added as a model; Renderer::update()
 *   re-syncs the GPU buffers after geometry was appended post-load.
 * - LinesDrawable, Polygon2: used in the contour branch to draw the outlines as thick colored
 *   lines (CONE impostors) with per-vertex colors.
 * - Camera::setViewDirection()/setUpVector(): face the text straight-on (it lies in the XY
 *   plane and is extruded along the Z axis).
 *
 * <b>Running it</b>
 * A window shows the two extruded text lines viewed from the front; navigate with the usual
 * viewer controls to see the thickness of the letters. Flip the \#if to \#else (0) and rebuild to
 * inspect the raw 2D contours instead: red = clockwise, green = counter-clockwise.
 *
 * \see TextMesher, SurfaceMesh, LinesDrawable, Polygon2, Tutorial_309_TextRendering
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create an Easy3D viewer.
    Viewer viewer(EXAMPLE_TITLE);

    // Create an instance of the mesher by specifying a font file.
    const std::string font_file = resource::directory() + "/fonts/en_Earth-Normal.ttf";
    TextMesher mesher(font_file);

#if 1 // extract and visualize the mesh model of the text
    // Generate a surface mesh for "Easy3D".
    // Arguments: text, start position (x, y) in the text plane, font size, extrusion height
    // along Z, and collision_free (unite overlapping characters into one clean solid).
    SurfaceMesh *mesh = mesher.generate("Easy3D", 0, 0, 48, 15, true);
    if (mesh)
        viewer.add_model(std::shared_ptr<SurfaceMesh>(mesh)); // Add the mesh to the viewer.
    // Generate a surface mesh for "Makes 3D Easy!".
    // Switch to a second font, then APPEND the second string to the same mesh (note the
    // mesh-first overload of generate), offset in x so the two lines sit side by side.
    mesher.set_font(resource::directory() + "/fonts/en_Roboto-Regular.ttf");
    mesher.generate(mesh, "Makes 3D Easy!", 350, 0, 25, 15, true);

#else // extract and visualize the 2D contours of the text
    // Alternative view: no triangulation/extrusion — just the raw glyph outlines.
    // One entry per character; each character may consist of several closed contours
    // (e.g. the letter "B" has an outer boundary plus two holes).
    std::vector<std::vector<Polygon2>> contours;
    mesher.set_font(font_file);
    // Generate the contours for "Easy3D".
    mesher.generate("Easy3D", 0, -60, 48, contours, true);
    mesher.set_font(resource::directory() + "/fonts/en_Roboto-Regular.ttf");
    // Generate the contours for "Makes 3D Easy!".
    mesher.generate("Makes 3D Easy!", 350, -60, 25, contours, true);

    // Prepare data for the rendering drawable.
    // Flatten all contours into one point list plus line-segment indices. The winding
    // direction distinguishes outer boundaries from holes: color them differently.
    std::vector<vec3> points, colors;
    std::vector<unsigned int> indices;
    int offset = 0;
    for (auto &cha : contours) {
        for (auto &con : cha) {
            const vec3 c = con.is_clockwise() ? vec3(1, 0, 0) : vec3(0, 1, 0);
            for (int i = 0; i < con.size(); ++i) {
                points.push_back(vec3(con[i], 0.0f)); // 2D contour points live in the z = 0 plane
                colors.push_back(c);
                // Each segment connects point i to point i+1, wrapping around to close the loop.
                indices.push_back(offset + i);
                indices.push_back(offset + (i + 1) % con.size());
            }
            offset += con.size();
        }
    }
    // Create the rendering drawable and transfer data into the buffers.
    LinesDrawable *d = new LinesDrawable;
    d->update_vertex_buffer(points);
    d->update_color_buffer(colors);
    d->update_element_buffer(indices);
    // CONE impostors render thick, smoothly shaded lines instead of 1-pixel GL lines.
    d->set_impostor_type(LinesDrawable::CONE);
    d->set_line_width(3);
    // Use the per-vertex colors uploaded above (winding-direction coloring).
    d->set_property_coloring(easy3d::State::VERTEX);
    // Add the drawable to the viewer.
    viewer.add_drawable(std::shared_ptr<LinesDrawable>(d));
#endif

    // We always want to look at the front of the meshed text.
    viewer.camera()->setViewDirection(vec3(0, 0, -1));
    viewer.camera()->setUpVector(vec3(0, 1, 0));

    // The mesher appended geometry after attaching the model; refresh its buffers and fit both strings.
    if (auto *model = viewer.current_model())
        model->renderer()->update();

    return viewer.run();
}
