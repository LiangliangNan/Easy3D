/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_ALGO_DELAUNAY_3D_H
#define EASY3D_ALGO_DELAUNAY_3D_H


#include <cassert>

#include <easy3d/algo/delaunay.h>


class tetgenio;


namespace easy3d {

    class VoronoiCell3d;

    /// \brief 3D Delaunay triangulation, using Hang Si's tetgen.
    /// \class Delaunay3 easy3d/algo/delaunay_3d.h
    /// \see Delaunay, Delaunay2.

    class Delaunay3 : public Delaunay {
    public:
        Delaunay3();

        ~Delaunay3() override;

        /// \brief Sets the vertices from an array of floating point numbers in which each consecutive number triple
        /// denotes a 3D point.
        void set_vertices(unsigned int nb_vertices, const float *vertices) override;

        /// \brief Sets the vertices from an array of 3D points.
        void set_vertices(const std::vector<vec3> &vertices) {
            // for QDel
            if (vertices.capacity() - vertices.size() < 8) {
                const_cast<std::vector<vec3> &>(vertices).reserve(vertices.size() + 8);
            }
            set_vertices((unsigned int) vertices.size(), &vertices[0].x);
        }

        /// \brief Returns the number of tetrahedra.
        unsigned int nb_tets() const { return nb_cells(); }

        const int *tet_to_v() const { return cell_to_v(); }

        const int *tet_to_tet() const { return cell_to_cell(); }

        int vertex_tet(int v) const { return vertex_cell(v); }

        unsigned int nearest_vertex(const float *p) const override {
            return Delaunay::nearest_vertex(p);
        }

        unsigned int nearest_vertex(const vec3 &p) const {
            return nearest_vertex(p.data());
        }

        const vec3 &vertex(unsigned int i) const {
            return *(const vec3 *) vertex_ptr(i);
        }

        /// \brief Returns the index of the \p lv_th vertex in the \p t_th tetrahedron.
        int tet_vertex(unsigned int t, unsigned int lv) const {
            return cell_vertex(t, lv);
        }

        int tet_adjacent(unsigned int t, unsigned int lf) const {
            return cell_adjacent(t, lf);
        }

        int tet_facet_vertex(unsigned int t, unsigned int lf, unsigned int lv) const {
            assert(lf < 4);
            assert(lv < 3);
            return tet_vertex(t, facet_vertex_[lf][lv]);
        }

        int next_around_halfedge(int t, unsigned int lv1, unsigned int lv2) const {
            assert(t < (int) nb_tets());
            assert(t >= 0);
            assert(lv1 < 4);
            assert(lv2 < 4);
            assert(lv1 != lv2);
            return tet_adjacent(t, next_around_halfedge_[lv1][lv2]);
        }

        int prev_around_halfedge(int t, unsigned int lv1, unsigned int lv2) const {
            return next_around_halfedge(t, lv2, lv1);
        }

        vec3 facet_normal(unsigned int t, unsigned int f) const {
            assert(t < nb_tets());
            assert(f < 4);
            const vec3 &p1 = vertex(tet_vertex(t, facet_vertex_[f][0]));
            const vec3 &p2 = vertex(tet_vertex(t, facet_vertex_[f][1]));
            const vec3 &p3 = vertex(tet_vertex(t, facet_vertex_[f][2]));
            vec3 result = cross(p2 - p1, p3 - p1);
            return result;
        }

        vec3 tet_circumcenter(unsigned int t) const {
            const vec3 &p0 = vertex(tet_vertex(t, 0));
            const vec3 &p1 = vertex(tet_vertex(t, 1));
            const vec3 &p2 = vertex(tet_vertex(t, 2));
            const vec3 &p3 = vertex(tet_vertex(t, 3));
            return geom::tetra_circum_center(p0, p1, p2, p3);
        }

        /// \brief Returns the Voronoi cell associated with vertex v.
        void get_voronoi_cell(
                unsigned int v, VoronoiCell3d &cell, bool geometry = true
        ) const;

    protected:
        void get_voronoi_facet(
                VoronoiCell3d &cell, unsigned int t,
                unsigned int lv1, unsigned int lv2, bool geometry
        ) const;

        static unsigned int other_in_face(
                unsigned int f, unsigned int lv1, unsigned int lv2
        ) {
            assert(f < 4);
            assert(lv1 < 4);
            assert(lv2 < 4);
            unsigned int ov1 = facet_vertex_[f][0];
            unsigned int ov2 = facet_vertex_[f][1];
            unsigned int ov3 = facet_vertex_[f][2];
            if (lv1 == ov1 && lv2 == ov2) { return ov3; }
            if (lv1 == ov2 && lv2 == ov1) { return ov3; }
            if (lv1 == ov1 && lv2 == ov3) { return ov2; }
            if (lv1 == ov3 && lv2 == ov1) { return ov2; }
            if (lv1 == ov2 && lv2 == ov3) { return ov1; }
            if (lv1 == ov3 && lv2 == ov2) { return ov1; }
            DCHECK(false) << "should not have reached here";
            return 4;
        }

    protected:
        static unsigned int next_around_halfedge_[4][4];
        static unsigned int facet_vertex_[4][3];

    protected:
        tetgenio *tetgen_out_;
        tetgenio *tetgen_in_;
    };

    //________________________________________________________________________________

    /**
     * \brief A data structure for 3D Voronoi cells
     * \class VoronoiCell3d easy3d/algo/delaunay_3d.h
     * \details A 3D Voronoi cell stores the dual facets in a Compressed Row Storage array.
     * - Each facet knows the bisector it is on, and the list of vertices/edges.
     *    - Each vertex knows the tet it is dual to.
     *    - Each edge knows the other bisector it is on (an edge is defined as the
     * intersection between the facet bisector and the edge bisector).
     */
    class VoronoiCell3d {
    public:
        VoronoiCell3d() { facet_ptr_.push_back(0); }

        void clear() {
            facet_ptr_.resize(0);
            facet_bisector_.resize(0);
            edge_bisector_.resize(0);
            vertex_.resize(0);
            infinite_.resize(0);
            facet_ptr_.push_back(0);
        }

        unsigned int nb_facets() const {
            return (unsigned int) facet_ptr_.size() - 1;
        }

        unsigned int facet_begin(unsigned int f) const {
            assert(f < nb_facets());
            return facet_ptr_[f];
        }

        unsigned int facet_end(unsigned int f) const {
            assert(f < nb_facets());
            return facet_ptr_[f + 1];
        }

        unsigned int nb_vertices(unsigned int f) const {
            assert(f < nb_facets());
            return facet_end(f) - facet_begin(f);
        }

        unsigned int next_around_facet(unsigned int f, unsigned int i) const {
            assert(i >= facet_begin(f) && i < facet_end(f));
            return (i + 1 == facet_end(f) ? facet_begin(f) : i + 1);
        }

        unsigned int prev_around_facet(unsigned int f, unsigned int i) const {
            assert(i >= facet_begin(f) && i < facet_end(f));
            return (i == facet_begin(f) ? facet_end(f) - 1 : i - 1);
        }

        /**
        * - This VoronoiCell3d is the dual of vertex v1,
        * - Facet f lies on the bisector plane of [v1,v2],
        * - facet_bisector(f) returns v2.
        */
        unsigned int facet_bisector(unsigned int f) const {
            assert(f < nb_facets());
            return facet_bisector_[f];
        }

        /**
        * -The vertices of facet(f) are denoted by indices i in the range
        *   [facet_begin(f) ... facet_end(f) - 1].
        * - vertex i is the dual of a tetrahedron t incident to the edge [v1,v2]
        * - edge_bisector(i) returns the vertex of t oriented clockwise
        *    relative to the edge [v1,v2].
        * - Note: an infinite facet has two vertices i1 and i2 that are
        *     dual to infinite tetrahedra.
        *   Supposing the proper ordering of i1 and i2, edge_bisector(i1)
        *   returns the vertex of the boundary facet that is incident to [v1,v2],
        *   and edge_bisector(i2) returns -1 (infinite vertex). (the other vertex
        *   of the other boundary facet that is incident to [v1,v2] is encountered
        *   when traversing the dual vertex/tetrahedron that precedes v2).
        */
        int edge_bisector(unsigned int i) const {
            assert(i < edge_bisector_.size());
            return edge_bisector_[i];
        }

        /**
        * - For finite dual vertices, returns the Voronoi vertex.
        * - For dual vertices at infinity, returns the direction
        *    (normal to primal facet on the boundary).
        */
        const vec3 &vertex(unsigned int i) const {
            assert(i < vertex_.size());
            return vertex_[i];
        }

        bool vertex_is_infinite(unsigned int i) const {
            assert(i < infinite_.size());
            return infinite_[i];
        }

        void begin_facet(unsigned int f_bisector) {
            facet_bisector_.push_back(f_bisector);
        }

        void add_to_facet(
                int e_bisector, const vec3 &v, bool infinite
        ) {
            edge_bisector_.push_back(e_bisector);
            vertex_.push_back(v);
            infinite_.push_back(infinite);
        }

        void add_to_facet(
                int e_bisector, bool infinite
        ) {
            edge_bisector_.push_back(e_bisector);
            infinite_.push_back(infinite);
        }

        void end_facet() {
            facet_ptr_.push_back((unsigned int) edge_bisector_.size());
        }

        unsigned int find_facet(unsigned int bisector) {
            for (unsigned int i = 0; i < facet_bisector_.size(); i++) {
                if (facet_bisector_[i] == bisector) {
                    return i;
                }
            }

            // unexpected thing has happened, let's print some information for debugging
            std::cerr << "bisector = " << bisector;
            std::cerr << " facet = [";
            for (auto fb : facet_bisector_) {
                std::cerr << fb << " ";
            }
            std::cerr << "]" << std::endl;
            DCHECK(false) << "should not have reached here";
            return 0;
        }

    private:
        std::vector<unsigned int> facet_ptr_;
        std::vector<unsigned int> facet_bisector_;
        std::vector<int> edge_bisector_;
        std::vector<vec3> vertex_;
        std::vector<bool> infinite_;
    };

/*
 * The commented one is enough for basic 3D Delaunay implementation.
 * The above one is verbose for easy understanding of the interface 
 * APIs and accessing Voronoi cells.
 */

//class MATH_API Delaunay3 : public Delaunay
//{
//public:
//	Delaunay3() ;
//	virtual ~Delaunay3() ;
//	virtual void set_vertices(unsigned int nb_vertices, const double* vertices) ;
//
//protected:
//	tetgenio* tetgen_out_ ;
//	tetgenio* tetgen_in_ ;
//} ;

}   // namespace easy3d


#endif  // EASY3D_ALGO_DELAUNAY_3D_H
