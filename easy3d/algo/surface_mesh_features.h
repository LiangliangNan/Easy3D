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

#ifndef EASY3D_ALGO_SURFACE_MESH_FEATURES_H
#define EASY3D_ALGO_SURFACE_MESH_FEATURES_H

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /// \brief Detect and mark feature edges based on boundary or dihedral angle
    /// \class SurfaceMeshFeatures easy3d/algo/surface_mesh_features.h
    class SurfaceMeshFeatures {
    public:
        //! \brief Construct with mesh to be analyzed.
        explicit SurfaceMeshFeatures(SurfaceMesh *mesh);

        //! \brief Clear features.
        void clear();

        //! \brief Mark all boundary edges as features.
        void detect_boundary();

        //! \brief Mark edges with dihedral angle larger than \p angle as feature.
        void detect_angle(float angle);

    private:
        SurfaceMesh *mesh_;

        SurfaceMesh::VertexProperty<bool> vfeature_;
        SurfaceMesh::EdgeProperty<bool> efeature_;
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_FEATURES_H