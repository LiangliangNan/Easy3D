/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/


#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/tessellator.h>


namespace easy3d {

    SurfaceMesh *SurfaceMeshComponent::to_mesh() const {
        const SurfaceMeshComponent *comp = this;
        SurfaceMesh *mesh = comp->mesh();

        SurfaceMesh *result = new SurfaceMesh;

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
            result.add_point(points[v]);
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
