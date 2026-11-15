/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_ALGO_SURFACE_MESH_POLYGONIZATION_H
#define EASY3D_ALGO_SURFACE_MESH_POLYGONIZATION_H

#include <set>
#include <vector>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class SurfaceMeshBuilder;

    /**
     * \brief Merge connected coplanar faces into a general polygon face.
     * \class SurfaceMeshPolygonization easy3d/algo/surface_mesh_polygonization.h
     * \details Every maximal edge-connected planar patch is merged into one polygon face. If a patch's boundary
     *      cannot be represented as a single polygon without holes (e.g., a patch with holes or a self-touching
     *      boundary), the patch is split into convex polygons. If that is not possible either, the patch's
     *      original faces are kept unchanged, so the operation never loses geometry. Degenerate faces (a
     *      zero-length edge or a zero-area polygon) are removed, because they enclose no area.
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
         * \attention The result doesn't contain polygon faces with holes. Patches with holes are decomposed into
         *      convex polygons or, if that fails, kept unchanged.
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
        std::vector<Loop> extract_boundary_loop(const SurfaceMesh *mesh,
                                                SurfaceMesh::FaceProperty<int> patch_id,
                                                int patch, std::set<SurfaceMesh::Halfedge>& boundary_edges) const;

        // classifies the boundary loops of a planar patch by their orientation with respect to the patch normal:
        // loops enclosing patch material are "outers" (CCW), loops enclosing holes are "holes" (CW). Returns false
        // if a loop is degenerate, so the patch cannot be classified reliably.
        bool classify(const SurfaceMesh *mesh, const std::vector<Loop>& loops, const vec3& normal,
                      std::vector<Loop>& outers, std::vector<Loop>& holes) const;

        // splits a planar patch (outer boundaries and holes) into convex polygons. The patch boundaries are first
        // normalized with the CSG operators (tessellator.h), because PolygonPartition requires simple contours
        // without self-intersections or overlaps. Returns polygons of builder vertices; vertices created by the
        // CSG operations are added to the builder.
        typedef std::vector<SurfaceMesh::Vertex> Contour;
        std::vector<Contour> split_complex_contour(
                const std::vector<Contour> &outer_polys,   // outer boundaries, each in counter-clockwise order
                const std::vector<Contour> &hole_polys,    // holes, each in clockwise order
                const vec3 &normal,                        // the normal of the polygon
                const SurfaceMesh *mesh,
                SurfaceMeshBuilder &builder,
                const std::vector<SurfaceMesh::Vertex> &vertex_map) const;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_POLYGONIZATION_H