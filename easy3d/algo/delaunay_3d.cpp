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
        if (tetgen_in_) delete tetgen_in_;
        if (tetgen_out_) delete tetgen_out_;
    }

    void Delaunay3::set_vertices(unsigned int nb_vertices, const float *vertices) {
        Delaunay::set_vertices(nb_vertices, vertices);
        tetgen_out_->deinitialize();
        tetgen_in_->numberofpoints = nb_vertices;

        // Liangliang: if you provide "vertices" as double type, you can just assign the pointer, but it
        //			   seems float may not work properly for some of my data, so I always use double.
        //tetgen_in_->pointlist = (float*)vertices;
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


        } catch (...) {
            LOG(ERROR) << "encountered a problem...";
        }

        set_arrays(
                tetgen_out_->numberoftetrahedra,
                tetgen_out_->tetrahedronlist, tetgen_out_->neighborlist
        );

        LOG(INFO) << "done. time: " << t.time_string();
    }






//________________________________________________________________________________

// Pre-computed table for turning around halfedges in
// a tetrahedron. Given a cell c and an halfedge (v1, v2), 
// c->neighbor(next_around_halfedge[v1][v2]) is the cell
// adjacent to c on the left of (v1,v2).
// Diagonal entries are not supposed to be accessed.
    unsigned int Delaunay3::next_around_halfedge_[4][4] = {
            {5, 3, 1, 2},
            {2, 5, 3, 0},
            {3, 0, 5, 1},
            {1, 2, 0, 5}
    };

    unsigned int Delaunay3::facet_vertex_[4][3] = {
            {1, 2, 3},
            {0, 3, 2},
            {3, 0, 1},
            {2, 1, 0}
    };

//________________________________________________________________________________

    inline bool contains(const std::vector<unsigned int> &V, unsigned int x) {
        for (unsigned int i = 0; i < V.size(); i++) {
            if (V[i] == x) { return true; }
        }
        return false;
    }

    void Delaunay3::get_voronoi_cell(unsigned int v, VoronoiCell3d &cell, bool geometry) const {
        if (nb_cells() == 0) {
            // Security Radius is currently not implemented in 3D.
            // Standard Delaunay implementation (TETGEN, QHULL or CGAL)
            // should be used instead.
            bool RoS_mode = true;
            assert(!RoS_mode);
        }
        assert(v < nb_vertices());
        cell.clear();
        std::vector<unsigned int> visited_neigh;

        // For each t incident to v
        unsigned int t = vertex_cell(v);
        do {
            unsigned int lvit = index(t, v);

            // For each edge (t,neigh) incident to v
            for (unsigned int lv = 0; lv < 4; lv++) {
                unsigned int neigh = tet_vertex(t, lv);
                if (lv != lvit && !contains(visited_neigh, neigh)) {
                    visited_neigh.push_back(neigh);
                    get_voronoi_facet(cell, t, lvit, lv, geometry);
                }
            }
            t = next_around_vertex(t, lvit);
        } while (t != vertex_cell(v));
    }

    inline unsigned int other(unsigned int i1, unsigned int i2, unsigned int i3) {
        for (unsigned int i = 0; i < 4; i++) {
            if (i != i1 && i != i2 && i != i3) {
                return i;
            }
        }
        DLOG_ASSERT(false) << "should not have reached here";
        return 4;
    }

    void Delaunay3::get_voronoi_facet(
            VoronoiCell3d &cell, unsigned int t,
            unsigned int lv1, unsigned int lv2, bool geometry
    ) const {
        unsigned int v1 = tet_vertex(t, lv1);
        unsigned int v2 = tet_vertex(t, lv2);
        int first = t, prev, cur, next;  // References to tetrahedra.
        unsigned int f;                  // Current facet.
        unsigned int eb1;                // First "edge bisector", i.e. index of vertex
        // opposite to edge (v1,v2) in the triangle
        // that "enters" the volume when turning
        // around the edge (v1,v2).

        // Start iteration from a tetrahedron incident to the border
        // (makes border management much simpler !)
        do {
            lv1 = index(first, v1);
            lv2 = index(first, v2);
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
                cell.add_to_facet(eb1, 10.0 * facet_normal(cur, f), true);
            } else {
                cell.add_to_facet(eb1, true);
            }
        }

        do {
            lv1 = index(cur, v1);
            lv2 = index(cur, v2);
            unsigned int f = next_around_halfedge_[lv1][lv2];
            unsigned int lv3 = other(lv1, lv2, f);

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