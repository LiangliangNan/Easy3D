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

#include "textured_viewer.h"

#include <unordered_map>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/tessellator.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/tinyobjloader/tiny_obj_loader.h>


namespace easy3d {


    TexturedViewer::TexturedViewer(const std::string &title)
            : Viewer(title) {
        camera()->setUpVector(vec3(0, 1, 0));
    }

    namespace details {

        // each group is a set of faces (denoted by their indices) sharing the same material
        struct Group : public std::vector<int> {
            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
            float shininess;
            std::string tex_file;
        };
    }


    Model *TexturedViewer::add_model(const std::string &file_name, bool create_default_drawables) {
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return nullptr;
        }

        if (file_system::extension(file_name, true) != "obj")
            return Viewer::add_model(file_name, create_default_drawables);

        tinyobj::ObjReaderConfig config;
        config.triangulate = false;
        config.vertex_color = false;
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(file_name, config)) {
            LOG(ERROR) << "failed parsing file: " + file_name + ". " << reader.Error() << reader.Warning();
            return nullptr;
        }

        // --------------------- collect the data ------------------------

        const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
        if (shapes.empty()) {
            LOG(WARNING) << "file contains no shape";
            return nullptr;
        }
        const tinyobj::attrib_t &attrib = reader.GetAttrib();
        const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();

        // for each texcoord
        std::vector<vec2> texcoords;
        for (std::size_t v = 0; v < attrib.texcoords.size(); v += 2) {
            texcoords.push_back(vec2(attrib.texcoords.data() + v));
        }

        // ------------------------ build the mesh ------------------------

        // clear the mesh in case of existing data
        SurfaceMesh *model = new SurfaceMesh;
        model->set_name(file_name);

        ManifoldBuilder builder(model);
        builder.begin_surface();

        // add vertices
        for (std::size_t v = 0; v < attrib.vertices.size(); v += 3) {
            // Should I create vertices later, to get rid of isolated vertices?
            builder.add_vertex(vec3(attrib.vertices.data() + v));
        }

        // create texture coordinate property if texture coordinates present
        if (!texcoords.empty())
            model->add_halfedge_property<vec2>("h:texcoord");
        auto prop_texcoords = model->get_halfedge_property<vec2>("h:texcoord");

        auto find_face_halfedge = [](SurfaceMesh *mesh, SurfaceMesh::Face face,
                                     SurfaceMesh::Vertex v) -> SurfaceMesh::Halfedge {
            for (auto h : mesh->halfedges(face)) {
                if (mesh->to_vertex(h) == v)
                    return h;
            }
            LOG_FIRST_N(ERROR, 1) << "could not find a halfedge pointing to " << v << " in face " << face
                                  << " (this is the first record)";
            return SurfaceMesh::Halfedge();
        };

        // invalid face will also be added, to ensure correct face indices
        std::vector<SurfaceMesh::Face> faces;
        // for each shape
        for (size_t i = 0; i < shapes.size(); i++) {
            std::size_t index_offset = 0;
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.material_ids.size());
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.smoothing_group_ids.size());

            // For each face
            for (std::size_t face_idx = 0; face_idx < shapes[i].mesh.num_face_vertices.size(); ++face_idx) {
                std::size_t face_size = shapes[i].mesh.num_face_vertices[face_idx];

                // For each vertex in the face
                std::vector<SurfaceMesh::Vertex> vertices;
                std::vector<int> texcoord_ids;
                for (std::size_t v = 0; v < face_size; v++) {
                    const tinyobj::index_t &face = shapes[i].mesh.indices[index_offset + v];
                    vertices.emplace_back(face.vertex_index);
                    if (prop_texcoords)
                        texcoord_ids.emplace_back(face.texcoord_index);
                }

                SurfaceMesh::Face face = builder.add_face(vertices);
                // invalid face will also be added, to ensure correct face indices
                faces.push_back(face);
                if (prop_texcoords && face.is_valid()) {
                    auto begin = find_face_halfedge(model, face, builder.face_vertices()[0]);
                    auto cur = begin;
                    int idx = 0;
                    do {
                        prop_texcoords[cur] = texcoords[texcoord_ids[idx++]];
                        cur = model->next_halfedge(cur);
                    } while (cur != begin);
                }

                //int smoothing_group_id = shapes[i].mesh.smoothing_group_ids[f];
                index_offset += face_size;
            }
        }
        builder.end_surface();

        // since the mesh has been built, skip texture if material and texcoord information don't exist
        if (materials.empty())
            return Viewer::add_model(model, true);

        // ------------- group the faces according to the material -------------
        // each group is a set of faces sharing the same material
        std::vector<details::Group> groups(materials.size());

        int face_idx = 0;
        for (std::size_t i = 0; i < shapes.size(); i++) {
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                auto face = faces[face_idx];
                if (face.is_valid()) {
                    // the material id of this face
                    int material_id = shapes[i].mesh.material_ids[f];
                    auto &g = groups[material_id];
                    g.push_back(face_idx);
                    const tinyobj::material_t &mat = materials[material_id];
                    g.ambient = vec3(mat.ambient);
                    g.diffuse = vec3(mat.diffuse);
                    g.specular = vec3(mat.specular);
                    g.shininess = mat.shininess;;
                    g.tex_file = mat.ambient_texname;
                    if (g.tex_file.empty())
                        g.tex_file = mat.diffuse_texname;
                    if (g.tex_file.empty())
                        g.tex_file = mat.specular_texname;
                }
                ++face_idx;
            }
        }

        auto points = model->get_vertex_property<vec3>("v:point");
        model->update_vertex_normals();
        auto normals = model->get_vertex_property<vec3>("v:normal");

        Tessellator tessellator;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto &group = groups[i];
            if (group.empty())
                continue;

            tessellator.reset();

            for (auto f : group) {
                auto face = faces[f];
                if (!face.is_valid())
                    continue;

                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    const vec3 &v = points[model->to_vertex(h)];
                    const vec3 &n = normals[model->to_vertex(h)];
                    if (prop_texcoords) {
                        const vec2 &t = prop_texcoords[h];
                        const float data[8] = {v.x, v.y, v.z, n.x, n.y, n.z, t.x, t.y};
                        tessellator.add_vertex(data, 8);
                    } else {
                        const float data[6] = {v.x, v.y, v.z, n.x, n.y, n.z};
                        tessellator.add_vertex(data, 6);
                    }
                }
                tessellator.end_contour();
                tessellator.end_polygon();
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex*>& vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                if (prop_texcoords)
                    d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &indices = tessellator.indices();

            TrianglesDrawable *drawable = model->add_triangles_drawable("faces_" + std::to_string(i));

            drawable->update_index_buffer(indices);
            drawable->update_vertex_buffer(d_points);
            drawable->update_normal_buffer(d_normals);
            if (prop_texcoords)
                drawable->update_texcoord_buffer(d_texcoords);
            drawable->set_smooth_shading(false);

            drawable->set_material(Material(group.ambient, group.specular, group.shininess));
            drawable->set_default_color(group.diffuse);

            std::string texname = group.tex_file;
            if (!texname.empty()) {
                const std::string texture_file = file_system::parent_directory(file_name) + "/" + texname;
                Texture *tex = Texture::create(texture_file, GL_REPEAT);
                if (tex) {
                    drawable->set_texture(tex);
                    drawable->set_use_texture(true);
                    LOG(INFO) << "texture created from " << texname;
                }
            }
        }

        // the drawables have already been created
        return Viewer::add_model(model, false);
    }

}
