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

    void SurfaceMeshEnumerator::propagate_connected_component(SurfaceMesh* mesh, SurfaceMesh::VertexProperty<int> id, SurfaceMesh::Vertex seed, int cur_id) {
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
        for (auto v : mesh->vertices())
            id[v] = -1;

        int cur_id = 0;
        for (auto v : mesh->vertices()) {
            if (id[v] == -1) {
                propagate_connected_component(mesh, id, v, cur_id);
                ++cur_id;
            }
        }
        return cur_id;
    }


    void SurfaceMeshEnumerator::propagate_connected_component(SurfaceMesh* mesh, SurfaceMesh::FaceProperty<int> id, SurfaceMesh::Face seed, int cur_id) {
        std::stack<SurfaceMesh::Face> stack;
        stack.push(seed);

        while (!stack.empty()) {
            SurfaceMesh::Face top = stack.top();
            stack.pop();
            if (id[top] == -1) {
                id[top] = cur_id;

                for (auto h : mesh->halfedges(top)) {
                    auto op = mesh->opposite_halfedge(h);
                    if (mesh->is_boundary(op)) {
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
        for (auto f : mesh->faces())
            id[f] = -1;

        int cur_id = 0;
        for (auto f : mesh->faces()) {
            if (id[f] == -1) {
                propagate_connected_component(mesh, id, f, cur_id);
                ++cur_id;
            }
        }
        return cur_id;
    }

}