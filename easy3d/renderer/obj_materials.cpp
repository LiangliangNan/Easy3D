// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/obj_materials.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/file_system.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#define FAST_OBJ_IMPLEMENTATION
#include <3rd_party/fastobj/fast_obj.h>

namespace easy3d {
namespace {
std::string texture_path(const fastObjTexture& map,const std::string& obj) {
    if (map.name && file_system::is_absolute_path(map.name) && file_system::is_file(map.name)) return map.name;
    if (map.path && file_system::is_file(map.path)) return map.path;
    if (map.name) {
        const auto relative=file_system::parent_directory(obj)+"/"+map.name;
        if (file_system::is_file(relative)) return relative;
    }
    return {};
}
}
ObjMaterialMesh load_obj_materials(const std::string& file) {
    const auto absolute=file_system::absolute_path(file);
    std::unique_ptr<fastObjMesh,decltype(&fast_obj_destroy)> source(fast_obj_read(absolute.c_str()),fast_obj_destroy);
    if (!source) throw std::runtime_error("could not read OBJ: "+file);
    const auto& obj=*source;
    if (obj.position_count<=1 || obj.position_count-1>unsigned(std::numeric_limits<int>::max()) || !obj.face_count)
        throw std::runtime_error("OBJ has no renderable surface: "+file);
    ObjMaterialMesh result; result.mesh=std::make_shared<SurfaceMesh>(); result.mesh->set_name(file);
    auto& mesh=*result.mesh;
    auto uv=mesh.add_halfedge_property<vec2>("h:texcoord");
    auto color=mesh.add_face_property<vec3>("f:color",vec3(1));
    // Split incomplete-UV faces from textured parts instead of sampling dummy UVs.
    std::map<std::pair<unsigned,bool>,std::vector<SurfaceMesh::Face>> groups;
    SurfaceMeshBuilder builder(&mesh); builder.begin_surface();
    // Defer errors until end_surface() so the builder always completes its lifecycle.
    std::string error;
    for (unsigned i=1;i<obj.position_count;++i) {
        const vec3 point(obj.positions+i*3);
        if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) error="non-finite vertex";
        builder.add_vertex(point);
    }
    std::size_t offset=0;
    for (unsigned i=0;i<obj.face_count && error.empty();++i) {
        const unsigned count=obj.face_vertices[i];
        if (count<3 || offset>std::numeric_limits<std::size_t>::max()-count) { error="invalid face size"; break; }
        const unsigned material=obj.material_count ? obj.face_materials[i] : 0;
        if (obj.material_count && material>=obj.material_count) { error="invalid material index"; break; }
        bool complete_uv=true;
        std::vector<SurfaceMesh::Vertex> vertices; std::vector<vec2> coords;
        for (unsigned j=0;j<count;++j) {
            const auto& index=obj.indices[offset+j];
            if (!index.p || index.p>=obj.position_count || (index.t && index.t>=obj.texcoord_count)) { error="invalid position/UV index"; break; }
            vertices.emplace_back(int(index.p-1));
            const vec2 coord=index.t ? vec2(obj.texcoords+index.t*2) : vec2(0);
            if (!std::isfinite(coord.x) || !std::isfinite(coord.y)) { error="non-finite UV"; break; }
            coords.push_back(coord); complete_uv=complete_uv && index.t!=0;
        }
        offset+=count;
        if (!error.empty()) break;
        const auto face=builder.add_face(vertices);
        if (!face.is_valid()) { error="face rejected by mesh builder"; break; }
        // Builder may duplicate vertices; its ordered face_vertices preserves the
        // source corner order. The mesh circulator's starting corner is arbitrary.
        SurfaceMesh::Halfedge begin;
        for (auto h:mesh.halfedges(face)) if (mesh.target(h)==builder.face_vertices()[0]) { begin=h; break; }
        if (!begin.is_valid()) { error="could not map OBJ face corners"; break; }
        auto h=begin;
        for (unsigned j=0;j<count;++j) { uv[h]=coords[j]; h=mesh.next(h); }
        if (obj.material_count) color[face]=vec3(obj.materials[material].Kd);
        groups[{material,complete_uv}].push_back(face);
    }
    builder.end_surface(false);
    if (!error.empty()) throw std::runtime_error("invalid OBJ ("+error+"): "+file);
    mesh.update_vertex_normals();
    const auto normals=mesh.get_vertex_property<vec3>("v:normal");
    Tessellator tessellator;
    for (const auto& group:groups) {
        ObjMaterialMesh::Part part;
        part.name="faces_"+std::to_string(group.first.first)+(group.first.second ? "" : "_untextured");
        if (obj.material_count) {
            const auto& material=obj.materials[group.first.first];
            part.diffuse=vec3(material.Kd);
            if (group.first.second) {
                part.texture_file=texture_path(material.map_Kd,absolute);
                // Retain the original tutorial's color-map fallbacks for assets
                // that only supply ambient/specular maps, preferring diffuse.
                if (part.texture_file.empty()) part.texture_file=texture_path(material.map_Ka,absolute);
                if (part.texture_file.empty()) part.texture_file=texture_path(material.map_Ks,absolute);
            }
        }
        for (unsigned j=0;j<3;++j)
            if (!std::isfinite(part.diffuse[j])) throw std::runtime_error("non-finite OBJ diffuse color: "+file);
        tessellator.reset();
        for (auto face:group.second) {
            const auto before=tessellator.elements().size();
            tessellator.begin_polygon(mesh.compute_face_normal(face));
            tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);
            tessellator.begin_contour();
            for (auto h:mesh.halfedges(face)) {
                const auto v=mesh.target(h);
                Tessellator::Vertex vertex(mesh.position(v),v.idx());
                vertex.append(normals[v]); vertex.append(uv[h]);
                tessellator.add_vertex(vertex);
            }
            tessellator.end_contour(); tessellator.end_polygon();
            part.source_faces.insert(part.source_faces.end(),tessellator.elements().size()-before,face.idx());
            if (tessellator.elements().size()==before) throw std::runtime_error("OBJ face produced no triangles: "+file);
        }
        for (const auto* v:tessellator.vertices()) {
            part.positions.emplace_back(v->data()); part.normals.emplace_back(v->data()+3); part.texcoords.emplace_back(v->data()+6);
        }
        for (const auto& triangle:tessellator.elements())
            part.indices.insert(part.indices.end(),triangle.begin(),triangle.end());
        result.parts.push_back(std::move(part));
    }
    return result;
}
void ObjMaterialMesh::create_drawables() const {
    if (!mesh || !mesh->renderer()) throw std::logic_error("attach OBJ mesh to a renderer before creating material drawables");
    auto& renderer=*mesh->renderer();
    for (const auto& part:parts)
        for (const auto& existing:renderer.triangles_drawables())
            if (existing->name()==part.name) throw std::logic_error("OBJ material drawable already exists: "+part.name);
    for (const auto& part:parts) {
        auto* d=renderer.add_triangles_drawable(part.name);
        // Material parts are import snapshots. A general Renderer::update() must
        // not replace each part with a tessellation of the entire model.
        const auto snapshot=std::make_shared<Part>(part);
        const auto upload=[snapshot](Model*,Drawable* target) {
            target->update_vertex_buffer(snapshot->positions); target->update_normal_buffer(snapshot->normals);
            target->update_texcoord_buffer(snapshot->texcoords); target->update_element_buffer(snapshot->indices);
            if (target->has_cpu_buffers()) target->set_source_faces(snapshot->source_faces);
        };
        d->set_update_func(upload);
        upload(mesh.get(),d);
        d->set_smooth_shading(false); d->set_distinct_back_color(false);
        d->set_uniform_coloring(vec4(part.diffuse,1));
        if (!part.texture_file.empty()) {
            auto* texture=d->has_cpu_buffers() ? TextureManager::request_cpu(part.texture_file,Texture::REPEAT) :
                TextureManager::request(part.texture_file,Texture::REPEAT);
            if (texture) d->set_texture_coloring(State::HALFEDGE,"h:texcoord",texture);
        }
    }
}
}
