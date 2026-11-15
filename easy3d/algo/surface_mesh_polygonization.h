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
     * \brief Merge connected coplanar faces into convex polygon faces.
     * \class SurfaceMeshPolygonization easy3d/algo/surface_mesh_polygonization.h
     * \details Faces are grouped into maximal edge-connected patches: two faces sharing an edge belong to the
     *      same patch if the angle between their normals is smaller than the threshold passed to apply() (in
     *      degrees). The test compares neighboring faces only, so the normals inside a large patch may drift by
     *      more than the threshold. There is no global planarity test: with a large threshold, a merged face
     *      spans its boundary and therefore only approximates the original geometry. Border edges and edges
     *      between different patches delimit the patch boundary.
     *
     *      Each patch is replaced by convex polygons whenever possible:
     *      - If the patch has no holes and every boundary loop is simple (does not visit a vertex twice), each
     *        outer boundary loop is partitioned into convex polygons with PolygonPartition (Hertel-Mehlhorn);
     *        an already convex loop becomes a single face.
     *      - If the patch has holes or a self-touching boundary, its boundary is first normalized by the CSG
     *        operators (easy3d/algo/tessellator.h) and then partitioned into convex polygons with
     *        PolygonPartition (Hertel-Mehlhorn). The holes remain open, and no face in the result has holes.
     *        Vertices created where boundary segments intersect are added to the mesh.
     *      - If the boundary cannot be extracted or classified, or the partition fails, the patch's original
     *        faces are kept instead of being dropped (input faces are not necessarily convex). Zero-area parts
     *        returned by the partition are skipped.
     *
     *      Degenerate faces reported by SurfaceMesh::is_degenerate() (triangles with a zero-length edge or a zero
     *      angle, and polygons without any valid triple) are removed, because they enclose no area.
     *      The mesh is rebuilt in place, which discards every property except the vertex positions (see apply()).
     *      Borders of the input stay borders; they are not stitched.
     */
    class SurfaceMeshPolygonization {
    public:

        /**
         * \brief Merges connected coplanar faces into convex polygon faces.
         * \details All faces created by merging are convex polygons. See the class documentation for how patches
         *      with holes, with a self-touching boundary, with a non-convex boundary, or that cannot be merged are
         *      handled. The operation is repeated until the face count no longer decreases, and every pass ends by
         *      calling merge_colinear_edges() with the same \p angle_threshold.
         * \param mesh The input surface mesh, modified in place.
         * \param angle_threshold Two faces sharing a common edge are considered coplanar if the angle between
         *      their normals is smaller than \p angle_threshold (in degrees).
         * \attention The mesh is rebuilt: only the vertex positions survive. All other vertex, edge, face and
         *      model properties (colors, normal vectors, labels, etc.) are discarded, and isolated vertices,
         *      e.g., the interior vertices of a merged patch, are removed. Vertices may be duplicated to
         *      resolve non-manifold configurations. Normal vectors are not recomputed; call
         *      update_vertex_normals()/update_face_normals() if needed.
         * \attention The result doesn't contain polygon faces with holes. Patches with holes are decomposed into
         *      convex polygons (the holes stay open) or, if that fails, kept as their original faces.
         */
        void apply(SurfaceMesh *mesh, float angle_threshold = 1.0f);

        /**
         * \brief Removes valence-2 vertices whose incident edges are nearly collinear.
         * \details If the two halfedges incident to a valence-2 vertex (typically a boundary vertex) deviate from
         *      a straight line by less than \p angle_threshold, the vertex is removed and its two edges are
         *      joined. A vertex is kept if removing it would collapse an incident face to zero area or leave an
         *      incident face with fewer than three vertices. This function modifies the mesh in place and keeps
         *      all properties; deleted elements are collected at the end.
         * \param mesh The input surface mesh, modified in place.
         * \param angle_threshold Two halfedges sharing a vertex are considered collinear if their angle is smaller
         *      than \p angle_threshold (in degrees). The angle is measured between the halfedge leaving the vertex
         *      and the halfedge entering it, i.e., it is the turn angle, so a straight vertex has an angle of 0
         *      degrees.
         * \sa apply().
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