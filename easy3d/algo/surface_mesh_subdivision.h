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

#ifndef EASY3D_ALGO_MESH_SUBDIVISION_H
#define EASY3D_ALGO_MESH_SUBDIVISION_H


namespace easy3d {

    class SurfaceMesh;

    /// \brief SurfaceMeshSubdivision implement several well-known subdivision algorithms.
    /// \class SurfaceMeshSubdivision easy3d/algo/surface_mesh_subdivision.h
    class SurfaceMeshSubdivision {
    public:
        /** \brief The Catmull-Clark subdivision. */
        static bool catmull_clark(SurfaceMesh *mesh);

        /** \brief The Loop subdivision. */
        static bool loop(SurfaceMesh *mesh);

        /** \brief The sqrt3 subdivision. */
        static bool sqrt3(SurfaceMesh *mesh);
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_MESH_SUBDIVISION_H