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


#include <easy3d/algo/surface_mesh_features.h>


namespace easy3d {

    SurfaceMeshFeatures::SurfaceMeshFeatures(SurfaceMesh *mesh) : mesh_(mesh) {
        vfeature_ = mesh_->vertex_property("v:feature", false);
        efeature_ = mesh_->edge_property("e:feature", false);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshFeatures::clear() {
        for (auto v : mesh_->vertices())
            vfeature_[v] = false;

        for (auto e : mesh_->edges())
            efeature_[e] = false;
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshFeatures::detect_boundary() {
        for (auto v : mesh_->vertices())
            if (mesh_->is_border(v))
                vfeature_[v] = true;

        for (auto e : mesh_->edges())
            if (mesh_->is_border(e))
                efeature_[e] = true;
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshFeatures::detect_angle(float angle) {
        const float feature_cosine = cos(angle / 180.0 * M_PI);

        for (auto e : mesh_->edges()) {
            if (!mesh_->is_border(e)) {
                const auto f0 = mesh_->face(mesh_->halfedge(e, 0));
                const auto f1 = mesh_->face(mesh_->halfedge(e, 1));

                const vec3 n0 = mesh_->compute_face_normal(f0);
                const vec3 n1 = mesh_->compute_face_normal(f1);

                if (dot(n0, n1) < feature_cosine) {
                    efeature_[e] = true;
                    vfeature_[mesh_->vertex(e, 0)] = true;
                    vfeature_[mesh_->vertex(e, 1)] = true;
                }
            }
        }
    }

} // namespace easy3d