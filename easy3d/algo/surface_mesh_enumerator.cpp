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


#include <easy3d/algo/surface_mesh_enumerator.h>

#include <stack>


namespace easy3d {

    void SurfaceMeshEnumerator::propagate_connected_component(SurfaceMesh *mesh, SurfaceMesh::VertexProperty<int> id,
                                                              SurfaceMesh::Vertex seed, int cur_id) {
        std::stack<SurfaceMesh::Vertex> stack;
        stack.push(seed);

        while (!stack.empty()) {
            SurfaceMesh::Vertex top = stack.top();
            stack.pop();
            if (id[top] == -1) {
                id[top] = cur_id;
                for (auto v : mesh->vertices(top)) {
                    if (id[v] == -1) {
                        stack.push(v);
                    }
                }
            }
        }
    }


    int SurfaceMeshEnumerator::enumerate_connected_components(SurfaceMesh *mesh, SurfaceMesh::VertexProperty<int> id) {
        id.vector().assign(mesh->n_vertices(), -1);

        int cur_id = 0;
        for (auto v : mesh->vertices()) {
            if (id[v] == -1) {
                propagate_connected_component(mesh, id, v, cur_id);
                ++cur_id;
            }
        }
        return cur_id;
    }


    void SurfaceMeshEnumerator::propagate_connected_component(SurfaceMesh *mesh, SurfaceMesh::FaceProperty<int> id,
                                                              SurfaceMesh::Face seed, int cur_id) {
        std::stack<SurfaceMesh::Face> stack;
        stack.push(seed);

        while (!stack.empty()) {
            SurfaceMesh::Face top = stack.top();
            stack.pop();
            if (id[top] == -1) {
                id[top] = cur_id;

                for (auto h : mesh->halfedges(top)) {
                    auto op = mesh->opposite(h);
                    if (!mesh->is_border(op)) {
                        auto f = mesh->face(op);
                        if (id[f] == -1) {
                            stack.push(f);
                        }
                    }
                }
            }
        }
    }


    int SurfaceMeshEnumerator::enumerate_connected_components(SurfaceMesh *mesh, SurfaceMesh::FaceProperty<int> id) {
        id.vector().assign(mesh->n_faces(), -1);

        int cur_id = 0;
        for (auto f : mesh->faces()) {
            if (id[f] == -1) {
                propagate_connected_component(mesh, id, f, cur_id);
                ++cur_id;
            }
        }

        return cur_id;
    }


    void SurfaceMeshEnumerator::propagate_planar_component(SurfaceMesh *mesh,
                                                           SurfaceMesh::FaceProperty<int> id,
                                                           SurfaceMesh::Face seed, int cur_id,
                                                           float angle_threshold
    ) {
        auto fnormals = mesh->get_face_property<vec3>("f:normal");
        auto is_degenerate = mesh->get_face_property<bool>("f:SurfaceMeshEnumerator:is_degenerate");

        std::stack<SurfaceMesh::Face> stack;
        stack.push(seed);

        while (!stack.empty()) {
            SurfaceMesh::Face top = stack.top();
            stack.pop();
            if (id[top] == -1) {
                id[top] = cur_id;
                const vec3 &n_top = fnormals[top];
                for (auto h : mesh->halfedges(top)) {
                    auto cur = mesh->face(mesh->opposite(h));
                    if (cur.is_valid() && id[cur] == -1 && !is_degenerate[cur]) {
                        const vec3 &n_cur = fnormals[cur];
                        auto angle = geom::angle(n_top, n_cur); // in [-pi, pi]
                        angle = rad2deg(std::abs(angle));
                        if (std::abs(angle) < angle_threshold)
                            stack.push(cur);
                    }
                }
            }
        }
    }


    int SurfaceMeshEnumerator::enumerate_planar_components(
            SurfaceMesh *mesh, SurfaceMesh::FaceProperty<int> id,
            float angle_threshold)
    {
        mesh->update_face_normals();
        auto is_degenerate = mesh->add_face_property<bool>("f:SurfaceMeshEnumerator:is_degenerate", false);

        int num_degenerate = 0;
        for (auto f : mesh->faces()) {
            id[f] = -1;
            if (mesh->is_degenerate(f)) {
                is_degenerate[f] = true;
                ++num_degenerate;
            }
        }

        int cur_id = 0;
        for (auto f : mesh->faces()) {
            if (!is_degenerate[f] && id[f] == -1) {
                propagate_planar_component(mesh, id, f, cur_id, angle_threshold);
                cur_id++;
            }
        }

        if (num_degenerate > 0) { // propagate the planar partition to degenerate faces
            LOG(WARNING) << "model has " << num_degenerate << " degenerate faces" << std::endl;
            int num_propagated = 0;
            do {
                num_propagated = 0;
                for (auto e : mesh->edges()) {
                    auto f0 = mesh->face(mesh->halfedge(e, 0));
                    auto f1 = mesh->face(mesh->halfedge(e, 1));
                    if (f0.is_valid() && f1.is_valid()) {
                        if (is_degenerate[f0] && id[f0] == -1 && !is_degenerate[f1]) {
                            id[f0] = id[f1];
                            ++num_propagated;
                        }
                    }
                }
            } while (num_propagated > 0);
        }

        mesh->remove_face_property(is_degenerate);
        return cur_id;
    }

}