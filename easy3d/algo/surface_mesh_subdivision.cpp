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


#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    bool SurfaceMeshSubdivision::catmull_clark(SurfaceMesh *mesh) {
        if (!mesh)
            return false;

        auto points = mesh->vertex_property<vec3>("v:point");
        auto vfeature = mesh->get_vertex_property<bool>("v:feature");
        auto efeature = mesh->get_edge_property<bool>("e:feature");

        // reserve memory
        std::size_t nv = mesh->n_vertices();
        std::size_t ne = mesh->n_edges();
        std::size_t nf = mesh->n_faces();
        mesh->reserve(nv + ne + nf, 2 * ne + 4 * nf, 4 * nf);

        // get properties
        auto vpoint = mesh->add_vertex_property<vec3>("catmull:vpoint");
        auto epoint = mesh->add_edge_property<vec3>("catmull:epoint");
        auto fpoint = mesh->add_face_property<vec3>("catmull:fpoint");

        // compute face vertices
        for (auto f : mesh->faces()) {
            vec3 p(0, 0, 0);
            float c(0);
            for (auto v : mesh->vertices(f)) {
                p += points[v];
                ++c;
            }
            p /= c;
            fpoint[f] = p;
        }

        // compute edge vertices
        for (auto e : mesh->edges()) {
            // boundary or feature edge?
            if (mesh->is_boundary(e) || (efeature && efeature[e])) {
                epoint[e] = 0.5f * (points[mesh->vertex(e, 0)] +
                                    points[mesh->vertex(e, 1)]);
            }

                // interior edge
            else {
                vec3 p(0, 0, 0);
                p += points[mesh->vertex(e, 0)];
                p += points[mesh->vertex(e, 1)];
                p += fpoint[mesh->face(e, 0)];
                p += fpoint[mesh->face(e, 1)];
                p *= 0.25f;
                epoint[e] = p;
            }
        }

        // compute new positions for old vertices
        for (auto v : mesh->vertices()) {
            // isolated vertex?
            if (mesh->is_isolated(v)) {
                vpoint[v] = points[v];
            }

                // boundary vertex?
            else if (mesh->is_boundary(v)) {
                auto h1 = mesh->halfedge(v);
                auto h0 = mesh->prev_halfedge(h1);

                vec3 p = points[v];
                p *= 6.0;
                p += points[mesh->to_vertex(h1)];
                p += points[mesh->from_vertex(h0)];
                p *= 0.125;

                vpoint[v] = p;
            }

                // interior feature vertex?
            else if (vfeature && vfeature[v]) {
                vec3 p = points[v];
                p *= 6.0;
                int count(0);

                for (auto h : mesh->halfedges(v)) {
                    if (efeature[mesh->edge(h)]) {
                        p += points[mesh->to_vertex(h)];
                        ++count;
                    }
                }

                if (count == 2) // vertex is on feature edge
                {
                    p *= 0.125;
                    vpoint[v] = p;
                } else // keep fixed
                {
                    vpoint[v] = points[v];
                }
            }

                // interior vertex
            else {
                // weights from SIGGRAPH paper "Subdivision Surfaces in Character Animation"

                const float k = mesh->valence(v);
                vec3 p(0, 0, 0);

                for (auto vv : mesh->vertices(v))
                    p += points[vv];

                for (auto f : mesh->faces(v))
                    p += fpoint[f];

                p /= (k * k);

                p += ((k - 2.0f) / k) * points[v];

                vpoint[v] = p;
            }
        }

        // assign new positions to old vertices
        for (auto v : mesh->vertices()) {
            points[v] = vpoint[v];
        }

        // split edges
        for (auto e : mesh->edges()) {
            // feature edge?
            if (efeature && efeature[e]) {
                auto h = mesh->insert_vertex(e, epoint[e]);
                auto v = mesh->to_vertex(h);
                auto e0 = mesh->edge(h);
                auto e1 = mesh->edge(mesh->next_halfedge(h));

                vfeature[v] = true;
                efeature[e0] = true;
                efeature[e1] = true;
            }

                // normal edge
            else {
                mesh->insert_vertex(e, epoint[e]);
            }
        }

        // split faces
        for (auto f : mesh->faces()) {
            auto h0 = mesh->halfedge(f);
            mesh->insert_edge(h0, mesh->next_halfedge(mesh->next_halfedge(h0)));

            auto h1 = mesh->next_halfedge(h0);
            mesh->insert_vertex(mesh->edge(h1), fpoint[f]);

            auto h =
                    mesh->next_halfedge(mesh->next_halfedge(mesh->next_halfedge(h1)));
            while (h != h0) {
                mesh->insert_edge(h1, h);
                h = mesh->next_halfedge(
                        mesh->next_halfedge(mesh->next_halfedge(h1)));
            }
        }

        // clean-up properties
        mesh->remove_vertex_property(vpoint);
        mesh->remove_edge_property(epoint);
        mesh->remove_face_property(fpoint);

        return true;
    }


    bool SurfaceMeshSubdivision::loop(SurfaceMesh *mesh) {
        if (!mesh)
            return false;

        if (!mesh->is_triangle_mesh()) {
            LOG(WARNING) << "the Loop subdivision method works only for triangle meshes";
            return false;
        }

        auto points = mesh->vertex_property<vec3>("v:point");
        auto vfeature = mesh->get_vertex_property<bool>("v:feature");
        auto efeature = mesh->get_edge_property<bool>("e:feature");

        // reserve memory
        std::size_t nv = mesh->n_vertices();
        std::size_t ne = mesh->n_edges();
        std::size_t nf = mesh->n_faces();
        mesh->reserve(nv + ne, 2 * ne + 3 * nf, 4 * nf);

        // add properties
        auto vpoint = mesh->add_vertex_property<vec3>("loop:vpoint");
        auto epoint = mesh->add_edge_property<vec3>("loop:epoint");

        // compute vertex positions
        for (auto v : mesh->vertices()) {
            // isolated vertex?
            if (mesh->is_isolated(v)) {
                vpoint[v] = points[v];
            }

                // boundary vertex?
            else if (mesh->is_boundary(v)) {
                auto h1 = mesh->halfedge(v);
                auto h0 = mesh->prev_halfedge(h1);

                vec3 p = points[v];
                p *= 6.0;
                p += points[mesh->to_vertex(h1)];
                p += points[mesh->from_vertex(h0)];
                p *= 0.125;
                vpoint[v] = p;
            }

                // interior feature vertex?
            else if (vfeature && vfeature[v]) {
                vec3 p = points[v];
                p *= 6.0;
                int count(0);

                for (auto h : mesh->halfedges(v)) {
                    if (efeature[mesh->edge(h)]) {
                        p += points[mesh->to_vertex(h)];
                        ++count;
                    }
                }

                if (count == 2) // vertex is on feature edge
                {
                    p *= 0.125;
                    vpoint[v] = p;
                } else // keep fixed
                {
                    vpoint[v] = points[v];
                }
            }

                // interior vertex
            else {
                vec3 p(0, 0, 0);
                float k(0);

                for (auto vv : mesh->vertices(v)) {
                    p += points[vv];
                    ++k;
                }
                p /= k;

                float beta =
                        (0.625 - pow(0.375 + 0.25 * cos(2.0 * M_PI / k), 2.0));

                vpoint[v] = points[v] * (float) (1.0 - beta) + beta * p;
            }
        }

        // compute edge positions
        for (auto e : mesh->edges()) {
            // boundary or feature edge?
            if (mesh->is_boundary(e) || (efeature && efeature[e])) {
                epoint[e] =
                        (points[mesh->vertex(e, 0)] + points[mesh->vertex(e, 1)]) *
                        float(0.5);
            }

                // interior edge
            else {
                auto h0 = mesh->halfedge(e, 0);
                auto h1 = mesh->halfedge(e, 1);
                vec3 p = points[mesh->to_vertex(h0)];
                p += points[mesh->to_vertex(h1)];
                p *= 3.0;
                p += points[mesh->to_vertex(mesh->next_halfedge(h0))];
                p += points[mesh->to_vertex(mesh->next_halfedge(h1))];
                p *= 0.125;
                epoint[e] = p;
            }
        }

        // set new vertex positions
        for (auto v : mesh->vertices()) {
            points[v] = vpoint[v];
        }

        // insert new vertices on edges
        for (auto e : mesh->edges()) {
            // feature edge?
            if (efeature && efeature[e]) {
                auto h = mesh->insert_vertex(e, epoint[e]);
                auto v = mesh->to_vertex(h);
                auto e0 = mesh->edge(h);
                auto e1 = mesh->edge(mesh->next_halfedge(h));

                vfeature[v] = true;
                efeature[e0] = true;
                efeature[e1] = true;
            }

                // normal edge
            else {
                mesh->insert_vertex(e, epoint[e]);
            }
        }

        // split faces
        SurfaceMesh::Halfedge h;
        for (auto f : mesh->faces()) {
            h = mesh->halfedge(f);
            mesh->insert_edge(h, mesh->next_halfedge(mesh->next_halfedge(h)));
            h = mesh->next_halfedge(h);
            mesh->insert_edge(h, mesh->next_halfedge(mesh->next_halfedge(h)));
            h = mesh->next_halfedge(h);
            mesh->insert_edge(h, mesh->next_halfedge(mesh->next_halfedge(h)));
        }

        // clean-up properties
        mesh->remove_vertex_property(vpoint);
        mesh->remove_edge_property(epoint);

        return true;
    }


    bool SurfaceMeshSubdivision::sqrt3(SurfaceMesh *mesh) {
        if (!mesh)
            return false;

        // reserve memory
        int nv = mesh->n_vertices();
        int ne = mesh->n_edges();
        int nf = mesh->n_faces();
        mesh->reserve(nv + nf, ne + 3 * nf, 3 * nf);

        auto points = mesh->vertex_property<vec3>("v:point");

        // remember end of old vertices and edges
        auto vend = mesh->vertices_end();
        auto eend = mesh->edges_end();

        // compute new positions of old vertices
        auto new_pos = mesh->add_vertex_property<vec3>("v:np");
        for (auto v : mesh->vertices()) {
            if (!mesh->is_boundary(v)) {
                float n = mesh->valence(v);
                float alpha = (4.0 - 2.0 * cos(2.0 * M_PI / n)) / 9.0;
                vec3 p(0, 0, 0);

                for (auto vv : mesh->vertices(v))
                    p += points[vv];

                p = (1.0f - alpha) * points[v] + alpha / n * p;
                new_pos[v] = p;
            }
        }

        // split faces
        for (auto f : mesh->faces()) {
            vec3 p(0, 0, 0);
            float c(0);

            for (auto fv : mesh->vertices(f)) {
                p += points[fv];
                ++c;
            }

            p /= c;

            mesh->split(f, p);
        }

        // set new positions of old vertices
        for (auto vit = mesh->vertices_begin(); vit != vend; ++vit) {
            if (!mesh->is_boundary(*vit)) {
                points[*vit] = new_pos[*vit];
            }
        }

        mesh->remove_vertex_property(new_pos);

        // flip old edges
        for (auto eit = mesh->edges_begin(); eit != eend; ++eit) {
            if (mesh->is_flip_ok(*eit)) {
                mesh->flip(*eit);
            }
        }

        return true;
    }

} // namespace easy3d