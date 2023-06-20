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

#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/algo/text_mesher.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


// This example shows how to
//		- generate a surface mesh models from texts

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
    SurfaceMesh* mesh = mesher.generate("Easy3D", 0, 0, 48, 15, true);
    if (mesh)
        viewer.add_model(mesh); // Add the mesh to the viewer.

    // Generate surface for "Makes 3D Easy!".
    mesher.set_font(resource::directory() + "/fonts/en_Roboto-Regular.ttf");
    mesher.generate(mesh,"Makes 3D Easy!", 350, 0, 25, 15, true);

#else // extract and visualize the 2D contours of the text
    std::vector< std::vector<Polygon2> > contours;
    mesher.set_font(font_file);
    mesher.generate("Easy3D", 0, -60, 48, contours, true);
    mesher.set_font(resource::directory() + "/fonts/en_Roboto-Regular.ttf");
    mesher.generate("Makes 3D Easy!", 350, -60, 25, contours, true);
    std::vector<vec3> points, colors;
    std::vector<unsigned int> indices;
    int offset = 0;
    for (auto &cha : contours) {
        for (auto &con : cha) {
            vec3 c = con.is_clockwise() ? vec3(1, 0, 0) : vec3(0, 1, 0);
            for (int i = 0; i < con.size(); ++i) {
                points.push_back(vec3(con[i], 0.0f));
                colors.push_back(c);
                indices.push_back(offset + i);
                indices.push_back(offset + (i + 1) % con.size());
            }
            offset += con.size();
        }
    }
    LinesDrawable *d = new LinesDrawable;
    d->update_vertex_buffer(points);
    d->update_color_buffer(colors);
    d->update_element_buffer(indices);
    d->set_impostor_type(LinesDrawable::CONE);
    d->set_line_width(2);
    d->set_property_coloring(easy3d::State::VERTEX);
    viewer.add_drawable(d);
#endif

    // We always want to look at the front of the meshed text.
    viewer.camera()->setViewDirection(vec3(0, 0, -1));
    viewer.camera()->setUpVector(vec3(0, 1, 0));

    // Go...
    return viewer.run();
}
