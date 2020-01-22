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

#ifndef EASY3D_CORE_SURFACE_MESH_BUILDER_H
#define EASY3D_CORE_SURFACE_MESH_BUILDER_H

#include <easy3d/core/surface_mesh.h>

#include <set>
#include <vector>


namespace easy3d {

    class SurfaceMeshBuilder {
    private:
        enum state {
            initial, surface, facet, final
        };

    public:
        SurfaceMeshBuilder(SurfaceMesh *target, bool quiet = false)
                : mesh_(target)
                , state_(initial)
                , quiet_(quiet)
        {
        }

        virtual void begin_surface();
        virtual void end_surface();
        virtual void reset();

        virtual SurfaceMesh::Vertex add_vertex(const vec3 &p);
        virtual SurfaceMesh::Vertex add_vertex(unsigned int id, const vec3 &p);

        virtual void begin_facet();
        virtual void end_facet();

        virtual void add_vertex_to_facet(int i);

        virtual void set_vertex(unsigned int v, const vec3 &p);
        virtual void set_vertex_color(unsigned int v, const vec3 &c);
        virtual void create_vertices(unsigned int nb_vertices, bool with_colors = false);

        SurfaceMesh::Vertex current_vertex();
        SurfaceMesh::Vertex vertex(int i);

        SurfaceMesh::Face current_facet();

        void set_quiet(bool quiet) { quiet_ = quiet; }

    protected:

        // Important note: in this class, all the Stars correspond to the
        // set of halfedges radiating FROM a vertex (i.e. h->vertex() != v
        // if h belongs to Star(v) ).

        SurfaceMesh::Vertex add_vertex_internal(unsigned int id, const vec3 &p);
        SurfaceMesh::Vertex add_vertex_internal(const vec3 &p);

        void begin_facet_internal();
        void end_facet_internal();

        void add_vertex_to_facet_internal(SurfaceMesh::Vertex v);

        SurfaceMesh::Vertex copy_vertex(SurfaceMesh::Vertex from_vertex);

        std::vector<SurfaceMesh::Vertex >   facet_vertex_;

    private:

        SurfaceMesh::Halfedge new_halfedge_between(SurfaceMesh::Vertex from, SurfaceMesh::Vertex to);
        SurfaceMesh::Halfedge find_halfedge_between(SurfaceMesh::Vertex from, SurfaceMesh::Vertex to);

        /**
        * Checks whether a vertex is manifold, by
        * checking that the star_ attribute and the
        * computed star have the same number of Halfedges.
        * Note: this test is valid only if the borders
        * have been constructed.
        */
        bool vertex_is_manifold(SurfaceMesh::Vertex v);

        /**
        * splits a non-manifold vertex into several vertices, as
        * needed. returns true if the vertex was non-manifold.
        */
        bool split_non_manifold_vertex(SurfaceMesh::Vertex v);

        /**
        * used by split_non_manifold_vertex:
        * sets v's halfedge pointer,
        * sets the halfedge vertex pointers to v,
        * removes the concerned halfedges from star,
        * updates the star of the concerned vertex,
        * if borders are found, connects them
        */
        void disconnect_vertex(
                SurfaceMesh::Halfedge start, SurfaceMesh::Vertex v,
                std::set<SurfaceMesh::Halfedge> &star
        );

        void terminate_surface();

        friend class SurfaceMeshSerializer_eobj;

        void transition(state from, state to);

        void check_state(state s);

        std::string state_to_string(state s);

    private:

        SurfaceMesh* mesh_;


        state state_;
        std::vector<SurfaceMesh::Vertex> vertex_;

        SurfaceMesh::Face current_facet_;
        SurfaceMesh::Vertex current_vertex_;
        SurfaceMesh::Vertex first_vertex_in_facet_;
        SurfaceMesh::Halfedge first_halfedge_in_facet_;
        SurfaceMesh::Halfedge current_halfedge_;

        typedef std::vector<SurfaceMesh::Halfedge> Star;
        SurfaceMesh::VertexProperty<Star> star_;
        SurfaceMesh::VertexProperty<vec3> vertices_;

        int nb_isolated_v_;
        int nb_non_manifold_v_;
        int nb_duplicate_e_;
        bool quiet_;
    };

}

#endif // EASY3D_CORE_SURFACE_MESH_BUILDER_H

