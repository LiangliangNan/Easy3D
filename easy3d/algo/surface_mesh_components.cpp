/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#include <easy3d/algo/surface_mesh_components.h>

#include <algorithm>

#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/tessellator.h>


namespace easy3d {


    namespace internal {
        // copy vertex property from mesh to component
        template<typename FT>
        inline void copy_vertex_property(SurfaceMesh *mesh, SurfaceMesh *comp_mesh,
                                         SurfaceMesh::VertexProperty<SurfaceMesh::Vertex> orig_vertex,
                                         const std::string &prop_name) {
            // test if the property already exists
            auto prop = mesh->get_vertex_property<FT>(prop_name);
            if (prop && !comp_mesh->get_vertex_property<FT>(prop_name)) {
                auto new_prop = comp_mesh->add_vertex_property<FT>(prop_name);
                for (auto v: comp_mesh->vertices())
                    new_prop[v] = prop[orig_vertex[v]];
            }
        }

        // copy face property from mesh to component
        template<typename FT>
        inline void copy_face_property(SurfaceMesh *mesh, SurfaceMesh *comp_mesh,
                                       SurfaceMesh::FaceProperty<SurfaceMesh::Face> orig_face,
                                       const std::string &prop_name) {
            // test if the property already exists
            auto prop = mesh->get_face_property<FT>(prop_name);
            if (prop && !comp_mesh->get_face_property<FT>(prop_name)) {
                auto new_prop = comp_mesh->add_face_property<FT>(prop_name);
                for (auto f: comp_mesh->faces())
                    new_prop[f] = prop[orig_face[f]];
            }
        }
    }


    SurfaceMesh *SurfaceMeshComponent::to_mesh() const {
        const SurfaceMeshComponent *comp = this;
        SurfaceMesh *mesh = comp->mesh();
        auto result = new SurfaceMesh;

        auto points = mesh->get_vertex_property<vec3>("v:point");
        auto vertex_id = mesh->add_vertex_property<int>("SurfaceMeshComponent::construct_mesh:vertex_id");
        auto orig_vertex = result->add_vertex_property<SurfaceMesh::Vertex>("SurfaceMeshComponent::construct_mesh:orig_vertex");
        int id = 0;
        for (auto v : comp->vertices()) {
            auto vtx = result->add_vertex(points[v]);
            orig_vertex[vtx] = v;
            vertex_id[v] = id;
            ++id;
        }

        auto orig_face = result->add_face_property<SurfaceMesh::Face>("SurfaceMeshComponent::construct_mesh:orig_face");
        for (auto f : comp->faces()) {
            std::vector<SurfaceMesh::Vertex> vts;
            for (auto v : mesh->vertices(f)) {
                int idx = vertex_id[v];
                vts.emplace_back(SurfaceMesh::Vertex(idx));
            }
            auto face = result->add_face(vts);
            orig_face[face] = f;
        }
        mesh->remove_vertex_property(vertex_id);

        // handle vertex attributes (only 'float' and 'vec3' are processed)
        const auto& v_prop_names = mesh->vertex_properties();
        for (const auto& prop_name : v_prop_names) {
            // the existence of each property will be checked within each copy function
            internal::copy_vertex_property<float>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<double>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<int>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<unsigned int>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<std::size_t>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<bool>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<char>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<unsigned char>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<vec2>(mesh, result, orig_vertex, prop_name);
            internal::copy_vertex_property<vec3>(mesh, result, orig_vertex, prop_name);
        }
        result->remove_vertex_property(orig_vertex);

        // note: handle face attributes (only 'float' and 'vec3' are processed)
        const auto& f_prop_names = mesh->face_properties();
        for (const auto& prop_name : f_prop_names) {
            // the existence of each property will be checked within each copy function
            internal::copy_face_property<float>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<double>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<int>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<unsigned int>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<std::size_t>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<bool>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<char>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<unsigned char>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<vec2>(mesh, result, orig_face, prop_name);
            internal::copy_face_property<vec3>(mesh, result, orig_face, prop_name);
        }
        result->remove_face_property(orig_face);

        return result;
    }


    float SurfaceMeshComponent::area() const {
        auto normals = mesh_->get_face_property<vec3>("f:normal");
        if (!normals) {
            mesh_->update_face_normals();
            normals = mesh_->get_face_property<vec3>("f:normal");
        }

        auto points = mesh_->get_vertex_property<vec3>("v:point");

        Tessellator tess;
        tess.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE

        for (auto f : faces_) {
            tess.begin_polygon(normals[f]);
            tess.begin_contour();
            for (auto v : mesh_->vertices(f)) {
                tess.add_vertex(points[v]);
            }
            tess.end_contour();
            tess.end_polygon();
        }

        float area = 0.0f;
        const std::vector<Tessellator::Vertex *> &vts = tess.vertices();
        const auto& triangles = tess.elements();
        for (const auto& t : triangles) {
            const vec3 a(vts[t[0]]->data());
            const vec3 b(vts[t[1]]->data());
            const vec3 c(vts[t[2]]->data());
            area += geom::triangle_area(a, b, c);
        }

        return area;
    }


    float SurfaceMeshComponent::border_length() const {
        auto points = mesh_->get_vertex_property<vec3>("v:point");

        float length = 0.0f;
        for (auto h : halfedges_) {
            if (mesh_->is_border(h)) {
                auto s = mesh_->source(h);
                auto t = mesh_->target(h);
                length += distance(points[s], points[t]);
            }
        }

        return length;
    }


    Box3 SurfaceMeshComponent::bbox() const {
        auto points = mesh_->get_vertex_property<vec3>("v:point");
        Box3 result;
        for (auto v : vertices_)
            result.grow(points[v]);
        return result;
    }


    void SurfaceMeshComponent::translate(const vec3 &offset) {
        auto points = mesh_->get_vertex_property<vec3>("v:point");
        for (auto v : vertices_)
            points[v] = points[v] + offset;
    }


    std::vector<SurfaceMeshComponent> SurfaceMeshComponent::extract(SurfaceMesh *mesh, bool descending) {
        auto component_id = mesh->add_vertex_property<int>("SurfaceMeshComponentExtractor::extract::component_id");
        int nb_components = SurfaceMeshEnumerator::enumerate_connected_components(mesh, component_id);
        std::vector<SurfaceMeshComponent> result;
        for (int i = 0; i < nb_components; i++) {
            result.emplace_back(SurfaceMeshComponent(mesh));
        }

        for (auto v : mesh->vertices()) {
            int idx = component_id[v];
            result[idx].vertices_.push_back(v);
        }

        for (auto f : mesh->faces()) {
            auto v = mesh->vertices(f).begin();
            int idx = component_id[*v];
            result[idx].faces_.push_back(f);
        }

        for (auto e : mesh->edges()) {
            auto v = mesh->vertex(e, 0);
            int idx = component_id[v];
            result[idx].edges_.push_back(e);
        }

        for (auto h : mesh->halfedges()) {
            auto v = mesh->target(h);
            int idx = component_id[v];
            result[idx].halfedges_.push_back(h);
        }

        mesh->remove_vertex_property(component_id);
        
        if (descending) {
            std::sort(result.begin(), result.end(), 
                [](const SurfaceMeshComponent& a, const SurfaceMeshComponent& b) { return a.n_faces() > b.n_faces(); }
            );
        }

        return result;
    }


    SurfaceMeshComponent SurfaceMeshComponent::extract(SurfaceMesh *mesh, SurfaceMesh::Face face) {
        auto component_id = mesh->add_vertex_property<int>("SurfaceMeshComponentExtractor::extract::component_id");
        SurfaceMeshEnumerator::enumerate_connected_components(mesh, component_id);

        SurfaceMeshComponent result(mesh);
        auto vertex = mesh->vertices(face).begin();
        int comp_id = component_id[*vertex];

        for (auto v : mesh->vertices()) {
            if (component_id[v] == comp_id)
                result.vertices_.push_back(v);
        }

        for (auto f : mesh->faces()) {
            auto v = mesh->vertices(f).begin();
            if (component_id[*v] == comp_id)
                result.faces_.push_back(f);
        }

        for (auto e : mesh->edges()) {
            auto v = mesh->vertex(e, 0);
            if (component_id[v] == comp_id)
                result.edges_.push_back(e);
        }

        for (auto h : mesh->halfedges()) {
            auto v = mesh->target(h);
            if (component_id[v] == comp_id)
                result.halfedges_.push_back(h);
        }

        return result;
    }


    SurfaceMeshComponent SurfaceMeshComponent::extract(SurfaceMesh *mesh, SurfaceMesh::Vertex vertex) {
        auto component_id = mesh->add_vertex_property<int>("SurfaceMeshComponentExtractor::extract::component_id");
        SurfaceMeshEnumerator::enumerate_connected_components(mesh, component_id);

        SurfaceMeshComponent result(mesh);
        int comp_id = component_id[vertex];

        for (auto v : mesh->vertices()) {
            if (component_id[v] == comp_id)
                result.vertices_.push_back(v);
        }

        for (auto f : mesh->faces()) {
            auto v = mesh->vertices(f).begin();
            if (component_id[*v] == comp_id)
                result.faces_.push_back(f);
        }

        for (auto e : mesh->edges()) {
            auto v = mesh->vertex(e, 0);
            if (component_id[v] == comp_id)
                result.edges_.push_back(e);
        }

        for (auto h : mesh->halfedges()) {
            auto v = mesh->target(h);
            if (component_id[v] == comp_id)
                result.halfedges_.push_back(h);
        }

        return result;
    }


}
