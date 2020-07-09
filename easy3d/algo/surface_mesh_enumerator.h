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

#ifndef EASY3D_ALGO_SURFACE_MESH_ENUMERATOR_H
#define EASY3D_ALGO_SURFACE_MESH_ENUMERATOR_H


#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class SurfaceMeshEnumerator {
    public:

        /**
         * Propagates a connected component of a surface mesh from the seed vertex.
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
         * Propagates a connected component of a surface mesh from the seed face.
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
         * Enumerates the connected components of a surface mesh from its vertices.
         * @param mesh The input mesh.
         * @param id The vertex property storing the result.
         * @return The number of connected components.
         */
        static int enumerate_connected_components(SurfaceMesh *mesh, SurfaceMesh::VertexProperty<int> id);

        /**
         * Enumerates the connected components of a surface mesh from its faces.
         * @param mesh The input mesh.
         * @param id The face property storing the result.
         * @return The number of connected components.
         */
        static int enumerate_connected_components(SurfaceMesh *mesh, SurfaceMesh::FaceProperty<int> id);

    };

}   // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_ENUMERATOR_H

