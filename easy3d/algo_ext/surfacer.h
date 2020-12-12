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

#ifndef EASY3D_ALGO_SURFACER_H
#define EASY3D_ALGO_SURFACER_H


#include <vector>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {


    class SurfaceMesh;

    /**
     * \brief A collection of mesh (and polygon soup) processing functions.
     *
     * \class Surfacer  easy3d/algo_ext/surfacer.h
     *
     * \details This class collects some related algorithms implemented using CGAL.
     *          It allows (re)orientation, detecting and resolving topological issues (e.g., duplicate vertices/faces,
     *          self intersection), and clipping/splitting/slicing of a surface mesh.
     *
     * \see DuplicateFaces and SelfIntersection.
     */
    class Surfacer {
    public:
        /// A polygon represented by a list of vertex indices
        typedef std::vector<int> Polygon;

        /// A polyline represented by a list of points.
        /// If a polyline is closed, its first and last points will be identical.
        typedef std::vector<vec3> Polyline;

    public:

        /// \name Orientation and stitching
        //@{

        /**
         * \brief Stitches connected components of a surface mesh
         * \details It first reverses the connected components having compatible boundary cycles that could be merged
         * if their orientation were made compatible. Then, it stitches those with compatible boundaries.
         *
         * Connected components are examined by increasing number of faces.
         *
         * \see merge_reversible_connected_components_2()
         */
        static void merge_reversible_connected_components(SurfaceMesh *mesh);

        /**
         * \brief Stitches connected components of a surface mesh
         * \details This function has the same goal as merge_reversible_connected_components(). The difference is that
         * it treats the input mesh as a polygon soup. Internally, it calls
         * orient_polygon_soup(std::vector<vec3>& points, std::vector<Polygon>& polygons).
         *
         * \see merge_reversible_connected_components()
         */
        static bool merge_reversible_connected_components_2(SurfaceMesh *mesh);

        /**
         * \brief Reverses the orientation of the entire mesh
         */
        static void reverse_orientation(SurfaceMesh *mesh);

        /**
         * \brief Makes each connected component of a closed triangle surface mesh inward or outward oriented.
         * \pre mesh.is_triangle_mesh(), mesh.is_closed()
         * @param mesh The input mesh.
         */
        static void orient(SurfaceMesh *mesh);

        /**
         * \brief Tries to consistently orient a polygon soup.
         * \details When it is not possible to produce a combinatorial manifold surface, some points are duplicated.
         * It also builds a polygon mesh if the oriented soup of polygons describes a consistently oriented polygon
         * mesh. The algorithm is described in
         *   - A.Guéziec, et al. Cutting and stitching: Converting sets of polygons to manifold surfaces. TVCG 2001.
         *
         * \param points Points of the soup of polygons. Some additional points might be pushed back to resolve
         *        non-manifoldness or non-orientability issues.
         * \param polygons Each element in the vector describes a polygon using the index of the points in points.
         *        If needed the order of the indices of a polygon might be reversed.
         * \return \c true if the orientation operation succeeded. \c false if some points were duplicated, thus
         *         producing a self-intersecting polyhedron.
         */
        static bool orient_polygon_soup(std::vector<vec3> &points, std::vector<Polygon> &polygons);

        /**
         * \brief Repairs a given polygon soup through various repairing operations.
         * \details This function carries out the following tasks, in the same order as they are listed:
         *  - merging of duplicate points, using CGAL::Polygon_mesh_processing::merge_duplicate_points_in_polygon_soup();
         *  - simplification of polygons to remove geometrically identical consecutive vertices;
         *  - splitting of "pinched" polygons, that is polygons in which a geometric position appears more than once.
         *    The splitting process results in multiple non-pinched polygons;
         *  - removal of invalid polygons, that is polygons with fewer than 2 vertices;
         *  - removal of duplicate polygons, using Polygon_mesh_processing::merge_duplicate_polygons_in_polygon_soup();
         *  - removal of isolated points, using Polygon_mesh_processing::remove_isolated_points_in_polygon_soup().
         * \note The point and polygon containers will be modified by the repairing operations, and thus the indexation
         * of the polygons will also be changed.
         */
        static void repair_polygon_soup(std::vector<vec3> &points, std::vector<Polygon> &polygons);

        /**
         * \brief Repairs a given polygon mesh through various repairing operations.
         * \details This function carries out the following tasks, in the same order as they are listed:
         *  - merging of duplicate points;
         *  - simplification of faces to remove geometrically identical consecutive vertices;
         *  - splitting of "pinched" faces, that is face in which a geometric position appears more than once.
         *    The splitting process results in multiple non-pinched faces;
         *  - removal of invalid faces, that is faces with fewer than 2 vertices;
         *  - removal of duplicate faces;
         *  - removal of isolated points.
         * This function treats the input mesh as a polygon soup. Internally, it calls
         * clean_polygon_soup(std::vector<vec3>& points, std::vector<Polygon>& polygons).
         * \note The point and face containers will be modified by the repairing operations, and thus the indexation
         * of the polygons will also be changed.
         */
        static void repair_polygon_mesh(SurfaceMesh *mesh);


        /**
         * \brief Stitches together border halfedges in a polygon mesh.
         * \details The pairs of halfedges to be stitched are automatically found amongst all border halfedges.
         * Two border halfedges h1 and h2 can be stitched if the points associated to the source and target vertices
         * of h1 are the same as those of the target and source vertices of h2 respectively.
         * @return The number of pairs of halfedges that were stitched.
         */
        static int stitch_borders(SurfaceMesh *pmesh);
        //@}


        /// \name Duplicate faces
        //@{

        /// \brief Detects duplicate faces.
        /// @param exact True: do exact predict; otherwise use the distance threshold.
        /// \return The set of duplicate faces, where the \c second of each entry contains the set of faces
        /// duplicating the \c first.
        static std::vector<std::pair<SurfaceMesh::Face, std::vector<SurfaceMesh::Face> > >
        detect_duplicate_faces(SurfaceMesh *mesh, bool exact = false, double dist_threshold = 1e-6);

        /// \brief Detects and removes duplicate faces.
        /// @param exact \c true to do exact predict; otherwise use the distance threshold.
        /// \return The number of faces that has been deleted.
        static unsigned int remove_duplicate_faces(SurfaceMesh *mesh, bool exact = false, double dist_threshold = 1e-6);
        //@}


        /// \name Self intersections
        //@{

        /**
         * \brief Collects all pairs of intersecting faces of a triangule surface mesh.
         * \details Two faces are said to intersect if the corresponding triangles intersect and the intersection is
         * not an edge nor a vertex incident to both faces.
         * \pre mesh.is_triangle_mesh().
         * \param mesh The triangle surface mesh to be checked.
         * \return All pairs of non-adjacent faces that intersect.
         */
        static std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
        detect_self_intersections(SurfaceMesh *mesh);

        /**
         * \brief Detects and remesh the intersecting faces.
         * \pre mesh.is_triangle_mesh().
         * @param mesh The input mesh. If self intersection exists, it carries the remeshed model. Otherwise it remains
         *             unchanged.
         * @param stitch Stitch the borders
         * @return \c true if remesh actually occurred (i.e., self intersection was detected).
         */
        static bool remesh_self_intersections(SurfaceMesh *mesh, bool stitch = true);
        //@}



        /// \name Clip, split, and slice
        //@{

        /**
         * \brief Clips a triangle mesh by keeping the part on the negative side of a plane (side opposite to its
         *       normal vector).
         * \details If \p mesh is closed, the clipped part can be closed too if the named parameter \p clip_volume
         *          is set to \c true.
         * \pre mesh.is_triangle_mesh(), !does_self_intersect(SurfaceMesh* mesh).
         * @param mesh The input triangle mesh.
         * @param plane The clipping plane whose negative side defines the half-space to intersect \p mesh with.
         *
         * @return \c true if the output surface mesh is manifold. If \c false is returned \par mesh is only refined
         *         by the intersection with plane.
         */
        static bool clip(SurfaceMesh *mesh, const Plane3 &plane, bool clip_volume = false);

        /**
         * \brief Split a triangle mesh by a plane.
         * \details It adds intersection edges of \p mesh and \p plane in \p mesh and duplicates those edges.
         * \pre mesh.is_triangle_mesh(), !does_self_intersect(SurfaceMesh* mesh).
         * @param mesh The input triangle mesh.
         * @param plane The plane that will be used to split \p mesh.
         */
        static void split(SurfaceMesh *mesh, const Plane3 &plane);

        /**
         * \brief Computes the intersection of a plane with a triangle surface mesh.
         * \pre mesh.is_triangle_mesh(), !does_self_intersect(SurfaceMesh* mesh).
         * @param mesh The input triangle mesh.
         * @param plane The plane to intersect the triangle surface mesh with.
         * @return The intersecting polylines. Each resulting polyline \p P is oriented such that for two consecutive
         *         points \p p and \p q in \p P, the normal vector of the face(s) containing the segment pq,
         *         the vector \p pq, and the normal of \p plane is a direct orthogonal basis. The normal vector of
         *         each face is chosen to point on the side of the face where its sequence of vertices is seen
         *         counterclockwise.
         * \note An edge shared by two faces included in plane will not be reported. For example, if plane passes
         *       though one face of a cube, only one closed polyline will be reported (the boundary of the face).
         * \see slice(SurfaceMesh *mesh, const std::vector<Plane3> &planes).
         */
        static std::vector<Polyline> slice(SurfaceMesh *mesh, const Plane3 &plane);

        /**
         * \brief Computes the intersection of a set of planes with a triangle surface mesh.
         * \pre mesh.is_triangle_mesh(), !does_self_intersect(SurfaceMesh* mesh).
         * @param mesh The input triangle mesh.
         * @param planes The set of planes to intersect the triangle surface mesh with.
         * @return The intersecting polylines by all the planes. The \c i_th entry denotes the polylines created by
         *         the \c i_th plane. Each resulting polyline \p P is oriented such that for two consecutive
         *         points \p p and \p q in \p P, the normal vector of the face(s) containing the segment pq,
         *         the vector \p pq, and the normal of \p plane is a direct orthogonal basis. The normal vector of
         *         each face is chosen to point on the side of the face where its sequence of vertices is seen
         *         counterclockwise.
         * \note An edge shared by two faces included in plane will not be reported. For example, if plane passes
         *       though one face of a cube, only one closed polyline will be reported (the boundary of the face).
         * \see slice(SurfaceMesh *mesh, const Plane3 &plane).
         */
        static std::vector< std::vector<Polyline> > slice(SurfaceMesh *mesh, const std::vector<Plane3> &planes);
        //@}
    };
}


#endif  // EASY3D_ALGO_SURFACER_H

