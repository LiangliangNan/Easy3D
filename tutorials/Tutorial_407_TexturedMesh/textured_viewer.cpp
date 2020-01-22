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
#include <easy3d/viewer/tessellator_gen.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/tinyobjloader/tiny_obj_loader.h>



// NOTE: The current SurfaceMesh implementation cannot handle non-manifold surfaces.
// So for the moment, I add only drawables to the viewer (without creating a mesh).
// TODO: modify the SurfaceMesh data structure to allow non-manifold surfaces.
#define DO_NOT_CREATE_SURFACE_MESH

namespace easy3d {


    TexturedViewer::TexturedViewer(const std::string &title)
            : Viewer(title) {
        camera()->setUpVector(vec3(0, 1, 0));
    }

#ifdef DO_NOT_CREATE_SURFACE_MESH

    namespace details {

        struct Face {
            std::vector<int> vertex_indices;
            std::vector<int> texcoord_indices;
            vec3 normal;
        };

        static inline vec3 compute_face_normal(const Face& f, const std::vector<vec3>& points) {
            int a = f.vertex_indices[0];
            int b = f.vertex_indices[1];
            int c = f.vertex_indices[2];
            return geom::triangle_normal(points[a], points[b], points[c]);
        }

    }


    bool TexturedViewer::add_model(const std::string &file_name, bool create_default_drawables) {
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
        // each group is a set of faces sharing the same material

        typedef std::vector<int> Group; // each element is a face index

        // the extra one is for all faces without material information
        std::vector<Group> groups(materials.size() + 1);

        int face_idx = 0;
        int count = 0;
        for (std::size_t i = 0; i < shapes.size(); i++) {
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                if (materials.empty()) {
                    groups[groups.size() - 1].push_back(face_idx);
                }
                else {
                    // the material id of this face
                    int material_id = shapes[i].mesh.material_ids[f];
                    groups[material_id].push_back(face_idx);
                }
                ++face_idx;
            }
        }

        TessellatorGen tessellator;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const Group &group = groups[i];
            if (group.empty())
                continue;

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;

             for (auto id : group) {
                 tessellator.reset();
                 details::Face& face = faces[id];
                 face.normal = details::compute_face_normal(face, points);
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

            auto drawable = new TrianglesDrawable(file_name + "-faces-" + std::to_string(i));
            drawable->update_vertex_buffer(d_points);
            drawable->update_normal_buffer(d_normals);
            drawable->set_smooth_shading(false);
            if (!texcoords.empty())
                drawable->update_texcoord_buffer(d_texcoords);

            add_drawable(drawable);
            ++count;

            if (i == groups.size() - 1)
                break;  // the last group doesn't have material information

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

        if (count > 0) {
            fit_screen();
            return true;
        }
        else
            return false;
    }

#else

    bool TexturedViewer::add_model(const std::string &file_name, bool create_default_drawables) {
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

        // since the mesh has been built, skip texturing if material and texcoord information don't exist
        if (materials.empty() || texcoords.empty())
            return Viewer::add_model(mesh, true);

        // ------------- group the faces according to the material -------------
        // each group is a set of faces sharing the same material

        typedef std::vector<easy3d::SurfaceMesh::Face> Group;
        std::vector<Group> groups(materials.size());

        int face_idx = 0;
        for (std::size_t i = 0; i < shapes.size(); i++) {
            for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                // the material id of this face
                int material_id = shapes[i].mesh.material_ids[f];
                groups[material_id].push_back(easy3d::SurfaceMesh::Face(face_idx));
                ++face_idx;
            }
        }

        auto points = mesh->get_vertex_property<vec3>("v:point");
        mesh->update_vertex_normals();
        auto normals = mesh->get_vertex_property<vec3>("v:normal");

        TessellatorGen tessellator;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const Group &group = groups[i];
            if (group.empty())
                continue;

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            for (auto f : group) {
                tessellator.reset();
                tessellator.begin_polygon(mesh->compute_face_normal(f));
                tessellator.set_winding_rule(TessellatorGen::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : mesh->halfedges(f)) {
                    const vec3& v = points[mesh->to_vertex(h)];
                    const vec3& n = normals[mesh->to_vertex(h)];
                    const vec2& t = prop_texcoords[h];
                    const float data[8] = {v.x, v.y, v.z, n.x, n.y, n.z, t.x, t.y};
                    tessellator.add_vertex(data, 8);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles();
                const std::vector<const double *> &vts = tessellator.get_vertices();
                for (std::size_t j = 0; j < num; ++j) {
                    std::size_t a, b, c;
                    tessellator.get_triangle(j, a, b, c);
                    d_points.emplace_back(vts[a]);   d_normals.emplace_back(vts[a] + 3);    d_texcoords.emplace_back(vec2(vts[a] + 6));
                    d_points.emplace_back(vts[b]);   d_normals.emplace_back(vts[b] + 3);    d_texcoords.emplace_back(vec2(vts[b] + 6));
                    d_points.emplace_back(vts[c]);   d_normals.emplace_back(vts[c] + 3);    d_texcoords.emplace_back(vec2(vts[c] + 6));
                }
            }

            TrianglesDrawable *drawable = mesh->add_triangles_drawable("faces_" + std::to_string(i));

            drawable->update_vertex_buffer(d_points);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);
            drawable->set_smooth_shading(false);

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

        // the drawables have already been created
        return Viewer::add_model(mesh, false);
    }
#endif

}
