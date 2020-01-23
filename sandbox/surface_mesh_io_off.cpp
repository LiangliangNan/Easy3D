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

#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/core/types.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/core/manifold_guard.h>
#include <easy3d/viewer/drawable_lines.h>

#include <fstream>
#include <cstring> // for strlen()

using namespace easy3d;

namespace details {

    // Some OFF files may skip lines
    static void get_line(io::LineInputStream &in) {
        in.get_line();
        const char *p = in.current_line().c_str();
        while (!in.eof() && (strlen(p) == 0 || !isprint(*p))) {
            in.get_line();
            p = in.current_line().c_str();
        }
    }
}


struct Face : std::vector<int> {};

struct Mesh {
    std::vector<vec3> points;
    std::vector<Face> faces;
};

Mesh load_file(const std::string &file_name) {
    Mesh mesh;

    std::ifstream in(file_name.c_str());
    if (in.fail()) {
        std::cerr << "Could not open file \'" << file_name << "\'" << std::endl;
        return mesh;
    }

    // Vertex index starts by 0 in off format.

    io::LineInputStream input(in);
    details::get_line(input);

    std::string magic;
    input >> magic;

    // NOFF is for Grimage "visual shapes".
    if (magic != "OFF" && magic != "NOFF") {
        std::cerr << "Not an OFF file. Key word is: " << magic << std::endl;
        return mesh;
    }

    if (magic != "NOFF") {
        details::get_line(input);
    }

    int nb_vertices, nb_facets, nb_edges;
    input >> nb_vertices >> nb_facets >> nb_edges;
    if (!input.ok()) {
        std::cerr << "An error in the file header: " << input.current_line() << std::endl;
        return mesh;
    }

    for (int i = 0; i < nb_vertices; i++) {
        vec3 p;
        details::get_line(input);
        input >> p;
        mesh.points.push_back(p);
    }

    for (int i = 0; i < nb_facets; i++) {
        int nb_vertices;
        details::get_line(input);
        input >> nb_vertices;

        Face face;
        for (int j = 0; j < nb_vertices; j++) {
            int index;
            input >> index;
            face.push_back(index);
        }
        mesh.faces.push_back(face);
    }

    return mesh;
}


bool load_off(const std::string &file_name, SurfaceMesh *mesh) {
    const Mesh& tmp = load_file(file_name);
    if (tmp.faces.empty())
        return false;

    ManifoldGuard guard(mesh);

    guard.begin();

    const auto& points = tmp.points;
    for (auto p : points)
        guard.add_vertex(p);

    const auto& faces = tmp.faces;
    for (auto ids : faces)
        guard.add_face(ids);

    guard.finish();




    // visualize the borders
    auto prop_points = mesh->vertex_property<vec3>("v:point");
    std::vector<vec3> d_points;
    for (auto e : mesh->edges()) {
        if (mesh->is_boundary(e)) {
            auto s = mesh->vertex(e, 0);
            auto t = mesh->vertex(e, 1);
            d_points.push_back(prop_points[s]);
            d_points.push_back(prop_points[t]);
        }
    }
    if (!d_points.empty()) {
        auto d = mesh->add_lines_drawable("borders");
        d->update_vertex_buffer(d_points);
        d->set_impostor_type(LinesDrawable::CYLINDER);
        d->set_line_width(3.0f);
        d->set_default_color(vec3(1, 0, 0));
    }







    return mesh->vertices_size() > 0;
}
