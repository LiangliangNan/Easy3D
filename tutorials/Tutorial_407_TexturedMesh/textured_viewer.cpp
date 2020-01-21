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
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/tinyobjloader/tiny_obj_loader.h>


namespace easy3d {


    TexturedViewer::TexturedViewer(const std::string &title)
    : Viewer(title)
    {
        camera()->setUpVector(vec3(0, 1, 0));
    }


    Model* TexturedViewer::open(const std::string &file_name, bool create_default_drawables) {
        if (file_system::extension(file_name, true) == "obj") {
            Model* mesh = load_obj(file_name);
            if (mesh)
                add_model(mesh, false);
            return mesh;
        }
        else
            return Viewer::open(file_name, create_default_drawables);
    }


    Model* TexturedViewer::load_obj(const std::string& file_name) {
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return nullptr;
        }

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

        SurfaceMesh* mesh = new SurfaceMesh;
        mesh->set_name(file_name);

        // add vertices
        for (std::size_t v = 0; v < attrib.vertices.size(); v += 3) {
            // Should I create vertices later, to get rid of isolated vertices?
            mesh->add_vertex(vec3(attrib.vertices.data() + v));
        }
        // for each texcoord
        std::vector<vec2> texcoords;
        for (std::size_t v = 0; v < attrib.texcoords.size(); v += 2) {
            texcoords.push_back(vec2(attrib.texcoords.data() + v));
        }

        // ------------------------ build the mesh ------------------------

        // create texture coordinate property if texture coordinates present
        if (!texcoords.empty())
            mesh->add_halfedge_property<vec2>("h:texcoord");
        auto prop_texcoords = mesh->get_halfedge_property<vec2>("h:texcoord");

        // for each shape
        for (size_t i = 0; i < shapes.size(); i++) {
            std::size_t index_offset = 0;
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.material_ids.size());
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.smoothing_group_ids.size());

            // For each face
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                const std::size_t fnum = shapes[i].mesh.num_face_vertices[f];

                // For each vertex in the face
                std::vector<SurfaceMesh::Vertex> vertices;
                std::unordered_map<int, int> texcoord_ids;  // vertex id -> texcoord id
                for (std::size_t v = 0; v < fnum; v++) {
                    tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                    SurfaceMesh::Vertex vtx(idx.vertex_index);
                    vertices.push_back(vtx);
                    if (prop_texcoords)
                        texcoord_ids[vtx.idx()] = idx.texcoord_index;
                }

                SurfaceMesh::Face face = mesh->add_face(vertices);
                if (prop_texcoords) {
                    for (auto h : mesh->halfedges(face)) {
                        auto v = mesh->to_vertex(h);
                        int id = texcoord_ids[v.idx()];
                        prop_texcoords[h] = texcoords[static_cast<std::size_t>(id)];
                    }
                }

                //int smoothing_group_id = shapes[i].mesh.smoothing_group_ids[f];
                index_offset += fnum;
            }
        }

        if (materials.empty()) {
            Viewer::create_drawables(mesh);
            return mesh;
        }

        // ------------- group the faces according to the material -------------
        // each group is a set of faces sharing the same material

        typedef std::vector<easy3d::SurfaceMesh::Face> Group;
        std::vector<Group> groups(materials.size());

        int face_idx = 0;
        for (std::size_t i = 0; i < shapes.size(); i++) {
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                if (!materials.empty()) {
                    // the material id of this face
                    int material_id = shapes[i].mesh.material_ids[f];
                    groups[material_id].push_back(easy3d::SurfaceMesh::Face(face_idx));
                }
                ++face_idx;
            }
        }

        auto points = mesh->get_vertex_property<vec3>("v:point");
        mesh->update_vertex_normals();
        auto normlals = mesh->get_vertex_property<vec3>("v:normal");
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const Group &group = groups[i];
            if (group.empty())
                continue;
            TrianglesDrawable *drawable = mesh->add_triangles_drawable("faces_" + std::to_string(i));

            std::vector<vec3> vertices, vertex_normals;
            std::vector<vec2> vertex_texcoords;
            for (auto f : group) {
                // we assume convex polygonal faces and we render in triangles
                SurfaceMesh::Halfedge start = mesh->halfedge(f);
                SurfaceMesh::Halfedge cur = mesh->next_halfedge(mesh->next_halfedge(start));
                SurfaceMesh::Vertex va = mesh->to_vertex(start);
                const vec3 &pa = points[va];
                const vec3 &na = normlals[va];
                while (cur != start) {
                    SurfaceMesh::Vertex vb = mesh->from_vertex(cur);
                    SurfaceMesh::Vertex vc = mesh->to_vertex(cur);
                    const vec3 &pb = points[vb];    const vec3 &nb = normlals[vb];
                    const vec3 &pc = points[vc];    const vec3 &nc = normlals[vc];
                    vertices.push_back(pa); vertex_normals.push_back(na);
                    vertices.push_back(pb); vertex_normals.push_back(nb);
                    vertices.push_back(pc); vertex_normals.push_back(nc);

                    if (prop_texcoords) {
                        vertex_texcoords.push_back(prop_texcoords[start]);
                        vertex_texcoords.push_back(prop_texcoords[mesh->prev_halfedge(cur)]);
                        vertex_texcoords.push_back(prop_texcoords[cur]);
                    }

                    cur = mesh->next_halfedge(cur);
                }
            }

            drawable->update_vertex_buffer(vertices);
            drawable->update_normal_buffer(vertex_normals);
            drawable->set_smooth_shading(false);
            if (prop_texcoords)
                drawable->update_texcoord_buffer(vertex_texcoords);
            drawable->release_element_buffer();

            const tinyobj::material_t &mat = materials[i];
            vec3 ambient(mat.ambient);
            vec3 diffuse(mat.diffuse);
            vec3 specular(mat.specular);
            float shininess(mat.shininess);
            drawable->set_material(Material(ambient, specular, shininess));
            drawable->set_default_color(diffuse);

            std::string texname = mat.ambient_texname;
            if (texname.empty())
                texname = mat.diffuse_texname;
            if (texname.empty())
                texname = mat.specular_texname;

            if (!texname.empty()) {
                const std::string texture_file = file_system::parent_directory(file_name) + "/" + texname;
                Texture *tex = Texture::create(texture_file, GL_REPEAT);
                if (tex) {
                    drawable->set_texture(tex);
                    LOG(INFO) << "texture created from " << texname;
                }
            }

        }

        return mesh;
    }
}
