/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>

#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/drawable_lines.h>

#include "text_mesh.h"


using namespace easy3d;

int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    const std::string font_file = resource::directory() + "/fonts/Earth-Normal.ttf";
    TextMesh mesher(font_file);
    if (!mesher.is_ready())
        return EXIT_FAILURE;

    Viewer viewer("Tutorial_TextMesh3D");

#if 0
    SurfaceMesh* mesh = texter.generate_mesh("Easy3D");
    if (!mesh)
        return EXIT_FAILURE;

    viewer.add_model(mesh);
    mesh->get_triangles_drawable("faces")->set_lighting_two_sides(true);
#else
    std::vector<TextMesh::CharContour> characters;
    mesher.generate_contours("Easy3D", characters);
    if (!characters.empty()) {
        LinesDrawable* d = new LinesDrawable("contours");

        std::vector<vec3> points;
        std::vector<unsigned int> indices;

        int offset = 0;
        for (const auto& ch :characters) {
            for (int c=0; c < ch.size(); ++c) {
                const TextMesh::Contour& contour = ch[c];
                for (int p=0; p<contour.size(); ++p) {
                    points.push_back(vec3(contour[p], 0.0f));
                    indices.push_back(offset + p);
                    indices.push_back(offset + (p + 1) % contour.size());
                }
                offset += contour.size();
            }
        }
        d->update_vertex_buffer(points);
        d->update_index_buffer(indices);

        viewer.add_drawable(d);
    }
#endif

    return viewer.run();
}
