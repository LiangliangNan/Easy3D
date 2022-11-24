/********************************************************************
 * Copyright (C) 2018-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2015 Alec Jacobson <alecjacobson@gmail.com>
 *
 * The code in this file is partly from libigl (version 3 Mar 2016) with
 * modifications and enhancement:
 *      https://libigl.github.io/
 * The original code was distributed under the Mozilla Public License v2.
 ********************************************************************/

#ifndef EASY3D_ALGO_EXT_SELF_INTERSECTION_H
#define EASY3D_ALGO_EXT_SELF_INTERSECTION_H

#include <vector>
#include <unordered_map>

// Use this instead to mute errors resulting from bad CGAL assertions
//#define CGAL_KERNEL_NO_ASSERTIONS

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/intersections.h>    // for triangle-triangle intersection
#include <CGAL/box_intersection_d.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_plus_2.h>

#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    /// \brief Detects and resolves self-intersection for surface mesh.
    /// \details Given a triangle mesh compute a new mesh which is the same
    /// as the input mesh except that any self-intersecting triangles
    /// have been subdivided (new vertices and face created) so that
    /// the self-intersection contour lies exactly on edges in the
    /// the new mesh. New vertices will appear in original faces or
    /// on original edges. New vertices on edges are "merged" only
    /// across original faces sharing that edge. This means that if
    /// the input triangle mesh is a closed manifold the output will
    /// be too.
    ///
    /// Known issues:
    /// If an existing edge lies exactly on another face then any
    /// resulting additional vertices along that edge may not get
    /// properly connected so that the output mesh has the same
    /// global topology.
    ///
    /// \class SelfIntersection  easy3d/algo_ext/self_intersection.h

    class SelfIntersection {
    public:
        SelfIntersection();
        ~SelfIntersection();

        /**
         * \brief Detect intersecting face pairs.
         * @param mesh The input mesh.
         * @param construct If true, also construct the intersecting geometry.
         * @return The intersecting face pairs.
         */
        std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> >
        detect(SurfaceMesh *mesh, bool construct = false);

        /**
         * \brief Detect and remesh the intersecting faces.
         * @param mesh The input mesh. If self intersection exists, it carries the remeshed model. Otherwise it remains
         *             unchanged.
         * @param stitch Stitch the borders
         * @return \c true if remesh actually occurred (i.e., self intersection was detected).
         */
        bool remesh(SurfaceMesh *mesh, bool stitch);

    private:

        typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;

        // 3D Primitives
        typedef CGAL::Point_3<Kernel> Point_3;
        typedef CGAL::Vector_3<Kernel>	Vector_3;
        typedef CGAL::Triangle_3<Kernel> Triangle_3;
        typedef CGAL::Segment_3<Kernel> Segment_3;
        typedef CGAL::Plane_3<Kernel> Plane_3;

        // Constrained Delaunay Triangulation
        typedef CGAL::Triangulation_vertex_base_2<Kernel> TVB_2;
        typedef CGAL::Constrained_triangulation_face_base_2<Kernel> CTFB_2;
        typedef CGAL::Triangulation_data_structure_2<TVB_2, CTFB_2> TDS_2;
        typedef CGAL::Exact_intersections_tag Itag;
        typedef CGAL::Constrained_Delaunay_triangulation_2<Kernel, TDS_2, Itag> CDT_2;
        typedef CGAL::Constrained_triangulation_plus_2<CDT_2> CDT_plus_2;

        typedef std::vector<std::pair<int, CGAL::Object> > ObjectList;

        struct Triangle {
        public:
            Triangle(const Point_3 &a, const Point_3 &b, const Point_3 &c, SurfaceMesh::Face f) : triangle(a, b, c),
                                                                                                  face(f) {}

            Triangle_3 triangle;
            SurfaceMesh::Face face;
            int index;    // face index
            std::vector<SurfaceMesh::Vertex> vertices;
        };

        // Axis-align boxes for all-pairs self-intersection detection
        typedef std::vector<Triangle> Triangles;
        typedef typename Triangles::iterator TrianglesIterator;
        typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, TrianglesIterator> Box;

    private:

        void mesh_to_cgal_triangle_list(SurfaceMesh *mesh);

        // test if two triangles intersect
        bool do_intersect(const Triangle &A, const Triangle &B);

        // Given a list of objects (e.g., resulting from intersecting a triangle
        // with many other triangles), construct a constrained Delaunay
        // triangulation on a given plane (P), by inserting constraints for each
        // object projected onto that plane.
        // Inputs:
        //   objects  list of objects. This should lie on the given plane (P),
        //     otherwise they are added to the cdt _after_ their non-trivial
        //     projection
        //   P  plane upon which all objects lie and upon which the CDT is
        //     conducted
        // Outputs:
        //   vertices  list of vertices of the CDT mesh _back on the 3D plane_
        //   faces  list of triangle indices into vertices
        //
        void projected_cdt(
                const std::vector<CGAL::Object> &objects,
                const Plane_3 &P,
                std::vector<Point_3> &vertices,
                std::vector<std::vector<int> > &faces
        );

        // Given a current 2D constrained Delaunay triangulation (cdt), insert a
        // 3D "object" (e.g., resulting from intersecting two triangles) into the
        // cdt, by projecting it via the given plane (P) and adding appropriate
        // constraints.
        // Inputs:
        //   obj  CGAL::Object representing a vertex, segment, or (convex)
        //     polygon. All vertices should lie on the plane P. If not, then this
        //     adds the _projection_ of this object to the cdt and that might not
        //     be what you wanted to do.
        //   P  plane obj lies on and upon which the cdt is being performed
        //   cdt  current CDT, see output
        // Outputs:
        //   cdt  CDT updated to contain constraints for the given object
        //
        void insert_into_cdt(const CGAL::Object &obj, const Plane_3 &P, CDT_plus_2 &cdt);

    private:
        // Helper function to mark a face as offensive
        // Inputs:
        //   f  index of face in F
        inline void mark_offensive(int f);

        // Helper function to count intersections between faces
        inline void count_intersection(int fa, int fb);

        // Helper function for box_intersect. Intersect two triangles A and B,
        // append the intersection object (point,segment,triangle) to a running
        // list for A and B
        // Inputs:
        //   A  triangle in 3D
        //   B  triangle in 3D
        // Returns true only if A intersects B
        inline bool intersect(const Triangle &A, const Triangle &B);

        // Helper function for box_intersect. In the case where A and B have
        // already been identified to share a vertex, then we only want to
        // add possible segment intersections. Assumes truly duplicate
        // triangles are not given as input
        // Inputs:
        //   A  triangle in 3D
        //   B  triangle in 3D
        //   va  shared vertex in A (and key into offending)
        //   vb  shared vertex in B (and key into offending)
        //   Returns true if intersection (besides shared point)
        inline bool single_shared_vertex(const Triangle &A, const Triangle &B, int va, int vb);

        // Helper handling one direction
        inline bool single_shared_vertex(const Triangle &A, const Triangle &B, int va);

        // Helper function for box_intersect. In the case where A and B have
        // already been identified to share two vertices, then we only want
        // to add a possible coplanar (Triangle) intersection. Assumes truly
        // degenerate facets are not given as input.
        inline bool double_shared_vertex(
                const Triangle &A,
                const Triangle &B,
                const std::vector<std::pair<int, int> > &shared
        );

    private:
        SurfaceMesh* mesh_;

        bool construct_intersection_;

        Triangles triangle_faces_;

        // index in 'triangle_faces_' (degenerate faces removed) -> original face
        std::unordered_map<int, SurfaceMesh::Face> original_face;

        // list of faces with intersections mapping to the order in
        // which they were first found
        std::unordered_map<int, ObjectList> offending_;

        // statistics on duplicate faces (to offer the user some feedback)
        int total_comb_duplicate_face_;    // Combinatorial duplicate faces
        int total_geom_duplicate_face_;    // Geometrically duplicate faces
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_EXT_SELF_INTERSECTION_H
