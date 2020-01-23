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

#ifndef EASY3D_ALGORITHM_EXT_MESH_SURFACER_H
#define EASY3D_ALGORITHM_EXT_MESH_SURFACER_H


#include <vector>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {


    class SurfaceMesh;

    // This class simply collects the related algorithms implemented using CGAL
    // (to eliminate including CGAL header files from client code)

    class MeshSurfacer
    {
    public:
        MeshSurfacer();
        virtual ~MeshSurfacer();

        // ------- detect/remove duplicated faces

        // detect and return the duplicated faces
        // exact == true: do exact predict; otherwise use the distance threshold.
        // upon return, the second component of each entry contains the set of faces
        // duplicating the one stored as the first component.
        // TODO: implement using threshold (now only 'exact' is implemented).
        std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
        detect_duplicated_faces(SurfaceMesh* mesh, bool exact = false, double dist_threshold = 1e-6);

        // detect and remove duplicated faces. Returns the number of deleted faces.
        // exact == true: do exact predict; otherwise use the distance threshold.
        // TODO: implement using threshold (now only 'exact' is implemented).
        unsigned int remove_duplicated_faces(SurfaceMesh* mesh, bool exact = false, double dist_threshold = 1e-6);


        // ------- detect/remesh self intersections

        // detect and return the intersecting face pairs.
        std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
        detect_self_intersections(SurfaceMesh* mesh);

        // detect and remesh the intersecting faces
        SurfaceMesh* remesh_self_intersections(SurfaceMesh* mesh, bool stitch = true);
    };

}


#endif  // EASY3D_ALGORITHM_EXT_MESH_SURFACER_H

