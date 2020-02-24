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

#include "some_test.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/core/random.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/dialogs.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


namespace easy3d {


    SomeTest::SomeTest(const std::string &title)
            : Viewer(title) {
    }


    std::string SomeTest::usage() const {
        return Viewer::usage() +
                (" -------------------- SomeTest Usage --------------------------- \n"
                "\tCtrl+O: Open file\n"
                "\tG: Ground truth\n"
                "\tE: Edges\n"
                "\tC: All candidate faces\n"
                " ---------------------------------------------------------------- \n");
    }


    bool SomeTest::open() {
        const std::string title("Please choose a file");
        const std::string default_path("");
        const std::vector<std::string>& filters = {
                "Mesh Files (*.obj *.ply *.off *.stl *.poly)" , "*.obj *.ply *.off *.stl *.poly" ,
                "Point Cloud Files (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)", "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "All Files (*.*)", "*"
        };

        const std::string& file_name = dialog::open(title, default_path, filters);
        auto mesh = SurfaceMeshIO::load(file_name);
        if (mesh) {
            clear_scene();
            add_model(mesh, true);
            fit_screen();
            return true;
        }
        return false;
    }


    bool SomeTest::key_press_event(int key, int modifiers) {
        if (key == GLFW_KEY_G) {
            std::cout << "Now I am showing ground truth" << std::endl;
            for (auto d : edges_ground_truth_)
                d->set_visible(false);
            for (auto d : candidate_faces_)
                d->set_visible(false);
            for (auto d : faces_ground_truth_)
                d->set_visible(true);
            update();
            return true;
        }
        else if (key == GLFW_KEY_E) {
            std::cout << "Now I am showing edges" << std::endl;
            for (auto d : faces_ground_truth_)
                d->set_visible(false);
            for (auto d : candidate_faces_)
                d->set_visible(false);
            for (auto d : edges_ground_truth_)
                d->set_visible(true);
            update();
            return true;
        }
        else if (key == GLFW_KEY_C) {
            std::cout << "Now I am showing all candidate faces" << std::endl;
            for (auto d : faces_ground_truth_)
                d->set_visible(false);
            for (auto d : edges_ground_truth_)
                d->set_visible(false);
            for (auto d : candidate_faces_)
                d->set_visible(true);
            update();
            return true;
        }
        else
            return Viewer::key_press_event(key, modifiers);
    }


    void SomeTest::create_drawables(Model* model) {
        auto mesh = dynamic_cast<SurfaceMesh*>(model);
        if (!mesh)
            return;

        faces_ground_truth_.clear();
        edges_ground_truth_.clear();
        candidate_faces_.clear();

        {   // candidate faces
            auto faces = mesh->add_triangles_drawable("faces");
            auto colors = mesh->face_property<vec3>("f:color");
            for (auto f : mesh->faces())
                colors[f] = random_color();
            renderer::update_buffer(mesh, faces);
            faces->set_lighting_two_sides(true);
            faces->set_visible(false);
            candidate_faces_.push_back(faces);

            auto* edges = mesh->add_lines_drawable("edges");
            renderer::update_buffer(mesh, edges);
            edges->set_visible(false);
            edges->set_line_width(2.0f);
            candidate_faces_.push_back(edges);

            auto borders = mesh->add_lines_drawable("borders");
            auto prop = mesh->get_vertex_property<vec3>("v:point");
            std::vector<vec3> points;
            for (auto e : mesh->edges()) {
                if (mesh->is_boundary(e)) {
                    points.push_back(prop[mesh->vertex(e, 0)]);
                    points.push_back(prop[mesh->vertex(e, 1)]);
                }
            }
            borders->update_vertex_buffer(points);
            borders->set_default_color(setting::surface_mesh_borders_color);
            borders->set_per_vertex_color(false);
            borders->set_impostor_type(LinesDrawable::CYLINDER);
            borders->set_line_width(setting::surface_mesh_borders_line_width);
            borders->set_visible(false);
            candidate_faces_.push_back(borders);
        }

        {   // ground truth faces
            // I am lazy... Just copy the mesh and create a drawable for all the selected faces
            auto copy = new SurfaceMesh(*mesh);
            copy->set_name("ground_truth");
            add_model(copy, false);
            auto labels = copy->get_face_property<int>("f:label");
            if (labels) {
                for (auto f : copy->faces()) {
                    if (labels[f] == 0)
                        copy->delete_face(f);
                }
                copy->garbage_collection();
            }

            auto faces = copy->add_triangles_drawable("faces");
            renderer::update_buffer(copy, faces);
            faces->set_lighting_two_sides(true);
            faces_ground_truth_.push_back(faces);

            auto* edges = copy->add_lines_drawable("edges");
            renderer::update_buffer(copy, edges);
            faces_ground_truth_.push_back(edges);

            auto borders = copy->add_lines_drawable("borders");
            auto prop = copy->get_vertex_property<vec3>("v:point");
            std::vector<vec3> points;
            for (auto e : copy->edges()) {
                if (copy->is_boundary(e)) {
                    points.push_back(prop[copy->vertex(e, 0)]);
                    points.push_back(prop[copy->vertex(e, 1)]);
                }
            }
            borders->update_vertex_buffer(points);
            borders->set_default_color(setting::surface_mesh_borders_color);
            borders->set_per_vertex_color(false);
            borders->set_impostor_type(LinesDrawable::CYLINDER);
            borders->set_line_width(setting::surface_mesh_borders_line_width);
            faces_ground_truth_.push_back(borders);
        }


        {   // ground truth edges

            auto prop = mesh->get_model_property<io::Element>("element-super_edge");
            if (!prop)
                return;

            const std::vector<io::Element>& elements= prop.vector();
            if (elements.empty())
                return;

            const io::Element& element = elements[0];
            std::vector<int> edge_labels;
            for (const auto& p : element.int_properties) {
                if (p.name == "label") {
                    edge_labels = p;
                }
            }
            if (edge_labels.empty())
                return;;

            const auto& points = mesh->points();
            std::vector<vec3> pts, cls;
            for (auto indices : element.int_list_properties) {
                if (indices.name == "vertex_indices") {
                    for (int i=0; i<indices.size(); ++i) {
                        if (edge_labels[i]) {
                            int s = indices[i][0];
                            int t = indices[i][1];
                            pts.push_back(points[s]);
                            pts.push_back(points[t]);
                            const auto& c = random_color();
                            cls.insert(cls.end(), 2, c);
                        }
                    }
                }
            }

            auto* edges = mesh->add_lines_drawable("ground_truth_edges");
            edges->update_vertex_buffer(pts);
            edges->update_color_buffer(cls);
            edges->set_default_color(setting::surface_mesh_borders_color);
            edges->set_per_vertex_color(true);
            edges->set_impostor_type(LinesDrawable::CYLINDER);
            edges->set_line_width(setting::surface_mesh_borders_line_width);
            edges->set_visible(false);
            edges_ground_truth_.push_back(edges);
        }

    }

}
