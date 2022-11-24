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

#include "viewer.h"

#include <unordered_map>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/file_system.h>


#include <3rd_party/fastobj/fast_obj.h>


namespace easy3d {


    TexturedViewer::TexturedViewer(const std::string &title)
            : Viewer(title) {
        camera()->setUpVector(vec3(0, 1, 0));
    }

    namespace internal {

        // each group is a set of faces (denoted by their indices) sharing the same material
        struct Group : public std::vector<SurfaceMesh::Face> {
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

        fastObjMesh *fom = fast_obj_read(file_name.c_str());
        if (!fom) {
            LOG(ERROR) << "failed reading file: " + file_name;
            return nullptr;
        }

        // Attention: Valid indices in the fastObjMesh::indices array start from 1.
        // A dummy position, normal and texture coordinate are added to the corresponding fastObjMesh arrays at
        // element 0 and then an index of 0 is used to indicate that attribute is not present at the vertex.

        // ------------------------ build the mesh ------------------------

        // clear the mesh in case of existing data
        auto mesh = new SurfaceMesh;
        mesh->set_name(file_name);

        SurfaceMeshBuilder builder(mesh);
        builder.begin_surface();

        // add vertices
        // skip the first point
        for (std::size_t v = 1; v < fom->position_count; ++v) {
            // Should I create vertices later, to get rid of isolated vertices?
            builder.add_vertex(vec3(fom->positions + v * 3));
        }

        // create texture coordinate property if texture coordinates present
        SurfaceMesh::HalfedgeProperty<vec2> prop_texcoords;
        if (fom->texcoord_count > 0 && fom->texcoords) // index starts from 1 and the first element is dummy
            prop_texcoords = mesh->add_halfedge_property<vec2>("h:texcoord");

        // create face color property if material information exists
        SurfaceMesh::FaceProperty<vec3> prop_face_color;
        if (fom->material_count > 0 && fom->materials)  // index starts from 1 and the first element is dummy
            prop_face_color = mesh->add_face_property<vec3>("f:color");

        // find the face's halfedge that points to v.
        auto find_face_halfedge = [](SurfaceMesh *mesh, SurfaceMesh::Face face,
                                     SurfaceMesh::Vertex v) -> SurfaceMesh::Halfedge {
            for (auto h : mesh->halfedges(face)) {
                if (mesh->target(h) == v)
                    return h;
            }
            LOG_N_TIMES(3, ERROR) << "could not find a halfedge pointing to " << v << " in face " << face
                                  << ". " << COUNTER;
            return SurfaceMesh::Halfedge();
        };

        // group the faces according to the material
        // each group is a set of faces sharing the same material
        std::vector<internal::Group> groups(fom->material_count);

        // for each shape
        for (std::size_t ii = 0; ii < fom->group_count; ii++) {
            const fastObjGroup &grp = fom->groups[ii];

//                if (grp.name)
//                    std::cout << "group name: " << std::string(grp.name) << std::endl;

            unsigned int idx = 0;
            for (unsigned int jj = 0; jj < grp.face_count; ++jj) {
                // number of vertices in the face
                unsigned int fv = fom->face_vertices[grp.face_offset + jj];
                std::vector<SurfaceMesh::Vertex> vertices;
                std::vector<unsigned int> texcoord_ids;
                for (unsigned int kk = 0; kk < fv; ++kk) {  // for each vertex in the face
                    const fastObjIndex &mi = fom->indices[grp.index_offset + idx];
                    if (mi.p)
                        vertices.emplace_back(SurfaceMesh::Vertex(mi.p - 1));
                    if (mi.t)
                        texcoord_ids.emplace_back(mi.t);
                    ++idx;
                }

                SurfaceMesh::Face face = builder.add_face(vertices);
                if (face.is_valid()) {
                    // texture coordinates
                    if (prop_texcoords && texcoord_ids.size() == vertices.size()) {
                        auto begin = find_face_halfedge(mesh, face, builder.face_vertices()[0]);
                        auto cur = begin;
                        unsigned int vid = 0;
                        do {
                            unsigned int tid = texcoord_ids[vid++];
                            prop_texcoords[cur] = vec2(fom->texcoords + 2 * tid);
                            cur = mesh->next(cur);
                        } while (cur != begin);
                    }

                    // now materials
                    if (prop_face_color) {
                        unsigned int mat_id = fom->face_materials[grp.face_offset + jj];
                        const fastObjMaterial &mat = fom->materials[mat_id];
                        prop_face_color[face] = vec3(mat.Kd); // currently easy3d uses only diffuse
                    }

                    auto get_file_name = [](const char *name, const char* path) -> std::string {
                        std::string file_name("");
                        if (name && file_system::is_file(std::string(name)))
                            file_name = std::string(name);
                        else if (path && file_system::is_file(std::string(path)))
                            file_name = std::string(path);
                        else if (name && path){
                            const std::string test_name = std::string(path) + "/" + std::string(name);
                            if (file_system::is_file(test_name))
                                file_name = test_name;
                        }
                        return file_name;
                    };

                    if (fom->material_count > 0 && fom->materials) {
                        unsigned int mat_id = fom->face_materials[grp.face_offset + jj];
                        const fastObjMaterial &mat = fom->materials[mat_id];
                        auto &g = groups[mat_id];
                        g.push_back(face);
                        g.ambient = vec3(mat.Ka);
                        g.diffuse = vec3(mat.Kd);
                        g.specular = vec3(mat.Ks);
                        g.shininess = mat.Ns;
                        g.tex_file = get_file_name(mat.map_Ka.name, mat.map_Ka.path); // use ambient texture it exists
                        if (g.tex_file.empty())
                            g.tex_file = get_file_name(mat.map_Kd.name, mat.map_Kd.path); // then try diffuse texture
                        if (g.tex_file.empty())
                            g.tex_file = get_file_name(mat.map_Ks.name, mat.map_Ks.path); // then try diffuse texture
                    }
                }
            }
        }

        builder.end_surface();

        // since the mesh has been built, skip texture if material and texcoord information don't exist
        if ((fom->material_count > 0 && fom->materials) == false) {
            Viewer::add_model(mesh, true);
            return mesh;
        }
        else
            Viewer::add_model(mesh, false);

        mesh->update_vertex_normals();
        auto normals = mesh->get_vertex_property<vec3>("v:normal");
        auto points = mesh->get_vertex_property<vec3>("v:point");

        Tessellator tessellator;
        for (std::size_t i = 0; i < groups.size(); ++i) {
            const auto &group = groups[i];
            if (group.empty())
                continue;

            tessellator.reset();

            for (auto face : group) {
                tessellator.begin_polygon(mesh->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : mesh->halfedges(face)) {
                    auto v = mesh->target(h);
                    Tessellator::Vertex vtx(points[v], v.idx());
                    vtx.append(normals[v]);
                    if (prop_texcoords)
                        vtx.append(prop_texcoords[h]);
                    if (prop_face_color)
                        vtx.append(prop_face_color[face]);
                    tessellator.add_vertex(vtx);
                }
                tessellator.end_contour();
                tessellator.end_polygon();
            }

            std::vector<vec3> d_points, d_colors, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex*>& vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                if (prop_texcoords) {
                    d_texcoords.emplace_back(v->data() + offset);
                    offset += 2;
                }
                if (prop_face_color)
                    d_colors.emplace_back(v->data() + offset);
            }

            const auto &d_indices = tessellator.elements();

            auto drawable = mesh->renderer()->add_triangles_drawable("faces_" + std::to_string(i));

            drawable->update_element_buffer(d_indices);
            drawable->update_vertex_buffer(d_points);
            drawable->update_normal_buffer(d_normals);
            if (!d_colors.empty())
                drawable->update_color_buffer(d_colors);
            if (!d_texcoords.empty())
                drawable->update_texcoord_buffer(d_texcoords);

            drawable->set_smooth_shading(false);
            if (prop_texcoords) {
                if (!group.tex_file.empty()) {
                    Texture *tex = TextureManager::request(group.tex_file, Texture::REPEAT);
                    if (tex) {
                        drawable->set_texture_coloring(State::HALFEDGE, "h:texcoord", tex);
                        drawable->set_distinct_back_color(false);
                        LOG(INFO) << "texture created from " << group.tex_file;
                    }
                }
            }

            if (!drawable->texture()) { // in case texture creation failed
                if (prop_face_color)
                    drawable->set_property_coloring(State::Location::FACE, "f:color");
                else
                    drawable->set_uniform_coloring(vec4(group.diffuse, 1.0f));
            }
        }

        return mesh;
    }

}
