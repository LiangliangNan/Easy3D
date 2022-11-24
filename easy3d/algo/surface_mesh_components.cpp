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
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/tessellator.h>


namespace easy3d {

    SurfaceMesh *SurfaceMeshComponent::to_mesh() const {
        const SurfaceMeshComponent *comp = this;
        SurfaceMesh *mesh = comp->mesh();
        auto result = new SurfaceMesh;

        auto points = mesh->get_vertex_property<vec3>("v:point");
        auto vertex_id = mesh->add_vertex_property<int>("SurfaceMeshComponent::construct_mesh:vertex_id");
        int id = 0;
        for (auto v : comp->vertices()) {
            result->add_vertex(points[v]);
            vertex_id[v] = id;
            ++id;
        }

        for (auto f : comp->faces()) {
            std::vector<SurfaceMesh::Vertex> vts;
            for (auto v : mesh->vertices(f)) {
                int idx = vertex_id[v];
                vts.emplace_back(SurfaceMesh::Vertex(idx));
            }
            result->add_face(vts);
        }

        mesh->remove_vertex_property(vertex_id);

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


    std::vector<SurfaceMeshComponent> SurfaceMeshComponent::extract(SurfaceMesh *mesh) {
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
