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

#include <easy3d/algo_ext/duplicated_faces.h>
#include <easy3d/algo_ext/self_intersection.h>

#include <easy3d/algo_ext/mesh_surfacer.h>

#include <easy3d/util/logging.h>


namespace easy3d {


    MeshSurfacer::MeshSurfacer()
    {
    }


    MeshSurfacer::~MeshSurfacer() {
    }


    std::vector< std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
    MeshSurfacer::detect_duplicated_faces(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        easy3d::DuplicatedFaces rdf;
        return rdf.detect(mesh, exact, dist_threshold);
    }


    unsigned int MeshSurfacer::remove_duplicated_faces(SurfaceMesh* mesh, bool exact, double dist_threshold)
    {
        DuplicatedFaces rdf;
        return rdf.remove(mesh, exact, dist_threshold);
    }


    std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
    MeshSurfacer::detect_self_intersections(SurfaceMesh* mesh)
    {
        SelfIntersection msi;
        return msi.detect(mesh, false);
    }


    SurfaceMesh* MeshSurfacer::remesh_self_intersections(SurfaceMesh* mesh, bool stitch /* = true */)
    {
        SelfIntersection msi;
        return msi.remesh(mesh, stitch);
    }

}
