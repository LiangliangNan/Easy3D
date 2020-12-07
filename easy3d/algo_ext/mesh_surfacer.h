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

#ifndef EASY3D_ALGO_MESH_SURFACER_H
#define EASY3D_ALGO_MESH_SURFACER_H


#include <vector>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {


    class SurfaceMesh;

    /// \brief Detect/Resolve topological issues (e.g., duplicate faces, self intersection) of a surface mesh.
    /// \class MeshSurfacer  easy3d/algo_ext/mesh_surfacer.h
    /// \details This class simply collects the related algorithms implemented using CGAL (to eliminate including CGAL
    /// header files from client code). \see DuplicatedFaces and SelfIntersection.
    class MeshSurfacer
    {
    public:
        MeshSurfacer();
        virtual ~MeshSurfacer();

        // ------- detect/remove duplicated faces

        /// \brief Detect and return the duplicated faces.
        /// @param exact True: do exact predict; otherwise use the distance threshold.
        /// Upon return, the second component of each entry contains the set of faces duplicating the one stored as
        /// the first component.
        std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
        detect_duplicated_faces(SurfaceMesh* mesh, bool exact = false, double dist_threshold = 1e-6);

        /// \brief Detect and remove duplicated faces. Returns the number of deleted faces.
        /// @param exact True: do exact predict; otherwise use the distance threshold.
        unsigned int remove_duplicated_faces(SurfaceMesh* mesh, bool exact = false, double dist_threshold = 1e-6);


        // ------- detect/remesh self intersections

        /**
         * \brief Detect intersecting face pairs.
         * @param mesh The input mesh.
         * @param construct If true, also construct the intersecting geometry.
         * @return The intersecting face pairs.
         */
        std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
        detect_self_intersections(SurfaceMesh* mesh);

        /**
         * \brief Detect and remesh the intersecting faces.
         * @param mesh The input mesh. If self intersection exists, it carries the remeshed model. Otherwise it remains
         *             unchanged.
         * @param stitch Stitch the borders
         * @return true if remesh actually occurred (i.e., self intersection was detected).
         */
        bool remesh_self_intersections(SurfaceMesh* mesh, bool stitch = true);
    };

}


#endif  // EASY3D_ALGO_MESH_SURFACER_H

