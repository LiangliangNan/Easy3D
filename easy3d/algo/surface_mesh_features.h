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
        /**
         * \brief Construct with mesh to be analyzed.
         * \param mesh The surface mesh to be analyzed.
         */
        explicit SurfaceMeshFeatures(SurfaceMesh *mesh);

        /**
         * \brief Clear features.
         * \details This function clears all previously detected features.
         */
        void clear();

        /**
         * \brief Mark all boundary edges as features.
         * \details This function detects and marks all boundary edges of the mesh as feature edges.
         */
        void detect_boundary();

        /**
         * \brief Mark edges with dihedral angle larger than \p angle as feature.
         * \param angle The dihedral angle threshold.
         * \details This function detects and marks edges with a dihedral angle larger than the specified angle as feature edges.
         */
        void detect_angle(float angle);

    private:
        SurfaceMesh *mesh_; //!< The mesh to be analyzed.

        // Property handles
        SurfaceMesh::VertexProperty<bool> vfeature_;
        SurfaceMesh::EdgeProperty<bool> efeature_;
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_FEATURES_H