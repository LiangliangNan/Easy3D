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

#include "nonmanifold_viewer.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/tessellator_gen.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/dialogs.h>

#include <3rd_party/tinyobjloader/tiny_obj_loader.h>



// NOTE: The current SurfaceMesh implementation cannot handle non-manifold surfaces.
// So for the moment, I add only drawables to the viewer (without creating a mesh).

namespace easy3d {

    NonmanifoldViewer::NonmanifoldViewer(const std::string &title)
            : Viewer(title)
    {
    }

    bool NonmanifoldViewer::open() {
        const std::string& title = "Please choose a file";
        const std::string& default_path = setting::resource_directory() + "/data/repair/non_manifold";
        const std::vector<std::string>& filters = {
                "Mesh Files (*.obj)" , "*.obj"
        };
        const std::vector<std::string>& file_names = dialog::open(title, default_path, filters, true);

        int count = 0;
        for (const auto& file_name : file_names) {
            if (add_model(file_name, false))
                ++count;
        }

        if (count > 0) {
            fit_screen();
            return true;
        }
        return false;
    }

    namespace details {

        struct Face {
            std::vector<int> vertex_indices;
            std::vector<int> texcoord_indices;
            vec3 normal;
        };

        // each group is a set of faces (denoted by their indices) sharing the same material
        struct Group : public std::vector<int> {
            vec3        ambient;
            vec3        diffuse;
            vec3        specular;
            float       shininess;
            std::string tex_file;
        };
    }

    bool NonmanifoldViewer::add_model(const std::string &file_name, bool create_default_drawables) {
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return false;
        }

        if (file_system::extension(file_name, true) != "obj")
            return Viewer::add_model(file_name, create_default_drawables);

        tinyobj::ObjReaderConfig config;
        config.triangulate = false;
        config.vertex_color = false;
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(file_name, config)) {
            std::string msg = "failed parsing file: " + file_name;
            const std::string &error = reader.Error();
            if (!error.empty())
                msg += error;
            const std::string &warning = reader.Warning();
            if (!warning.empty())
                msg += warning;
            LOG(ERROR) << msg;
            return false;
        }

        // --------------------- collect the data ------------------------

        const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
        if (shapes.empty()) {
            LOG(WARNING) << "file contains no shape";
            return false;
        }
        const tinyobj::attrib_t &attrib = reader.GetAttrib();
        const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();

        std::vector<vec3> points;
        for (std::size_t v = 0; v < attrib.vertices.size(); v += 3) {
            // Should I create vertices later, to get rid of isolated vertices?
            points.emplace_back(vec3(attrib.vertices.data() + v));
        }
        std::vector<vec2> texcoords;
        for (std::size_t v = 0; v < attrib.texcoords.size(); v += 2) {
            texcoords.push_back(vec2(attrib.texcoords.data() + v));
        }

        std::vector<details::Face> faces;
        for (size_t i = 0; i < shapes.size(); i++) {
            std::size_t index_offset = 0;
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.material_ids.size());
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.smoothing_group_ids.size());

            // For each face
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                details::Face face;
                const std::size_t fnum = shapes[i].mesh.num_face_vertices[f];
                for (std::size_t v = 0; v < fnum; v++) {
                    tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                    face.vertex_indices.push_back(idx.vertex_index);
                    if (!texcoords.empty())
                        face.texcoord_indices.push_back(idx.texcoord_index);
                }
                faces.push_back(face);

                //int smoothing_group_id = shapes[i].mesh.smoothing_group_ids[f];
                index_offset += fnum;
            }
        }

        // ------------- group the faces according to the material -------------

        // the extra one is for all faces without material information
        std::vector<details::Group> groups(materials.size() + 1);

        int face_idx = 0;
        int count = 0;
        for (std::size_t i = 0; i < shapes.size(); i++) {
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                if (materials.empty())
                    groups[groups.size() - 1].push_back(face_idx);
                else {
                    // the material id of this face
                    int material_id = shapes[i].mesh.material_ids[f];
                    auto& g = groups[material_id];
                    g.push_back(face_idx);

                    const tinyobj::material_t &mat = materials[material_id];
                    g.ambient = vec3(mat.ambient);
                    g.diffuse = vec3(mat.diffuse);
                    g.specular = vec3(mat.specular);
                    g.shininess = mat.shininess;                   ;
                    g.tex_file = mat.ambient_texname;
                    if (g.tex_file.empty())
                        g.tex_file = mat.diffuse_texname;
                    if (g.tex_file.empty())
                        g.tex_file = mat.specular_texname;
                }
                ++face_idx;
            }
        }


        auto compute_face_normal = [](const details::Face& f, const std::vector<vec3>& points) ->vec3 {
            int a = f.vertex_indices[0];
            int b = f.vertex_indices[1];
            int c = f.vertex_indices[2];
            return geom::triangle_normal(points[a], points[b], points[c]);
        };

        SurfaceMesh* mesh = new SurfaceMesh;
        mesh->set_name(file_name);

        TessellatorGen tessellator;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto &group = groups[i];
            if (group.empty())
                continue;

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;

             for (auto id : group) {
                 tessellator.reset();
                 details::Face& face = faces[id];
                 face.normal = compute_face_normal(face, points);
                 tessellator.begin_polygon(face.normal);
                 tessellator.set_winding_rule(TessellatorGen::NONZERO);  // or POSITIVE
                 tessellator.begin_contour();
                 const auto& vertex_indices = face.vertex_indices;
                 const auto& texcoord_indices = face.texcoord_indices;
                 if (texcoords.empty()) {
                     for (int j = 0; j < vertex_indices.size(); ++j)
                         tessellator.add_vertex(points[vertex_indices[j]]);
                 } else {
                     for (int j = 0; j < vertex_indices.size(); ++j)
                         tessellator.add_vertex(points[vertex_indices[j]], texcoords[texcoord_indices[j]]);
                 }
                 tessellator.end_contour();
                 tessellator.end_polygon();

                 std::size_t num = tessellator.num_triangles();
                 const std::vector<const double *> &vts = tessellator.get_vertices();
                 for (std::size_t j = 0; j < num; ++j) {
                     std::size_t a, b, c;
                     tessellator.get_triangle(j, a, b, c);
                     const vec3 va(vts[a]); d_points.emplace_back(va);
                     const vec3 vb(vts[b]); d_points.emplace_back(vb);
                     const vec3 vc(vts[c]); d_points.emplace_back(vc);
                     d_normals.insert(d_normals.end(), 3, geom::triangle_normal(va, vb, vc));
                     if (!texcoords.empty()) {
                         d_texcoords.emplace_back(vec2(vts[a] + 3));
                         d_texcoords.emplace_back(vec2(vts[b] + 3));
                         d_texcoords.emplace_back(vec2(vts[c] + 3));
                     }
                 }
            }

            auto faces = mesh->add_triangles_drawable("faces-" + std::to_string(i));
            faces->update_vertex_buffer(d_points);
            faces->update_normal_buffer(d_normals);
            faces->set_smooth_shading(false);
            if (!texcoords.empty())
                faces->update_texcoord_buffer(d_texcoords);

            if (i == groups.size() - 1)
                break;  // the last group doesn't have material information

            faces->set_material(Material(group.ambient, group.specular, group.shininess));
            faces->set_default_color(group.diffuse);

            std::string texname = group.tex_file;
            if (!texname.empty()) {
                const std::string texture_file = file_system::parent_directory(file_name) + "/" + texname;
                Texture *tex = Texture::create(texture_file, GL_REPEAT);
                if (tex) {
                    faces->set_texture(tex);
                    LOG(INFO) << "texture created from " << texname;
                }
            }
        }

        //------ also edges -----
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto &group = groups[i];
            if (group.empty())
                continue;

            std::vector<vec3> d_points;
            for (auto id : group) {
                details::Face& face = faces[id];
                const auto& vertex_indices = face.vertex_indices;
                for (int j = 0; j < vertex_indices.size(); ++j) {
                    d_points.push_back(points[vertex_indices[j]]);
                    d_points.push_back(points[vertex_indices[(j + 1)%vertex_indices.size()]]);
                }
            }
            auto edges = mesh->add_lines_drawable("edges-" + std::to_string(i));
            edges->update_vertex_buffer(d_points);
        }
        //------

        fit_screen();
        return Viewer::add_model(mesh, false);
    }
}
