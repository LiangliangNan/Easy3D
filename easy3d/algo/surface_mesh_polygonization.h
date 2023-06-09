/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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

#ifndef EASY3D_ALGO_SURFACE_MESH_POLYGONIZATION_H
#define EASY3D_ALGO_SURFACE_MESH_POLYGONIZATION_H

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * \brief Merge connected coplanar faces into a general polygon face.
     * \class SurfaceMeshPolygonization easy3d/algo/surface_mesh_polygonization.h
     * \todo Support faces with holes (i.e., polygons with multiple contours)
     */
    class SurfaceMeshPolygonization {
    public:

        /**
         * \brief Merges connected coplanar faces into a general polygon face.
         * \details The result is a general polygonal mesh.
         * \param mesh The input surface mesh. Upon return, the mesh will be modified.
         * \param angle_threshold Two faces sharing a common edge are considered coplanar if the dihedral angle is
         *      smaller than \p angle_threshold (in degrees).
         * \attention The current implementation doesn't support polygon faces with holes.
         */
        void apply(SurfaceMesh *mesh, float angle_threshold = 1.0f);

        /**
         * \brief Removes 2-degree vertices.
         * \details For every 2-degree vertex, if the angle between its two incident edges is smaller than a threshold,
         * merge the two incident edges by removing this vertex.
         * \param mesh The input surface mesh. Upon return, the mesh will be modified.
         * \param angle_threshold Two edges sharing the same vertex are considered colinear if their angle is smaller
         *      than \p angle_threshold (in degrees).
         */
        void merge_colinear_edges(SurfaceMesh *mesh, float angle_threshold = 1.0f);

    private:
        void internal_apply(SurfaceMesh *mesh, float angle_threshold);

        typedef std::vector<SurfaceMesh::Halfedge> Loop;
        std::vector<Loop> extract_boundary_loop(const SurfaceMesh *mesh, int comp_id, std::set<SurfaceMesh::Halfedge>& boundary_edges);

        // classify the loops of a planar region into an "outer" loop and several "holes".
        void classify(const SurfaceMesh *mesh, const std::vector<Loop>& loops, Loop& outer, std::vector<Loop>& holes);

        // split a complex polygon (with duplicate vertices and possibly hole) into a set of convex polygons
        typedef std::vector<SurfaceMesh::Vertex> Contour;
        std::vector<Contour> split_complex_contour(
                const Contour &outer_poly,              // the outer polygon represented by a list of SurfaceMesh::Vertex
                const std::vector<Contour> &hole_polys, // the holes each represented by a list of SurfaceMesh::Vertex
                const vec3 &normal,                     // the normal of the polygon
                const SurfaceMesh *mesh
        ) const;

    private:
        SurfaceMesh::FaceProperty<int> planar_segments_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_POLYGONIZATION_H