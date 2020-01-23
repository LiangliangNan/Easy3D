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
#include <easy3d/core/manifold_guard.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/tessellator_gen.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/tinyobjloader/tiny_obj_loader.h>


// In case the ManifoldGuard failed to handle the mesh
#define DO_NOT_CREATE_SURFACE_MESH

namespace easy3d {


    TexturedViewer::TexturedViewer(const std::string &title)
            : Viewer(title) {
        camera()->setUpVector(vec3(0, 1, 0));
    }

    namespace details {

        // each group is a set of faces (denoted by their indices) sharing the same material
        struct Group : public std::vector<int> {
            vec3        ambient;
            vec3        diffuse;
            vec3        specular;
            float       shininess;
            std::string tex_file;
        };
    }

#ifdef DO_NOT_CREATE_SURFACE_MESH

    Model* TexturedViewer::add_model(const std::string &file_name, bool create_default_drawables) {
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

        std::vector<vec3> points;
        for (std::size_t v = 0; v < attrib.vertices.size(); v += 3) {
            // Should I create vertices later, to get rid of isolated vertices?
            points.emplace_back(vec3(attrib.vertices.data() + v));
        }
        std::vector<vec2> texcoords;
        for (std::size_t v = 0; v < attrib.texcoords.size(); v += 2) {
            texcoords.push_back(vec2(attrib.texcoords.data() + v));
        }

        struct Face {
            std::vector<int> vertex_indices;
            std::vector<int> texcoord_indices;
            vec3 normal;
        };
        std::vector<Face> faces;

        for (size_t i = 0; i < shapes.size(); i++) {
            std::size_t index_offset = 0;
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.material_ids.size());
            LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.smoothing_group_ids.size());

            // For each face
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                Face face;
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


        auto compute_face_normal = [](const Face& f, const std::vector<vec3>& points) ->vec3 {
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
                 Face& face = faces[id];
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

            auto drawable = mesh->add_triangles_drawable("faces-" + std::to_string(i));
            drawable->update_vertex_buffer(d_points);
            drawable->update_normal_buffer(d_normals);
            drawable->set_smooth_shading(false);
            if (!texcoords.empty())
                drawable->update_texcoord_buffer(d_texcoords);

            if (i == groups.size() - 1)
                break;  // the last group doesn't have material information

            drawable->set_material(Material(group.ambient, group.specular, group.shininess));
            drawable->set_default_color(group.diffuse);

            std::string texname = group.tex_file;
            if (!texname.empty()) {
                const std::string texture_file = file_system::parent_directory(file_name) + "/" + texname;
                Texture *tex = Texture::create(texture_file, GL_REPEAT);
                if (tex) {
                    drawable->set_texture(tex);
                    LOG(INFO) << "texture created from " << texname;
                }
            }
        }

        if (mesh->triangles_drawables().size() > 0) {
            Viewer::add_model(mesh, false);
            fit_screen();
        }
        return mesh;
    }

#else

    Model* TexturedViewer::add_model(const std::string &file_name, bool create_default_drawables) {
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

        ManifoldGuard guard(mesh);
        guard.begin();

        // there might be invalid faces
        std::vector<SurfaceMesh::Face> faces;

        // add vertices
        for (std::size_t v = 0; v < attrib.vertices.size(); v += 3) {
            // Should I create vertices later, to get rid of isolated vertices?
            guard.add_vertex(vec3(attrib.vertices.data() + v));
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
                std::vector<unsigned int> vertices;
                std::unordered_map<int, int> texcoord_ids;  // vertex id -> texcoord id
                for (std::size_t v = 0; v < fnum; v++) {
                    tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                    int vtx = idx.vertex_index;
                    vertices.push_back(vtx);
                    if (prop_texcoords)
                        texcoord_ids[vtx] = idx.texcoord_index;
                }

                SurfaceMesh::Face face = guard.add_face(vertices);
                faces.push_back(face);
                if (face.is_valid() && prop_texcoords) {
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

        guard.finish();

        // since the mesh has been built, skip texture if material and texcoord information don't exist
        if (materials.empty())
            return Viewer::add_model(mesh, true);

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

        auto points = mesh->get_vertex_property<vec3>("v:point");
        mesh->update_vertex_normals();
        auto normals = mesh->get_vertex_property<vec3>("v:normal");

        TessellatorGen tessellator;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto &group = groups[i];
            if (group.empty())
                continue;

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            for (auto f : group) {
                auto face = faces[f];
                if (!face.is_valid())
                    continue;
                tessellator.reset();
                tessellator.begin_polygon(mesh->compute_face_normal(face));
                tessellator.set_winding_rule(TessellatorGen::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : mesh->halfedges(face)) {
                    const vec3& v = points[mesh->to_vertex(h)];
                    const vec3& n = normals[mesh->to_vertex(h)];
                    if (prop_texcoords) {
                        const vec2 &t = prop_texcoords[h];
                        const float data[8] = {v.x, v.y, v.z, n.x, n.y, n.z, t.x, t.y};
                        tessellator.add_vertex(data, 8);
                    }
                    else {
                        const float data[6] = {v.x, v.y, v.z, n.x, n.y, n.z};
                        tessellator.add_vertex(data, 6);
                    }
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles();
                const std::vector<const double *> &vts = tessellator.get_vertices();
                for (std::size_t j = 0; j < num; ++j) {
                    std::size_t a, b, c;
                    tessellator.get_triangle(j, a, b, c);
                    d_points.emplace_back(vts[a]);   d_normals.emplace_back(vts[a] + 3);
                    d_points.emplace_back(vts[b]);   d_normals.emplace_back(vts[b] + 3);
                    d_points.emplace_back(vts[c]);   d_normals.emplace_back(vts[c] + 3);
                    if (prop_texcoords) {
                        d_texcoords.emplace_back(vec2(vts[a] + 6));
                        d_texcoords.emplace_back(vec2(vts[b] + 6));
                        d_texcoords.emplace_back(vec2(vts[c] + 6));
                    }
                }
            }

            TrianglesDrawable *drawable = mesh->add_triangles_drawable("faces_" + std::to_string(i));

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
                    LOG(INFO) << "texture created from " << texname;
                }
            }

        }

        // the drawables have already been created
        return Viewer::add_model(mesh, false);
    }

#endif

}
