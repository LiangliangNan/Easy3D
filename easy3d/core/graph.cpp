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

#include <easy3d/core/graph.h>

#include <algorithm>


namespace easy3d {


    Graph::Graph()
    {
        // allocate standard properties
        // same list is used in operator=() and assign()
        vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
        econn_    = add_edge_property<EdgeConnectivity>("e:connectivity");
        vpoint_   = add_vertex_property<vec3>("v:point");
        vdeleted_ = add_vertex_property<bool>("v:deleted", false);
        edeleted_ = add_edge_property<bool>("e:deleted", false);

        mprops_.push_back();

        deleted_vertices_ = deleted_edges_ = 0;
        garbage_ = false;
    }


    //-----------------------------------------------------------------------------


    Graph& Graph::operator=(const Graph& rhs)
    {
        if (this != &rhs)
        {
            // deep copy of property containers
            vprops_ = rhs.vprops_;
            eprops_ = rhs.eprops_;
            mprops_ = rhs.mprops_;

            // property handles contain pointers, have to be reassigned
            vconn_    = vertex_property<VertexConnectivity>("v:connectivity");
            econn_    = edge_property<EdgeConnectivity>("e:connectivity");
            vdeleted_ = vertex_property<bool>("v:deleted");
            edeleted_ = edge_property<bool>("e:deleted");
            vpoint_   = vertex_property<vec3>("v:point");

            // how many elements are deleted?
            deleted_vertices_ = rhs.deleted_vertices_;
            deleted_edges_    = rhs.deleted_edges_;
            garbage_          = rhs.garbage_;
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    Graph& Graph::assign(const Graph& rhs)
    {
        if (this != &rhs)
        {
            // clear properties
            vprops_.clear();
            eprops_.clear();
            mprops_.clear();

            // allocate standard properties
            vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
            econn_    = add_edge_property<EdgeConnectivity>("e:connectivity");
            vpoint_   = add_vertex_property<vec3>("v:point");
            vdeleted_ = add_vertex_property<bool>("v:deleted", false);
            edeleted_ = add_edge_property<bool>("e:deleted", false);

            // copy properties from other graph
            vconn_.array()     = rhs.vconn_.array();
            econn_.array()     = rhs.econn_.array();
            vpoint_.array()    = rhs.vpoint_.array();
            vdeleted_.array()  = rhs.vdeleted_.array();
            edeleted_.array()  = rhs.edeleted_.array();

            // resize (needed by property containers)
            vprops_.resize(rhs.vertices_size());
            eprops_.resize(rhs.edges_size());
            mprops_.resize(1);

            // how many elements are deleted?
            deleted_vertices_ = rhs.deleted_vertices_;
            deleted_edges_    = rhs.deleted_edges_;
            garbage_          = rhs.garbage_;
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    void Graph::clear()
    {
        //---- clear without removing properties

        vprops_.resize(0);
        eprops_.resize(0);
        mprops_.resize(0);

        vprops_.shrink_to_fit();
        eprops_.shrink_to_fit();
        mprops_.shrink_to_fit();

        deleted_vertices_ = deleted_edges_ = 0;
        garbage_ = false;

        //---- keep the standard properties and remove all the other properties

        vprops_.resize_property_array(3);   // "v:connectivity", "v:point", "v:deleted"
        eprops_.resize_property_array(2);   // "e:connectivity", "e:deleted"

        mprops_.clear();
        mprops_.resize(1);
    }


    //-----------------------------------------------------------------------------


    void Graph::reserve(unsigned int nvertices, unsigned int nedges)
    {
        vprops_.reserve(nvertices);
        eprops_.reserve(nedges);
        mprops_.reserve(1);
    }


    //-----------------------------------------------------------------------------


    void Graph::property_stats(std::ostream& output) const
    {
        std::vector<std::string> props;

        props = model_properties();
        if (!props.empty())
        {
            output << "model properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

		props = vertex_properties();
		if (!props.empty())
		{
            output << "vertex properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
		}


        props = edge_properties();
		if (!props.empty())
		{
            output << "edge properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
		}
    }


    //-----------------------------------------------------------------------------


    Graph::Vertex Graph:: add_vertex(const vec3& p)
    {
        Vertex v = new_vertex();
        vpoint_[v] = p;
        return v;
    }


    Graph::Edge Graph::add_edge(const Vertex& v1, const Vertex& v2) {
        if (!is_valid(v1) || !is_valid(v2) || is_deleted(v1) || is_deleted(v2)) {
            LOG(WARNING) << "cannot connect an invalid or deleted vertex [" << v1 << ", " << v2 << "]";
            return Edge();
        }

        // a self-loop cannot be represented: the circulators and find_edge() assume the two
        // endpoints of an edge are different vertices.
        if (v1 == v2) {
            LOG(WARNING) << "cannot add a self-loop on vertex [" << v1 << "]";
            return Edge();
        }

        auto e = find_edge(v1, v2);
        if (e.is_valid()) {
            LOG(WARNING) << "edge already exists [" << v1 << ", " << v2 << "]";
            return e;
        }

        e = new_edge();
        econn_[e].source_ = v1;
        econn_[e].target_ = v2;
        vconn_[v1].edges_.push_back(e);
        vconn_[v2].edges_.push_back(e);
        return e;
    }


    //-----------------------------------------------------------------------------


    Graph::Edge Graph::find_edge(Vertex v1, Vertex v2) const
    {
        if (!is_valid(v1) || !is_valid(v2))
            return Edge();

        for (auto e : edges(v1)) {
            if (vertex(e, 0) == v2 || vertex(e, 1) == v2)
                return e;
        }

        return Edge();
    }


    //-----------------------------------------------------------------------------


    unsigned int Graph::valence(Vertex v) const
    {
        return static_cast<unsigned int>(vconn_[v].edges_.size());
    }


    //-----------------------------------------------------------------------------

    float Graph::edge_length(Edge e) const
    {
        return norm(vpoint_[vertex(e, 0)] - vpoint_[vertex(e, 1)]);
    }


    //-----------------------------------------------------------------------------

    void Graph::delete_vertex(Vertex v)
    {
        if (!is_valid(v) || vdeleted_[v])
            return;

        // the incident edges cannot survive their vertex: note that the connectivity is
        // copied because delete_edge() removes the edge from the adjacency list.
        const std::vector<Edge> incident = vconn_[v].edges_;
        for (auto e : incident)
            delete_edge(e);

        vdeleted_[v] = true;
        deleted_vertices_++;
        garbage_ = true;
    }


    //-----------------------------------------------------------------------------


    void Graph::delete_edge(Edge e)
    {
        if (!is_valid(e) || edeleted_[e])
            return;

        // unlink the edge from the adjacency lists of its endpoints: unlike the mesh
        // halfedges, the graph circulators iterate over these lists directly and do not
        // skip deleted edges.
        for (unsigned int i = 0; i < 2; ++i) {
            auto& edges = vconn_[vertex(e, i)].edges_;
            edges.erase(std::remove(edges.begin(), edges.end(), e), edges.end());
        }

        edeleted_[e] = true;
        deleted_edges_++;
        garbage_ = true;
    }


    //-----------------------------------------------------------------------------

    void Graph::collect_garbage()
    {
        if (!garbage_)
            return;

        const int nV = static_cast<int>(vertices_size());
        const int nE = static_cast<int>(edges_size());

        // setup handle mapping: because the mapping is a property, compacting an element
        // moves its entry along and the entries of the live elements end up being the
        // handles they get in the compacted graph.
        auto vmap = add_vertex_property<Vertex>("v:garbage-collection");
        auto emap = add_edge_property<Edge>("e:garbage-collection");
        for (int i = 0; i < nV; ++i)
            vmap[Vertex(i)] = Vertex(i);
        for (int i = 0; i < nE; ++i)
            emap[Edge(i)] = Edge(i);

        // remove deleted vertices
        int new_nv = nV;
        if (nV > 0)
        {
            int i0 = 0;
            int i1 = nV - 1;

            while (true)
            {
                // find first deleted and last un-deleted
                while (!vdeleted_[Vertex(i0)] && i0 < i1)  ++i0;
                while ( vdeleted_[Vertex(i1)] && i0 < i1)  --i1;
                if (i0 >= i1) break;

                // swap
                vprops_.swap(i0, i1);
            };

            // remember new size
            new_nv = vdeleted_[Vertex(i0)] ? i0 : i0 + 1;
        }

        // remove deleted edges
        int new_ne = nE;
        if (nE > 0)
        {
            int i0 = 0;
            int i1 = nE - 1;

            while (true)
            {
                // find first deleted and last un-deleted
                while (!edeleted_[Edge(i0)] && i0 < i1)  ++i0;
                while ( edeleted_[Edge(i1)] && i0 < i1)  --i1;
                if (i0 >= i1) break;

                // swap
                eprops_.swap(i0, i1);
            };

            // remember new size
            new_ne = edeleted_[Edge(i0)] ? i0 : i0 + 1;
        }

        // the connectivity stores handles, so it has to be remapped: the adjacency lists
        // reference edges and the edge endpoints reference vertices
        for (int i = 0; i < new_ne; ++i) {
            auto& conn = econn_[Edge(i)];
            conn.source_ = vmap[conn.source_];
            conn.target_ = vmap[conn.target_];
        }

        for (int i = 0; i < new_nv; ++i) {
            for (auto& e : vconn_[Vertex(i)].edges_)
                e = emap[e];
        }

        // remove handle maps
        remove_vertex_property(vmap);
        remove_edge_property(emap);

        // finally resize arrays
        vprops_.resize(new_nv);
        eprops_.resize(new_ne);
        vprops_.shrink_to_fit();
        eprops_.shrink_to_fit();

        deleted_vertices_ = deleted_edges_ = 0;
        garbage_ = false;
    }


} // namespace easy3d
