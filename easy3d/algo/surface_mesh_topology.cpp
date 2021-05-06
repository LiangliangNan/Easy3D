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

#include <easy3d/algo/surface_mesh_topology.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_components.h>

namespace easy3d {


    SurfaceMeshTopology::SurfaceMeshTopology(const SurfaceMeshComponent *comp) {
        component_ = comp;
        SurfaceMesh *mesh = comp->mesh();

        number_of_borders_ = 0;
        largest_border_size_ = 0;

        auto is_marked = mesh->add_halfedge_property<bool>("SurfaceMeshTopology:is_marked");

        for (auto h : comp->halfedges()) {
            auto cur = h;
            if (mesh->is_border(cur) && !is_marked[cur]) {
                ++number_of_borders_;
                std::size_t border_size = 0;
                do {
                    ++border_size;
                    is_marked[cur] = true;
                    cur = mesh->next(cur);
                } while (cur != h);
                largest_border_size_ = std::max(largest_border_size_, border_size);
            }
        }

        mesh->remove_halfedge_property(is_marked);
    }


    int SurfaceMeshTopology::euler_poincare() const {
        return component_->n_vertices() + component_->n_faces() - component_->n_edges();
    }

}