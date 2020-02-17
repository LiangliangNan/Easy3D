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

#include <easy3d/core/graph.h>

#include <cmath>


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


    Graph::~Graph()
    {
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

            // copy properties from other mesh
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
        vprops_.resize(0);
        eprops_.resize(0);
        mprops_.resize(0);

        free_memory();

        deleted_vertices_ = deleted_edges_ = 0;
        garbage_ = false;
    }


    //-----------------------------------------------------------------------------


    void Graph::free_memory()
    {
        vprops_.free_memory();
        eprops_.free_memory();
        mprops_.free_memory();
    }


    //-----------------------------------------------------------------------------


    void Graph::reserve(unsigned int nvertices, unsigned int nedges)
    {
        vprops_.reserve(nvertices);
        eprops_.reserve(nedges);
        mprops_.reserve(1);
    }


    //-----------------------------------------------------------------------------


    void Graph::property_stats() const
    {
        std::vector<std::string> props;

		props = vertex_properties();
		if (!props.empty())
		{
			std::cout << "vertex properties:\n";
			for (unsigned int i = 0; i < props.size(); ++i)
				std::cout << "\t" << props[i] << std::endl;
		}


        props = edge_properties();
		if (!props.empty())
		{
			std::cout << "edge properties:\n";
			for (unsigned int i = 0; i < props.size(); ++i)
				std::cout << "\t" << props[i] << std::endl;
		}

		props = model_properties();
		if (!props.empty())
		{
			std::cout << "model properties:\n";
			for (unsigned int i = 0; i < props.size(); ++i)
				std::cout << "\t" << props[i] << std::endl;
		}
    }


    //-----------------------------------------------------------------------------


    Graph::Vertex Graph:: add_vertex(const vec3& p)
    {
        Vertex v = new_vertex();
        vpoint_[v] = p;
        return v;
    }


    Graph::Edge Graph::add_edge(const Vertex& start, const Vertex& end) {
        assert(start != end);
        Edge e = new_edge();
        econn_[e].source_ = start;
        econn_[e].target_ = end;
        vconn_[start].edges_.push_back(e);
        vconn_[end].edges_.push_back(e);
        return e;
    }


    //-----------------------------------------------------------------------------


    Graph::Edge Graph::find_edge(Vertex start, Vertex end) const
    {
        assert(is_valid(start) && is_valid(end));

        for (auto e : edges(start)) {
            if (from_vertex(e) == end || to_vertex(e) == end)
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
        return norm(vpoint_[from_vertex(e)] - vpoint_[to_vertex(e)]);
    }


    //-----------------------------------------------------------------------------

    void Graph::delete_vertex(Vertex v)
    {
    //    if (vdeleted_[v])  return;

    //    // collect incident faces
    //    std::vector<Face> incident_faces;
    //    incident_faces.reserve(6);

    //    FaceAroundVertexCirculator fc, fc_end;
    //    fc = fc_end = faces(v);

    //    if (fc)
    //    do
    //    {
    //        incident_faces.push_back(*fc);
    //    } while (++fc != fc_end);

    //    // delete incident faces
    //    std::vector<Face>::iterator fit(incident_faces.begin()),
    //                                fend(incident_faces.end());

    //    for (; fit != fend; ++fit)
    //        delete_face(*fit);

    //    // mark v as deleted if not yet done by delete_face()
    //    if (!vdeleted_[v])
    //    {
    //        vdeleted_[v] = true;
    //        deleted_vertices_++;
    //        garbage_ = true;
    //    }
    }


    //-----------------------------------------------------------------------------


    void Graph::delete_edge(Edge e)
    {
    //    if (edeleted_[e])  return;

    //    Face f0 = face(halfedge(e, 0));
    //    Face f1 = face(halfedge(e, 1));

    //    if (f0.is_valid()) delete_face(f0);
    //    if (f1.is_valid()) delete_face(f1);
    }


    //-----------------------------------------------------------------------------

    void Graph::garbage_collection()
    {
    //    int  i, i0, i1,
    //    nV(vertices_size()),
    //    nE(edges_size()),
    //    nH(halfedges_size()),

    //    Vertex    v;
    //    Halfedge  h;


    //    // setup handle mapping
    //    VertexProperty<Vertex>      vmap = add_vertex_property<Vertex>("v:garbage-collection");
    //    EdgeProperty<Edge>          emap = add_edge_property<Edge>("e:garbage-collection");

    //    for (i=0; i<nV; ++i)
    //        vmap[Vertex(i)] = Vertex(i);
    //    for (i=0; i<nH; ++i)
    //        emap[Edge(i)] = Edge(i);

    //    // remove deleted vertices
    //    if (nV > 0)
    //    {
    //        i0=0;  i1=nV-1;

    //        while (1)
    //        {
    //            // find first deleted and last un-deleted
    //            while (!vdeleted_[Vertex(i0)] && i0 < i1)  ++i0;
    //            while ( vdeleted_[Vertex(i1)] && i0 < i1)  --i1;
    //            if (i0 >= i1) break;

    //            // swap
    //            vprops_.swap(i0, i1);
    //        };

    //        // remember new size
    //        nV = vdeleted_[Vertex(i0)] ? i0 : i0+1;
    //    }


    //    // remove deleted edges
    //    if (nE > 0)
    //    {
    //        i0=0;  i1=nE-1;

    //        while (1)
    //        {
    //            // find first deleted and last un-deleted
    //            while (!edeleted_[Edge(i0)] && i0 < i1)  ++i0;
    //            while ( edeleted_[Edge(i1)] && i0 < i1)  --i1;
    //            if (i0 >= i1) break;

    //            // swap
    //            eprops_.swap(i0, i1);
    //            eprops_.swap(2*i0,   2*i1);
    //        };

    //        // remember new size
    //        nE = edeleted_[Edge(i0)] ? i0 : i0+1;
    //        nH = 2*nE;
    //    }

    //    // update vertex connectivity
    //    for (i=0; i<nV; ++i)
    //    {
    //        v = Vertex(i);
    //        if (!is_isolated(v))
    //            set_halfedge(v, hmap[edges(v)]);
    //    }


    //    // update halfedge connectivity
    //    for (i=0; i<nH; ++i)
    //    {
    //        h = Halfedge(i);
    //        set_vertex(h, vmap[to_vertex(h)]);
    //        set_next_halfedge(h, hmap[next_halfedge(h)]);
    //        if (!is_boundary(h))
    //            set_face(h, fmap[face(h)]);
    //    }

    //    // remove handle maps
    //    remove_vertex_property(vmap);
    //    remove_edge_property(emap);


    //    // finally resize arrays
    //    vprops_.resize(nV); vprops_.free_memory();
    //    eprops_.resize(nH); eprops_.free_memory();

    //    deleted_vertices_ = deleted_edges_ = 0;
    //    garbage_ = false;
    }


//    std::vector<Graph::Vertex> Graph::vertices(Vertex v) const {
//        assert(v.is_valid());
//        std::vector<Graph::Vertex> result;
//        for (auto e : edges(v)) {
//            Vertex another = from_vertex(e);
//            if (another != v)
//                result.push_back(another);
//            else
//                result.push_back(to_vertex(e));
//        }

//        return result;
//    }


} // namespace easy3d
