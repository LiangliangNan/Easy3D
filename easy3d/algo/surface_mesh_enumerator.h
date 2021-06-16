/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_ENUMERATOR_H
#define EASY3D_ALGO_SURFACE_MESH_ENUMERATOR_H


#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * \brief Enumerates connected components for a surface mesh.
     * \class SurfaceMeshEnumerator easy3d/algo/surface_mesh_enumerator.h
     */
    class SurfaceMeshEnumerator {
    public:

        /**
         * \brief Enumerates the connected components of a surface mesh from its vertices.
         * @param mesh The input mesh.
         * @param id The vertex property storing the result.
         * @return The number of connected components.
         */
        static int enumerate_connected_components(SurfaceMesh *mesh, SurfaceMesh::VertexProperty<int> id);

        /**
         * \brief Enumerates the connected components of a surface mesh from its faces.
         * @param mesh The input mesh.
         * @param id The face property storing the result.
         * @return The number of connected components.
         */
        static int enumerate_connected_components(SurfaceMesh *mesh, SurfaceMesh::FaceProperty<int> id);

        /**
         * \brief Propagates a connected component of a surface mesh from the seed vertex.
         * @param mesh The input mesh.
         * @param id The vertex property storing the result.
         * @param seed The seed vertex from which the connected component will be propagated.
         * @param cur_id The index of the connected component to be propagated.
         * @attention The vertex property id must be initialize with "-1".
         */
        static void
        propagate_connected_component(SurfaceMesh *mesh, SurfaceMesh::VertexProperty<int> id, SurfaceMesh::Vertex seed,
                                      int cur_id);

        /**
         * \brief Propagates a connected component of a surface mesh from the seed face.
         * @param mesh The input mesh.
         * @param id The face property storing the result.
         * @param seed The seed face from which the connected component will be propagated.
         * @param cur_id The index of the connected component to be propagated.
         * @attention The face property id must be initialize with "-1".
         */
        static void
        propagate_connected_component(SurfaceMesh *mesh, SurfaceMesh::FaceProperty<int> id, SurfaceMesh::Face seed,
                                      int cur_id);

        /**
         * \brief Enumerates planar patches.
         * @param mesh The input mesh.
         * @param id The vertex property storing the result.
         * \param angle_threshold Two faces sharing a common edge are considered coplanar if the dihedral angle is
         *      smaller than \p angle_threshold (in degrees).
         * @return The number of connected components.
         */
        static int enumerate_planar_components(
                SurfaceMesh *mesh,
                SurfaceMesh::FaceProperty<int> id,
                float angle_threshold = 1.0f
        );

        /**
         * \brief Propagates a planar patch from the seed face.
         * @param mesh The input mesh.
         * @param id The face property storing the result.
         * @param seed The seed face from which the planar patch will be propagated.
         * @param cur_id The index of the connected component to be propagated.
         * \param angle_threshold Two faces sharing a common edge are considered coplanar if the dihedral angle is
         *      smaller than \p angle_threshold (in degrees).
         * \pre The face normal property (with name \c "f:normal") must exist and be up to date.
         * @attention The face property id must be initialize with "-1".
         */
        static void propagate_planar_component(SurfaceMesh *mesh,
                                               SurfaceMesh::FaceProperty<int> id,
                                               SurfaceMesh::Face seed, int cur_id,
                                               float angle_threshold
        );

    };

}   // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_ENUMERATOR_H

