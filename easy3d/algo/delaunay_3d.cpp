/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#include <easy3d/algo/delaunay_3d.h>
#include <easy3d/util/stop_watch.h>

#include <3rd_party/tetgen/tetgen.h>


namespace easy3d {


    Delaunay3::Delaunay3() : Delaunay(3) {
        tetgen_in_ = new tetgenio;
        tetgen_out_ = new tetgenio;

        tetgen_in_->firstnumber = 0;
        tetgen_out_->firstnumber = 0;
    }

    Delaunay3::~Delaunay3() {
        delete tetgen_in_;
        delete tetgen_out_;
    }

    void Delaunay3::set_vertices(unsigned int nb_vertices, const float *vertices) {
        Delaunay::set_vertices(nb_vertices, vertices);
        tetgen_out_->clean_memory();
        tetgen_in_->numberofpoints = static_cast<int>(nb_vertices);

        // Liangliang: tetgen uses double by default.
        tetgen_in_->pointlist = new double[tetgen_in_->numberofpoints * 3];
        for (unsigned int i = 0; i < nb_vertices * 3; ++i) {
            tetgen_in_->pointlist[i] = vertices[i];
        }

        LOG(INFO) << "Delaunay triangulation...";
        StopWatch t;

        try {
            tetgenbehavior tetgen_args_;
            // Q: quiet
            // n: output tet neighbors
            // V: verbose
            tetgen_args_.parse_commandline((char *) ("Qn"));
            ::tetrahedralize(&tetgen_args_, tetgen_in_, tetgen_out_);
        } catch (const std::exception& e) {
            LOG(ERROR) << "encountered a problem: " << e.what();
        }

        set_arrays(
                tetgen_out_->numberoftetrahedra,
                tetgen_out_->tetrahedronlist, tetgen_out_->neighborlist
        );

        LOG(INFO) << "done. time: " << t.time_string();
    }






//________________________________________________________________________________

// Pre-computed table for turning around halfedges in
// a tetrahedron. Given a cell c and a halfedge (v1, v2),
// c->neighbor(next_around_halfedge[v1][v2]) is the cell
// adjacent to c on the left of (v1,v2).
// Diagonal entries are not supposed to be accessed.
    unsigned int Delaunay3::next_around_halfedge_[4][4] = {
            {5, 3, 1, 2},
            {2, 5, 3, 0},
            {3, 0, 5, 1},
            {1, 2, 0, 5}
    };

    // tet facet vertex is such that the tetrahedron formed with:
    //  vertex lv
    //  facet_vertex_[lv][0]
    //  facet_vertex_[lv][1]
    //  facet_vertex_[lv][2]
    // has the same orientation as the original tetrahedron for any vertex lv.
    unsigned int Delaunay3::facet_vertex_[4][3] = {
            {1, 2, 3},
            {0, 3, 2},
            {3, 0, 1},
            {2, 1, 0}
    };

//________________________________________________________________________________

    // \cond
    namespace internal {

        inline bool contains(const std::vector<unsigned int> &V, unsigned int x) {
            return std::find(V.begin(), V.end(), x) != V.end();
        }

        inline unsigned int other(unsigned int i1, unsigned int i2, unsigned int i3) {
            for (unsigned int i = 0; i < 4; i++) {
                if (i != i1 && i != i2 && i != i3) {
                    return i;
                }
            }
            DCHECK(false) << "should not have reached here";
            return 4;
        }

    }
    // \endcond

    void Delaunay3::get_voronoi_cell(unsigned int v, VoronoiCell3d &cell, bool geometry) const {
        if (nb_cells() == 0) {
            // Security Radius is currently not implemented in 3D.
            // Standard Delaunay implementation (TETGEN, QHULL or CGAL) should be used instead.
            bool RoS_mode = true;
            assert(!RoS_mode);
            (void)RoS_mode;
        }
        assert(v < nb_vertices());
        cell.clear();
        std::vector<unsigned int> visited_neigh;

        // For each t incident to v
        unsigned int t = vertex_cell(v);
        do {
            unsigned int lvit = index(t, static_cast<int>(v));

            // For each edge (t,neigh) incident to v
            for (unsigned int lv = 0; lv < 4; lv++) {
                unsigned int neigh = tet_vertex(t, lv);
                if (lv != lvit && !internal::contains(visited_neigh, neigh)) {
                    visited_neigh.push_back(neigh);
                    get_voronoi_facet(cell, t, lvit, lv, geometry);
                }
            }
            t = next_around_vertex(t, lvit);
        } while (t != vertex_cell(v));
    }


    void Delaunay3::get_voronoi_facet(
            VoronoiCell3d &cell, unsigned int t,
            unsigned int lv1, unsigned int lv2, bool geometry
    ) const {
        unsigned int v1 = tet_vertex(t, lv1);
        unsigned int v2 = tet_vertex(t, lv2);
        int first = static_cast<int>(t), prev, cur, next;  // References to tetrahedra.
        unsigned int f;                  // Current facet.
        unsigned int eb1;                // First "edge bisector", i.e. index of vertex
        // opposite to edge (v1,v2) in the triangle
        // that "enters" the volume when turning
        // around the edge (v1,v2).

        // Start iteration from a tetrahedron incident to the border
        // (makes border management much simpler !)
        do {
            lv1 = index(first, static_cast<int>(v1));
            lv2 = index(first, static_cast<int>(v2));
            f = next_around_halfedge_[lv2][lv1];  // (= prev_around_halfedge(lv1,lv2))
            prev = tet_adjacent(first, f);
            if (prev == -1) {
                eb1 = tet_vertex(first, other_in_face(f, lv1, lv2));
                break;
            }
            first = prev;
        } while (first != t);

        cell.begin_facet(v2);

        cur = first;

        // Border management: infinite vertex #1
        if (prev == -1) {
            if (geometry) {
                cell.add_to_facet(static_cast<int>(eb1), 10.0f * facet_normal(cur, f), true);
            } else {
                cell.add_to_facet(static_cast<int>(eb1), true);
            }
        }

        do {
            lv1 = index(cur, static_cast<int>(v1));
            lv2 = index(cur, static_cast<int>(v2));
            unsigned int f = next_around_halfedge_[lv1][lv2];
            unsigned int lv3 = internal::other(lv1, lv2, f);

            if (geometry) {
                cell.add_to_facet(tet_vertex(cur, lv3), tet_circumcenter(cur), false);
            } else {
                cell.add_to_facet(tet_vertex(cur, lv3), false);
            }
            next = tet_adjacent(cur, f);
            if (next == -1) { break; }
            cur = next;
        } while (cur != first);

        // Border management: infinite vertex #2
        if (next == -1) {
            if (geometry) {
                cell.add_to_facet(-1, 10.0 * facet_normal(cur, f), true);
            } else {
                cell.add_to_facet(-1, true);
            }
        }

        cell.end_facet();
    }

}