/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
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
        const float feature_cosine = std::cos(angle / 180.0 * M_PI);

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