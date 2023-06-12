/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2013 by Graphics & Geometry Group, Bielefeld University
 * Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
 *
 * The code in this file is partly from Surface_mesh (v1.1) with
 * modifications and enhancement:
 *      https://opensource.cit-ec.de/projects/surface_mesh
 * The original code was distributed under the GNU GPL License.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation, version 2.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ********************************************************************/

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>

#include <cmath>
#include <fstream>

namespace easy3d {

    SurfaceMesh::SurfaceMesh()
    {
        // allocate standard properties
        // same list is used in operator=() and assign()
        vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
        hconn_    = add_halfedge_property<HalfedgeConnectivity>("h:connectivity");
        fconn_    = add_face_property<FaceConnectivity>("f:connectivity");
        vpoint_   = add_vertex_property<vec3>("v:point");
        vdeleted_ = add_vertex_property<bool>("v:deleted", false);
        edeleted_ = add_edge_property<bool>("e:deleted", false);
        fdeleted_ = add_face_property<bool>("f:deleted", false);

        mprops_.push_back();

        deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
        garbage_ = false;
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh& SurfaceMesh::operator=(const SurfaceMesh& rhs)
    {
        if (this != &rhs)
        {
            // deep copy of property containers
            vprops_ = rhs.vprops_;
            hprops_ = rhs.hprops_;
            eprops_ = rhs.eprops_;
            fprops_ = rhs.fprops_;
            mprops_ = rhs.mprops_;

            // property handles contain pointers, have to be reassigned
            vconn_    = vertex_property<VertexConnectivity>("v:connectivity");
            hconn_    = halfedge_property<HalfedgeConnectivity>("h:connectivity");
            fconn_    = face_property<FaceConnectivity>("f:connectivity");
            vdeleted_ = vertex_property<bool>("v:deleted");
            edeleted_ = edge_property<bool>("e:deleted");
            fdeleted_ = face_property<bool>("f:deleted");
            vpoint_   = vertex_property<vec3>("v:point");

            // normals might be there, therefore use get_property
            vnormal_  = get_vertex_property<vec3>("v:normal");
            fnormal_  = get_face_property<vec3>("f:normal");

            // how many elements are deleted?
            deleted_vertices_ = rhs.deleted_vertices_;
            deleted_edges_    = rhs.deleted_edges_;
            deleted_faces_    = rhs.deleted_faces_;
            garbage_          = rhs.garbage_;
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh &SurfaceMesh::join(const SurfaceMesh &other) {
        // increase capacity
        const unsigned int nv = vertices_size(), nh = halfedges_size(), nf = faces_size();
        resize(vertices_size() + other.vertices_size(),
               edges_size() + other.edges_size(),
               faces_size() + other.faces_size());

        // append properties in the free space created by resize
        vprops_.transfer(other.vprops_);
        hprops_.transfer(other.hprops_);
        fprops_.transfer(other.fprops_);
        eprops_.transfer(other.eprops_);

        // translate halfedge index in vertex -> halfedge
        for (unsigned int i = nv; i < nv + other.vertices_size(); i++) {
            Vertex vi(static_cast<int>(i));
            if (vconn_[vi].halfedge_ != Halfedge()) {
                vconn_[vi].halfedge_ = Halfedge(vconn_[vi].halfedge_.idx() + static_cast<int>(nh));
            }
        }
        // translate halfedge index in face -> halfedge
        for (unsigned int i = nf; i < nf + other.faces_size(); i++) {
            Face fi(static_cast<int>(i));
            if (fconn_[fi].halfedge_ != Halfedge()) {
                fconn_[fi].halfedge_ = Halfedge(fconn_[fi].halfedge_.idx() + static_cast<int>(nh));
            }
        }
        // translate indices in halfedge -> face, halfedge -> target, halfedge -> prev, and halfedge -> next
        for (unsigned int i = nh; i < nh + other.halfedges_size(); i++) {
            Halfedge hi(static_cast<int>(i));
            if (hconn_[hi].face_ != Face()) {
                hconn_[hi].face_ = Face(hconn_[hi].face_.idx() + static_cast<int>(nf));
            }
            if (hconn_[hi].vertex_ != Vertex()) {
                hconn_[hi].vertex_ = Vertex(hconn_[hi].vertex_.idx() + static_cast<int>(nv));
            }
            if (hconn_[hi].next_ != Halfedge()) {
                hconn_[hi].next_ = Halfedge(hconn_[hi].next_.idx() + static_cast<int>(nh));
            }
            if (hconn_[hi].prev_ != Halfedge()) {
                hconn_[hi].prev_ = Halfedge(hconn_[hi].prev_.idx() + static_cast<int>(nh));
            }
        }

        // update garbage infos
        garbage_ = garbage_ || other.garbage_;
        deleted_vertices_ += other.deleted_vertices_;
        deleted_edges_ += other.deleted_edges_;
        deleted_faces_ += other.deleted_faces_;
        return *this;
    }


    //-----------------------------------------------------------------------------

    
    SurfaceMesh& SurfaceMesh::assign(const SurfaceMesh& rhs)
    {
        if (this != &rhs)
        {
            // clear properties
            vprops_.clear();
            hprops_.clear();
            eprops_.clear();
            fprops_.clear();
            mprops_.clear();

            // allocate standard properties
            vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
            hconn_    = add_halfedge_property<HalfedgeConnectivity>("h:connectivity");
            fconn_    = add_face_property<FaceConnectivity>("f:connectivity");
            vpoint_   = add_vertex_property<vec3>("v:point");
            vdeleted_ = add_vertex_property<bool>("v:deleted", false);
            edeleted_ = add_edge_property<bool>("e:deleted", false);
            fdeleted_ = add_face_property<bool>("f:deleted", false);

            // normals might be there, therefore use get_property
            vnormal_  = get_vertex_property<vec3>("v:normal");
            fnormal_  = get_face_property<vec3>("f:normal");

            // copy properties from other mesh
            vconn_.array()     = rhs.vconn_.array();
            hconn_.array()     = rhs.hconn_.array();
            fconn_.array()     = rhs.fconn_.array();
            vpoint_.array()    = rhs.vpoint_.array();
            vdeleted_.array()  = rhs.vdeleted_.array();
            edeleted_.array()  = rhs.edeleted_.array();
            fdeleted_.array()  = rhs.fdeleted_.array();

            // resize (needed by property containers)
            vprops_.resize(rhs.vertices_size());
            hprops_.resize(rhs.halfedges_size());
            eprops_.resize(rhs.edges_size());
            fprops_.resize(rhs.faces_size());
            mprops_.resize(1);

            // how many elements are deleted?
            deleted_vertices_ = rhs.deleted_vertices_;
            deleted_edges_    = rhs.deleted_edges_;
            deleted_faces_    = rhs.deleted_faces_;
            garbage_          = rhs.garbage_;
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::read(const std::string &file_name)
    {
        clear();

        // open file (in binary mode)
        std::ifstream input(file_name.c_str(), std::fstream::binary);
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        // how many elements?
        unsigned int nv, ne, nh, nf;
        input.read((char*)&nv, sizeof(unsigned int));
        input.read((char*)&ne, sizeof(unsigned int));
        input.read((char*)&nf, sizeof(unsigned int));
        nh = 2*ne;

        // resize containers
        resize(nv, ne, nf);

        // get properties
        auto vconn = vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
        auto hconn = halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
        auto fconn = face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
        auto point = vertex_property<vec3>("v:point");

        // read properties from file
        input.read((char*)vconn.data(), static_cast<long>(nv * sizeof(SurfaceMesh::VertexConnectivity)  ));
        input.read((char*)hconn.data(), static_cast<long>(nh * sizeof(SurfaceMesh::HalfedgeConnectivity)));
        input.read((char*)fconn.data(), static_cast<long>(nf * sizeof(SurfaceMesh::FaceConnectivity)    ));
        input.read((char*)point.data(), static_cast<long>(nv * sizeof(vec3)                             ));

        bool has_colors = false;
        input.read((char*)&has_colors, sizeof(bool));
        if (has_colors) {
            SurfaceMesh::VertexProperty<vec3> color = vertex_property<vec3>("v:color");
            input.read((char*)color.data(), static_cast<long>(nv * sizeof(vec3)));
        }

        return n_faces() > 0;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::write(const std::string& file_name) const
    {
        if (n_faces() == 0) {
            LOG(ERROR) << "empty mesh";
            return false;
        }

        // open file (in binary mode)
        std::ofstream output(file_name.c_str(), std::fstream::binary);
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        // how many elements?
        unsigned int nv, ne, nh, nf;
        nv = n_vertices();
        ne = n_edges();
        nh = n_halfedges();
        nf = n_faces();
        nh = 2*ne;

        output.write((char*)&nv, sizeof(unsigned int));
        output.write((char*)&ne, sizeof(unsigned int));
        output.write((char*)&nf, sizeof(unsigned int));

        // get properties
        auto vconn = get_vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
        auto hconn = get_halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
        auto fconn = get_face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
        auto point = get_vertex_property<vec3>("v:point");

        // write properties to file
        output.write((char*)vconn.data(), static_cast<long>(nv * sizeof(SurfaceMesh::VertexConnectivity)));
        output.write((char*)hconn.data(), static_cast<long>(nh * sizeof(SurfaceMesh::HalfedgeConnectivity)));
        output.write((char*)fconn.data(), static_cast<long>(nf * sizeof(SurfaceMesh::FaceConnectivity)));
        output.write((char*)point.data(), static_cast<long>(nv * sizeof(vec3)));

        // check for colors
        auto color = get_vertex_property<vec3>("v:color");
        bool has_colors = color;
        output.write((char*)&has_colors, sizeof(bool));
        if (has_colors)
            output.write((char*)color.data(), static_cast<long>(nv * sizeof(vec3)));

        return true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::clear()
    {
        //---- clear without removing properties

        vprops_.resize(0);
        hprops_.resize(0);
        eprops_.resize(0);
        fprops_.resize(0);
        mprops_.resize(0);

        vprops_.shrink_to_fit(); // remove unused memory from vectors
        hprops_.shrink_to_fit();
        eprops_.shrink_to_fit();
        fprops_.shrink_to_fit();
        mprops_.shrink_to_fit();

        deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
        garbage_ = false;

        //---- keep the standard properties and remove all the other properties

        vprops_.resize_property_array(3);   // "v:connectivity", "v:point", "v:deleted"
        hprops_.resize_property_array(1);   // "h:connectivity"
        eprops_.resize_property_array(1);   // "e:deleted"
        fprops_.resize_property_array(2);   // "f:connectivity", "f:deleted"
        mprops_.clear();
        mprops_.resize(1);

        // update/invalidate the normal properties
        vnormal_  = VertexProperty<vec3>();
        fnormal_  = FaceProperty<vec3>();
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::reserve(unsigned int nvertices,
            unsigned int nedges,
            unsigned int nfaces )
    {
        vprops_.reserve(nvertices);
        hprops_.reserve(2*nedges);
        eprops_.reserve(nedges);
        fprops_.reserve(nfaces);
        mprops_.reserve(1);
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::property_stats(std::ostream& output) const
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

		props = halfedge_properties();
		if (!props.empty())
		{
            output << "halfedge properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
		}

		props = edge_properties();
		if (!props.empty())
		{
            output<< "edge properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
		}

		props = face_properties();
		if (!props.empty())
		{
            output << "face properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
		}

		props = model_properties();
		if (!props.empty())
		{
            output << "model properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
		}
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Halfedge SurfaceMesh::find_halfedge(Vertex start, Vertex end) const
    {
        assert(is_valid(start) && is_valid(end));

        Halfedge h  = out_halfedge(start);
        const Halfedge hh = h;

        if (h.is_valid())
        {
            do
            {
                if (target(h) == end)
                    return h;
                h = next_around_source(h);
            }
            while (h != hh);
        }

        return Halfedge();
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Edge SurfaceMesh::find_edge(Vertex a, Vertex b) const
    {
        Halfedge h = find_halfedge(a,b);
        return h.is_valid() ? edge(h) : Edge();
    }


    //-----------------------------------------------------------------------------

    void SurfaceMesh::adjust_outgoing_halfedges() {
        // We need to take care of isolated vertices
        auto reachable = add_vertex_property<bool>("v:temp:reachable", false);

        for (auto f : faces()) {
            for (auto h : halfedges(f)) {
                auto v = source(h);
                set_out_halfedge(v, h);
                adjust_outgoing_halfedge(v);
                reachable[target(h)] = true;
            }
        }

        for (auto v : vertices()) {
            if (!reachable[v]) {
                // mark this vertex isolated (by assigning an invalid halfedge)
                set_out_halfedge(v, Halfedge());
            }
        }
        remove_vertex_property(reachable);
    }

    void SurfaceMesh::adjust_outgoing_halfedge(Vertex v)
    {
        Halfedge h  = out_halfedge(v);
        const Halfedge hh = h;

        if (h.is_valid())
        {
            do
            {
                if (is_border(h))
                {
                    set_out_halfedge(v, h);
                    return;
                }
                h = next_around_source(h);
            }
            while (h != hh);
        }
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Face SurfaceMesh::add_triangle(Vertex v0, Vertex v1, Vertex v2)
    {
        add_face_vertices_.resize(3);
        add_face_vertices_[0] = v0;
        add_face_vertices_[1] = v1;
        add_face_vertices_[2] = v2;
        return add_face(add_face_vertices_);
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Face SurfaceMesh::add_quad(Vertex v0, Vertex v1, Vertex v2, Vertex v3)
    {
        add_face_vertices_.resize(4);
        add_face_vertices_[0] = v0;
        add_face_vertices_[1] = v1;
        add_face_vertices_[2] = v2;
        add_face_vertices_[3] = v3;
        return add_face(add_face_vertices_);
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Face SurfaceMesh::add_face(const std::vector<Vertex>& vertices)
    {
        const std::size_t n(vertices.size());
        assert (n > 2);

        std::vector<Vertex> vts = vertices;
        std::sort(vts.begin(), vts.end());
        auto duplicate_pos = std::adjacent_find(vts.begin(), vts.end());
        if (duplicate_pos != vts.end()) {
            LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: face contains duplicate vertex (" << *duplicate_pos << "). Face ignored. " << COUNTER;
#ifndef NDEBUG
            for (const auto v: vertices)
                LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
#endif
            return Face();
        }

        Vertex        v;
        std::size_t   i, ii, id;
        Halfedge      inner_next, inner_prev, outer_next, outer_prev, boundary_next, boundary_prev, patch_start, patch_end;

        // use global arrays to avoid new/delete of local arrays!!!
        std::vector<Halfedge>&  halfedges    = add_face_halfedges_;
        std::vector<bool>&      is_new       = add_face_is_new_;
        std::vector<bool>&      needs_adjust = add_face_needs_adjust_;
        NextCache&              next_cache   = add_face_next_cache_;
        halfedges.clear();
        halfedges.resize(n);
        is_new.clear();
        is_new.resize(n);
        needs_adjust.clear();
        needs_adjust.resize(n, false);
        next_cache.clear();
        next_cache.reserve(3*n);


        // test for topological errors
        for (i=0, ii=1; i<n; ++i, ++ii, ii%=n)
        {
            if ( !is_border(vertices[i]) )
            {
                LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex vertex (" << vertices[i] << "). " << COUNTER;

#ifndef NDEBUG
                static bool show = true;
                if (show) {
                    LOG(ERROR) << "\tvertex coordinates: ";
                    LOG(ERROR) << "\t\t" << vertices[i] << ": " << vpoint_[vertices[i]];
                    LOG(ERROR) << "\tvertices of the face: ";
                    for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                    show = false;
                }
#endif
                return Face();
            }

            halfedges[i] = find_halfedge(vertices[i], vertices[ii]);
            is_new[i]    = !halfedges[i].is_valid();

            if (!is_new[i] && !is_border(halfedges[i]))
            {
                LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edge (" << vertices[i] << " -> " << vertices[ii] << "). " << COUNTER;

#ifndef NDEBUG
                static bool show = true;
                if (show) {
                    LOG(ERROR) << "\tvertex coordinates: ";
                    LOG(ERROR) << "\t\t" << vertices[i] << ": " << vpoint_[vertices[i]];
                    LOG(ERROR) << "\t\t" << vertices[ii] << ": " << vpoint_[vertices[ii]];
                    LOG(ERROR) << "\tvertices of the face: ";
                    for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                    show = false;
                }
#endif
                return Face();
            }
        }


        // re-link patches if necessary
        for (i=0, ii=1; i<n; ++i, ++ii, ii%=n)
        {
            if (!is_new[i] && !is_new[ii])
            {
                inner_prev = halfedges[i];
                inner_next = halfedges[ii];

                if (next(inner_prev) != inner_next)
                {
                    // here comes the ugly part... we have to relink a whole patch

                    // search a free gap.
                    // free gap will be between boundary_prev and boundary_next
                    outer_prev = opposite(inner_next);
                    outer_next = opposite(inner_prev);
                    boundary_prev = outer_prev;
                    do
                    {
                        boundary_prev = opposite(next(boundary_prev));
                    }
                    while (!is_border(boundary_prev) || boundary_prev==inner_prev);
                    boundary_next = next(boundary_prev);
                    assert(is_border(boundary_prev));
                    assert(is_border(boundary_next));


                    // ok ?
                    if (boundary_next == inner_next)
                    {
                        LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: patch re-linking failed (" << vertices << "). " << COUNTER;
#ifndef NDEBUG
                        static bool show = true;
                        if (show) {
                            LOG(ERROR) << "\tvertices of the face: ";
                            for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                            show = false;
                        }
#endif
                        return Face();
                    }

                    // other halfedges' handles
                    patch_start = next(inner_prev);
                    patch_end   = prev(inner_next);

                    // relink
#ifndef NDEBUG
                    if (!boundary_prev.is_valid() || !patch_start.is_valid() || !patch_end.is_valid() ||
                        !boundary_next.is_valid() || !inner_prev.is_valid() || !inner_next.is_valid())
                    {
                        LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edges (" << vertices << "). " << COUNTER;
                        static bool show = true;
                        if (show) {
                            LOG(ERROR) << "\tvertices of the face: ";
                            for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                            show = false;
                        }
                        return Face();
                    }
#endif
                    next_cache.emplace_back(NextCacheEntry(boundary_prev, patch_start));
                    next_cache.emplace_back(NextCacheEntry(patch_end, boundary_next));
                    next_cache.emplace_back(NextCacheEntry(inner_prev, inner_next));
                }
            }
        }



        // create missing edges
        for (i=0, ii=1; i<n; ++i, ++ii, ii%=n)
        {
            if (is_new[i])
            {
                halfedges[i] = new_edge(vertices[i], vertices[ii]);
            }
        }



        // create the face
        Face f(new_face());
        set_halfedge(f, halfedges[n-1]);



        // setup halfedges
        for (i=0, ii=1; i<n; ++i, ++ii, ii%=n)
        {
            v          = vertices[ii];
            inner_prev = halfedges[i];
            inner_next = halfedges[ii];

            id = 0;
            if (is_new[i])  id |= 1;
            if (is_new[ii]) id |= 2;

            if (id)
            {
                outer_prev = opposite(inner_next);
                outer_next = opposite(inner_prev);

                // set outer links
                switch (id)
                {
                    case 1: // prev is new, next is old
                        boundary_prev = prev(inner_next);
#ifndef NDEBUG
                        if (!boundary_prev.is_valid() || !outer_next.is_valid())
                        {
                            LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edges (" << vertices << "). " << COUNTER;
                            static bool show = true;
                            if (show) {
                                LOG(ERROR) << "\tvertices of the face: ";
                                for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                                show = false;
                            }
                            return Face();
                        }
#endif
                        next_cache.emplace_back(NextCacheEntry(boundary_prev, outer_next));
                        set_out_halfedge(v, outer_next);
                        break;

                    case 2: // next is new, prev is old
                        boundary_next = next(inner_prev);
#ifndef NDEBUG
                        if (!outer_prev.is_valid() || !boundary_next.is_valid())
                        {
                            LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edges (" << vertices << "). " << COUNTER;
                            static bool show = true;
                            if (show) {
                                LOG(ERROR) << "\tvertices of the face: ";
                                for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                                show = false;
                            }
                            return Face();
                        }
#endif
                        next_cache.emplace_back(NextCacheEntry(outer_prev, boundary_next));
                        set_out_halfedge(v, boundary_next);
                        break;

                    case 3: // both are new
                        if (!out_halfedge(v).is_valid())
                        {
                            set_out_halfedge(v, outer_next);
#ifndef NDEBUG
                            if (!outer_prev.is_valid() || !outer_next.is_valid())
                            {
                                LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edges (" << vertices << "). " << COUNTER;
                                static bool show = true;
                                if (show) {
                                    LOG(ERROR) << "\tvertices of the face: ";
                                    for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                                    show = false;
                                }
                                return Face();
                            }
#endif
                            next_cache.emplace_back(NextCacheEntry(outer_prev, outer_next));
                        }
                        else
                        {
                            boundary_next = out_halfedge(v);
                            boundary_prev = prev(boundary_next);
#ifndef NDEBUG
                            if (!boundary_prev.is_valid() || !outer_next.is_valid() || !outer_prev.is_valid() || !boundary_next.is_valid())
                            {
                                LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edges (" << vertices << "). " << COUNTER;
                                static bool show = true;
                                if (show) {
                                    LOG(ERROR) << "\tvertices of the face: ";
                                    for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                                    show = false;
                                }
                                return Face();
                            }
#endif
                            next_cache.emplace_back(NextCacheEntry(boundary_prev, outer_next));
                            next_cache.emplace_back(NextCacheEntry(outer_prev, boundary_next));
                        }
                        break;
                }

                // set inner link
#ifndef NDEBUG
                if (!inner_prev.is_valid() || !inner_next.is_valid())
                {
                    LOG_N_TIMES(3, ERROR) << "SurfaceMesh::add_face: complex edges (" << vertices << "). " << COUNTER;
                    static bool show = true;
                    if (show) {
                        LOG(ERROR) << "\tvertices of the face: ";
                        for (auto v : vertices) LOG(ERROR) << "\t\t" << v << ": " << vpoint_[v];
                        show = false;
                    }
                    return Face();
                }
#endif
                next_cache.emplace_back(NextCacheEntry(inner_prev, inner_next));
            }
            else needs_adjust[ii] = (out_halfedge(v) == inner_next);


            // set face handle
            set_face(halfedges[i], f);
        }



        // process next halfedge cache
        NextCache::const_iterator ncIt(next_cache.begin()), ncEnd(next_cache.end());
        for (; ncIt != ncEnd; ++ncIt)
        {
            set_next(ncIt->first, ncIt->second);
        }



        // adjust vertices' halfedge handle
        for (i=0; i<n; ++i)
        {
            if (needs_adjust[i])
            {
                adjust_outgoing_halfedge(vertices[i]);
            }
        }


        return f;
    }


    //-----------------------------------------------------------------------------


    unsigned int SurfaceMesh::valence(Vertex v) const
    {
        unsigned int count(0);

        VertexAroundVertexCirculator vvit = vertices(v);
        VertexAroundVertexCirculator vvend = vvit;
        if (vvit) do
        {
            ++count;
        } while (++vvit != vvend);

        return count;
    }


    //-----------------------------------------------------------------------------


    unsigned int SurfaceMesh::valence(Face f) const
    {
        unsigned int count(0);

        VertexAroundFaceCirculator fvit = vertices(f);
        VertexAroundFaceCirculator fvend = fvit;
        do {
            ++count;
        } while (++fvit != fvend);

        return count;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::is_closed() const
    {
        EdgeIterator eit=edges_begin(), eend=edges_end();
        for (; eit!=eend; ++eit)
            if (is_border(*eit))
                return false;
        return true;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::is_triangle_mesh() const
    {
        FaceIterator fit=faces_begin(), fend=faces_end();
        for (; fit!=fend; ++fit)
            if (valence(*fit) != 3)
                return false;

        return true;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::is_quad_mesh() const
    {
        for (auto f : faces())
            if (valence(f) != 4)
                return false;
        return true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::triangulate()
    {
        /* The iterators will stay valid, even though new faces are added,
         because they are now implemented index-based instead of
         pointer-based.
         */
        FaceIterator fit=faces_begin(), fend=faces_end();
        for (; fit!=fend; ++fit)
            triangulate(*fit);
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::reverse_orientation() {
        auto reverse_orientation = [](SurfaceMesh::Halfedge first, SurfaceMesh &mesh) -> void {
            if (first == SurfaceMesh::Halfedge())
                return;
            SurfaceMesh::Halfedge last = first;
            SurfaceMesh::Halfedge prev = first;
            SurfaceMesh::Halfedge start = first;
            first = mesh.next(first);
            SurfaceMesh::Vertex new_v = mesh.target(start);
            while (first != last) {
                SurfaceMesh::Vertex tmp_v = mesh.target(first);
                mesh.set_target(first, new_v);
                mesh.set_out_halfedge(new_v, first);
                new_v = tmp_v;
                SurfaceMesh::Halfedge n = mesh.next(first);
                mesh.set_next(first, prev);
                prev = first;
                first = n;
            }
            mesh.set_target(start, new_v);
            mesh.set_out_halfedge(new_v, start);
            mesh.set_next(start, prev);
        };

        for (auto f : faces())
            reverse_orientation(halfedge(f), *this);

        // Note: A border edge is now parallel to its opposite edge.
        // We scan all border edges for this property. If it holds, we reorient the associated hole and search
        // again until no border edge with that property exists any longer. Then, all holes are reoriented.
        for (auto h : halfedges()) {
            if (is_border(h) && target(h) == target(opposite(h)))
                reverse_orientation(h, *this);
        }
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::triangulate(Face f)
    {
        /*
         Split an arbitrary face into triangles by connecting
         each vertex of fh after its second to vh.

         - fh will remain valid (it will become one of the
         triangles)
         - the halfedge handles of the new triangles will
         point to the old halfedges
         */

        Halfedge base_h  = halfedge(f);
        Vertex   start_v = source(base_h);
        Halfedge next_h  = next(base_h);

        while (target(next(next_h)) != start_v)
        {
            Halfedge next_next_h(next(next_h));

            Face new_f = new_face();
            set_halfedge(new_f, base_h);

            Halfedge new_h = new_edge(target(next_h), start_v);

            set_next(base_h, next_h);
            set_next(next_h, new_h);
            set_next(new_h,  base_h);

            set_face(base_h, new_f);
            set_face(next_h, new_f);
            set_face(new_h,  new_f);

            base_h = opposite(new_h);
            next_h = next_next_h;
        }
        set_halfedge(f, base_h);  //the last face takes the handle _fh

        set_next(base_h, next_h);
        set_next(next(next_h), base_h);

        set_face(base_h, f);
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::update_face_normals()
    {
        if (!fnormal_)
            fnormal_ = face_property<vec3>("f:normal");

        FaceIterator fit, fend=faces_end();

        int num_degenerate = 0;
        for (fit=faces_begin(); fit!=fend; ++fit) {
            if (is_degenerate(*fit)) {
                ++num_degenerate;
                fnormal_[*fit] = vec3(0, 0, 1);
            } else
                fnormal_[*fit] = compute_face_normal(*fit);
        }

        if (num_degenerate > 0)
            LOG(WARNING) << "model has " << num_degenerate << " degenerate faces";
    }


    //-----------------------------------------------------------------------------


    vec3 SurfaceMesh::compute_face_normal(Face f) const
    {
        Halfedge h = halfedge(f);
        Halfedge hend = h;

        vec3 p0 = vpoint_[target(h)];
        h = next(h);
        vec3 p1 = vpoint_[target(h)];
        h = next(h);
        vec3 p2 = vpoint_[target(h)];

        if (next(h) == hend) // face is a triangle
        {
            return cross(p2-=p1, p0-=p1).normalize();
        }

        else // face is a general polygon
        {
            vec3 n(0,0,0);

            hend = h;
            do
            {
#if 0
                // I believe this function is not robust for some concave polygons.
                if (distance2(p2, p1) < min<float>() || distance2(p0, p1) < min<float>() || distance2(p2, p0) < min<float>())
                    LOG(WARNING) << "degenarate case encountered";
                else
                    n += cross(p2 - p1, p0 - p1);
#else
                // This seems to be a robust solution: 
                //   - Choose any point C near the polygon(any vertex or mass center).
                //   - Sum cross products(P[i] - C) x (P[i + 1] - C) for all i(including last and first points pair).
                //   - Normalize the sum vector.
                // Note that after step 2 you have a vector which has normal direction with proper orientation, and its magnitude is 2 S, 
                // where S is the area of your polygon. That's why it should work unless your polygon has zero or almost zero area. 
                // By the way, point C is used here only to make calculation a bit more precise for small polygons located far from the origin.
                // You can choose C = (0, 0, 0), efficiently removing it from calculations.
                if (distance2(p0, p1) < min<float>())
                    LOG(WARNING) << "0-length edge encountered";
                else
                    n += cross(p0, p1); // C = (0, 0, 0)
#endif
                h  = next(h);
                p0 = p1;
                p1 = p2;
                p2 = vpoint_[target(h)];
            }
            while (h != hend);

            return n.normalize();
        }
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::update_vertex_normals()
    {
        if (!vnormal_)
            vnormal_ = vertex_property<vec3>("v:normal");

        VertexIterator vit, vend=vertices_end();

#if 0   // not stable for concave vertices
        for (vit=vertices_begin(); vit!=vend; ++vit)
            vnormal_[*vit] = compute_vertex_normal(*vit);
#else // the angle-weighted average of incident face average

        auto angle_weighted_face_normals = [this](Vertex v) -> vec3 {
            vec3     nn(0,0,0);
            Halfedge  h = out_halfedge(v);

            if (h.is_valid())
            {
                const Halfedge hend = h;
                const vec3& p0 = position(v);

                vec3   n, p1, p2;
                float  cosine, angle, denom;

                do
                {
                    if (!is_border(h))
                    {
                        p1 = vpoint_[target(h)];
                        p1 -= p0;

                        p2 = vpoint_[source(prev(h))];
                        p2 -= p0;

                        // check whether we can robustly compute angle
                        denom = std::sqrt(dot(p1,p1)*dot(p2,p2));
                        if (denom > std::numeric_limits<float>::min())
                        {
                            cosine = dot(p1,p2) / denom;
                            if      (cosine < -1.0) cosine = -1.0;
                            else if (cosine >  1.0) cosine =  1.0;
                            angle = std::acos(cosine);

                            n   = fnormal_[face(h)];

                            // check whether normal is != 0
                            denom = norm(n);
                            if (denom > std::numeric_limits<float>::min())
                            {
                                n  *= angle/denom;
                                nn += n;
                            }
                        }
                    }

                    h  = next_around_source(h);
                }
                while (h != hend);

                nn.normalize();
            }

            return nn;
        };

        // always re-compute face normals
        update_face_normals();

        for (vit=vertices_begin(); vit!=vend; ++vit)
            vnormal_[*vit] = angle_weighted_face_normals(*vit);
#endif
    }


    //-----------------------------------------------------------------------------


    vec3 SurfaceMesh::compute_vertex_normal(Vertex v) const
    {
        vec3     nn(0,0,0);
        Halfedge  h = out_halfedge(v);

        if (h.is_valid())
        {
            const Halfedge hend = h;
            const vec3 p0 = vpoint_[v];

            vec3   n, p1, p2;
            float  cosine, angle, denom;

            do
            {
                if (!is_border(h))
                {
                    p1 = vpoint_[target(h)];
                    p1 -= p0;

                    p2 = vpoint_[source(prev(h))];
                    p2 -= p0;

                    // check whether we can robustly compute angle
                    denom = std::sqrt(dot(p1,p1)*dot(p2,p2));
                    if (denom > std::numeric_limits<float>::min())
                    {
                        cosine = dot(p1,p2) / denom;
                        if      (cosine < -1.0) cosine = -1.0;
                        else if (cosine >  1.0) cosine =  1.0;
                        angle = std::acos(cosine);

                        n   = cross(p1,p2);

                        // check whether normal is != 0
                        denom = norm(n);
                        if (denom > std::numeric_limits<float>::min())
                        {
                            n  *= angle/denom;
                            nn += n;
                        }
                    }
                }

                h  = next_around_source(h);
            }
            while (h != hend);

            nn.normalize();
        }

        return nn;
    }


    //-----------------------------------------------------------------------------


    float SurfaceMesh::edge_length(Edge e) const {
        return norm(vpoint_[vertex(e,0)] - vpoint_[vertex(e,1)]);
    }

    float SurfaceMesh::edge_length(Halfedge h) const {
        return norm(vpoint_[source(h)] - vpoint_[target(h)]);
    }

    //-----------------------------------------------------------------------------


    void SurfaceMesh::split(Face f, Vertex v)
    {
        /*
         Split an arbitrary face into triangles by connecting each vertex of fh to vh.
         - fh will remain valid (it will become one of the triangles)
         - the halfedge handles of the new triangles will point to the old halfedges
         */

        Halfedge hend = halfedge(f);
        Halfedge h    = next(hend);

        Halfedge hold = new_edge(target(hend), v);

        set_next(hend, hold);
        set_face(hold, f);

        hold = opposite(hold);

        while (h != hend)
        {
            Halfedge hnext = next(h);

            Face fnew = new_face();
            set_halfedge(fnew, h);

            Halfedge hnew = new_edge(target(h), v);

            set_next(hnew, hold);
            set_next(hold, h);
            set_next(h,    hnew);

            set_face(hnew, fnew);
            set_face(hold, fnew);
            set_face(h,    fnew);

            hold = opposite(hnew);

            h = hnext;
        }

        set_next(hold, hend);
        set_next(next(hend), hold);

        set_face(hold, f);

        set_out_halfedge(v, hold);
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Halfedge SurfaceMesh::split(Edge e, Vertex v)
    {
        Halfedge h0 = halfedge(e, 0);
        Halfedge o0 = halfedge(e, 1);

        Vertex   v2 = target(o0);

        Halfedge e1 = new_edge(v, v2);
        Halfedge t1 = opposite(e1);

        Face     f0 = face(h0);
        Face     f3 = face(o0);

        set_out_halfedge(v, h0);
        set_target(o0, v);

        if (!is_border(h0))
        {
            Halfedge h1 = next(h0);
            Halfedge h2 = next(h1);

            Vertex   v1 = target(h1);

            Halfedge e0 = new_edge(v, v1);
            Halfedge t0 = opposite(e0);

            Face f1 = new_face();
            set_halfedge(f0, h0);
            set_halfedge(f1, h2);

            set_face(h1, f0);
            set_face(t0, f0);
            set_face(h0, f0);

            set_face(h2, f1);
            set_face(t1, f1);
            set_face(e0, f1);

            set_next(h0, h1);
            set_next(h1, t0);
            set_next(t0, h0);

            set_next(e0, h2);
            set_next(h2, t1);
            set_next(t1, e0);
        }
        else
        {
            set_next(prev(h0), t1);
            set_next(t1, h0);
            // halfedge handle of _vh already is h0
        }


        if (!is_border(o0))
        {
            Halfedge o1 = next(o0);
            Halfedge o2 = next(o1);

            Vertex v3 = target(o1);

            Halfedge e2 = new_edge(v, v3);
            Halfedge t2 = opposite(e2);

            Face f2 = new_face();
            set_halfedge(f2, o1);
            set_halfedge(f3, o0);

            set_face(o1, f2);
            set_face(t2, f2);
            set_face(e1, f2);

            set_face(o2, f3);
            set_face(o0, f3);
            set_face(e2, f3);

            set_next(e1, o1);
            set_next(o1, t2);
            set_next(t2, e1);

            set_next(o0, e2);
            set_next(e2, o2);
            set_next(o2, o0);
        }
        else
        {
            set_next(e1, next(o0));
            set_next(o0, e1);
            set_out_halfedge(v, e1);
        }

        if (out_halfedge(v2) == h0)
            set_out_halfedge(v2, t1);

        return t1;
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Halfedge SurfaceMesh::insert_vertex(Halfedge h0, Vertex v)
    {
        // before:
        //
        // v0      h0       v2
        //  o--------------->o
        //   <---------------
        //         o0
        //
        // after:
        //
        // v0  h0   v   h1   v2
        //  o------>o------->o
        //   <------ <-------
        //     o0       o1

        Halfedge h2 = next(h0);
        Halfedge o0 = opposite(h0);
        Halfedge o2 = prev(o0);
        Vertex   v2 = target(h0);
        Face     fh = face(h0);
        Face     fo = face(o0);

        Halfedge h1 = new_edge(v, v2);
        Halfedge o1 = opposite(h1);

        // adjust halfedge connectivity
        set_next(h1, h2);
        set_next(h0, h1);
        set_target(h0, v);
        set_target(h1, v2);
        set_face(h1, fh);

        set_next(o1, o0);
        set_next(o2, o1);
        set_target(o1, v);
        set_face(o1, fo);

        // adjust vertex connectivity
        set_out_halfedge(v2, o1);
        adjust_outgoing_halfedge(v2);
        set_out_halfedge(v, h1);
        adjust_outgoing_halfedge(v);

        // adjust face connectivity
        if (fh.is_valid()) set_halfedge(fh, h0);
        if (fo.is_valid()) set_halfedge(fo, o1);

        return o1;
    }


    //-----------------------------------------------------------------------------


    SurfaceMesh::Halfedge SurfaceMesh::insert_edge(Halfedge h0, Halfedge h1)
    {
        assert(face(h0) == face(h1));
        assert(face(h0).is_valid());

        Vertex   v0 = target(h0);
        Vertex   v1 = target(h1);

        Halfedge h2 = next(h0);
        Halfedge h3 = next(h1);

        Halfedge h4 = new_edge(v0, v1);
        Halfedge h5 = opposite(h4);

        Face     f0 = face(h0);
        Face     f1 = new_face();

        set_halfedge(f0, h0);
        set_halfedge(f1, h1);

        set_next(h0, h4);
        set_next(h4, h3);
        set_face(h4, f0);

        set_next(h1, h5);
        set_next(h5, h2);
        Halfedge h = h2;
        do
        {
            set_face(h, f1);
            h = next(h);
        }
        while (h != h2);

        return h4;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::is_flip_ok(Edge e) const
    {
        // boundary edges cannot be flipped
        if (is_border(e)) return false;

        // check if the flipped edge is already present in the mesh

        Halfedge h0 = halfedge(e, 0);
        Halfedge h1 = halfedge(e, 1);

        Vertex v0 = target(next(h0));
        Vertex v1 = target(next(h1));

        if (v0 == v1)   // this is generally a bad sign !!!
            return false;

        if (find_halfedge(v0, v1).is_valid())
            return false;

        return true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::flip(Edge e)
    {
        // CAUTION : Flipping a halfedge may result in
        // a non-manifold mesh, hence check for yourself
        // whether this operation is allowed or not!

        //let's make it sure it is actually checked
        assert(is_flip_ok(e));

        Halfedge a0 = halfedge(e, 0);
        Halfedge b0 = halfedge(e, 1);

        Halfedge a1 = next(a0);
        Halfedge a2 = next(a1);

        Halfedge b1 = next(b0);
        Halfedge b2 = next(b1);

        Vertex   va0 = target(a0);
        Vertex   va1 = target(a1);

        Vertex   vb0 = target(b0);
        Vertex   vb1 = target(b1);

        Face     fa  = face(a0);
        Face     fb  = face(b0);

        set_target(a0, va1);
        set_target(b0, vb1);

        set_next(a0, a2);
        set_next(a2, b1);
        set_next(b1, a0);

        set_next(b0, b2);
        set_next(b2, a1);
        set_next(a1, b0);

        set_face(a1, fb);
        set_face(b1, fa);

        set_halfedge(fa, a0);
        set_halfedge(fb, b0);

        if (out_halfedge(va0) == b0)
            set_out_halfedge(va0, a1);
        if (out_halfedge(vb0) == a0)
            set_out_halfedge(vb0, b1);
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::can_merge_vertices(Halfedge h0, Halfedge h1) const {
        // It's OK if they are already the same!
        if (target(h0) == target(h1))
            return true;

        Halfedge cir_h0 = h0;
        do {
            // Number of potential opposites half_edges (should not be greater than 1)
            int nb_common = 0;
            Halfedge hh0 = opposite(cir_h0);
            Halfedge cir_h1 = h1;
            do {
                Halfedge hh1 = opposite(cir_h1);
                if (target(hh0) == target(hh1) ||
                    (target(hh0) == source(h0) && target(hh1) == source(h1)) ||
                    (target(hh0) == source(h1) && target(hh1) == source(h0))
                    )
                {
                    if ((is_border(opposite(hh0)) && is_border(hh1)) ||
                        (is_border(hh0) && is_border(opposite(hh1)))) {
                        // Found a potential opposite edge.
                        nb_common++;
                    } else {
                        // Potential opposite edge not on the border.
                        return false;
                    }
                }
                cir_h1 = prev(opposite(cir_h1));
            } while (cir_h1 != h1);
            if (nb_common > 1) {
                return false;
            }
            cir_h0 = prev(opposite(cir_h0));
        } while (cir_h0 != h0);
        return true;
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::is_stitch_ok(Halfedge h0, Halfedge h1) {
        // check if both halfedges are on the border.
        if (!is_border(h0) || !is_border(h1)) {
            return false;
        }

        // the two halfedges must point in reverse directions
        if (edge_length(edge(h0)) > 1e-6 && edge_length(edge(h1)) > 1e-6) {
            auto s0 = source(h0);
            auto t0 = target(h0);
            auto s1 = source(h1);
            auto t1 = target(h1);
            const auto dir0 = position(t0) - position(s0);
            const auto dir1 = position(t1) - position(s1);
            if (dot(dir0, dir1) > 0)
                return false;
        }

        // we cannot glue two halfedges on a same face
        const auto opp_h0 = opposite(h0);
        const auto opp_h1 = opposite(h1);
        if (face(opp_h0) == face(opp_h1))
            return false;

        // don't merge two vertices on a same halfedge
        if (find_halfedge(target(h0), source(h1)).is_valid() ||
            find_halfedge(source(h0), target(h1)).is_valid()) {
            return false;
        }

        if (!can_merge_vertices(h0, opp_h1) ||
            !can_merge_vertices(h1, opp_h0)) {
            return false;
        }

        return true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::stitch(Halfedge h0, Halfedge h1) {
        // CAUTION : Stitching two halfedges may result in a non-manifold mesh, hence check for yourself
        // whether this operation is allowed or not!

        //let's make it sure it is actually checked
        assert(is_stitch_ok(h0, h1));

        // the new position of the end points
        auto org0 = source(h0);
        auto org1 = source(h1);
        auto dest0 = target(h0);
        auto dest1 = target(h1);
        const vec3& p_org0 = geom::barycenter(vpoint_[org0], vpoint_[dest1]);
        const vec3& p_org1 = geom::barycenter(vpoint_[dest0], vpoint_[org1]);

        Halfedge new_h0 = new_edge(org1, org0);
        Halfedge new_h1 = opposite(new_h0);
        vpoint_[org0] = p_org0;
        vpoint_[org1] = p_org1;

        set_target(new_h0, org0);
        set_target(new_h1, org1);

        auto op_h0 = opposite(h0);
        auto op_h1 = opposite(h1);
        set_target(prev(op_h0), org1);
        set_target(prev(op_h1), org0);

        if (out_halfedge(org0) == h0)
            set_out_halfedge(org0, new_h1);
        if (out_halfedge(org1) == h1)
            set_out_halfedge(org1, new_h0);

        // set face
        auto f0 = face(op_h0);
        set_face(new_h0, f0);
        set_halfedge(f0, new_h0);
        auto f1 = face(op_h1);
        set_face(new_h1, f1);
        set_halfedge(f1, new_h1);

        auto locked = get_vertex_property<bool>("v:locked");
        if (locked) {
            if (locked[dest1])
                locked[org0] = true;
            if (locked[dest0])
                locked[org1] = true;
        }

        auto set_vertex_on_orbit = [](SurfaceMesh* mesh, Halfedge h, Vertex v) ->void {
            Halfedge it = h ;
            do {
                mesh->set_target(it, v);
                it = mesh->prev(mesh->opposite(it));
            } while(it != h) ;
        };

        if (org0 != dest1) {
            set_vertex_on_orbit(this, h1, org0);
            if (!vdeleted_[dest1]) {
                vdeleted_[dest1] = true;
                set_out_halfedge(dest1, Halfedge());
                deleted_vertices_++;
                garbage_ = true;
            }
        }

        if (org1 != dest0) {
            set_vertex_on_orbit(this, h0, org1);
            if (!vdeleted_[dest0]) {
                vdeleted_[dest0] = true;
                set_out_halfedge(dest0, Halfedge());
                deleted_vertices_++;
                garbage_ = true;
            }
        }

        // set halfedge connections
        set_next(prev(op_h0), new_h0);
        set_next( new_h0, next(op_h0));
        set_next(prev(op_h1), new_h1);
        set_next( new_h1, next(op_h1));
        auto prev_h0 = prev(h0);
        if (is_border(prev_h0))
            set_next(prev_h0, next(h1));
        auto prev_h1 = prev(h1);
        if (is_border(prev_h1))
            set_next(prev_h1, next(h0));

        // mark the two edges deleted (done in garbage collection)
        auto e0 = edge(h0);
        if (!edeleted_[e0]) {
            edeleted_[e0] = true;
            deleted_edges_++;
            garbage_ = true;
        }
        auto e1 = edge(h1);
        if (!edeleted_[e1]) {
            edeleted_[e1] = true;
            deleted_edges_++;
            garbage_ = true;
        }
    }


    //-----------------------------------------------------------------------------


    bool SurfaceMesh::is_collapse_ok(Halfedge v0v1) const
    {
        Halfedge  v1v0(opposite(v0v1));
        Vertex    v0(target(v1v0));
        Vertex    v1(target(v0v1));
        Vertex    vv, vl, vr;
        Halfedge  h1, h2;


        // the edges v1-vl and vl-v0 must not be both boundary edges
        if (!is_border(v0v1))
        {
            vl = target(next(v0v1));
            h1 = next(v0v1);
            h2 = next(h1);
            if (is_border(opposite(h1)) && is_border(opposite(h2)))
                return false;
        }


        // the edges v0-vr and vr-v1 must not be both boundary edges
        if (!is_border(v1v0))
        {
            vr = target(next(v1v0));
            h1 = next(v1v0);
            h2 = next(h1);
            if (is_border(opposite(h1)) && is_border(opposite(h2)))
                return false;
        }


        // if vl and vr are equal or both invalid -> fail
        if (vl == vr) return false;


        // edge between two boundary vertices should be a boundary edge
        if ( is_border(v0) && is_border(v1) &&
            !is_border(v0v1) && !is_border(v1v0))
            return false;


        // test intersection of the one-rings of v0 and v1
        VertexAroundVertexCirculator vv_it, vv_end;
        vv_it = vv_end = vertices(v0);
        do
        {
            vv = *vv_it;
            if (vv != v1 && vv != vl && vv != vr)
                if (find_halfedge(vv, v1).is_valid())
                    return false;
        }
        while (++vv_it != vv_end);


        // passed all tests
        return true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::collapse(Halfedge h)
    {
        //let's make it sure it is actually checked
        assert(is_collapse_ok(h));

        Halfedge h0 = h;
        Halfedge h1 = prev(h0);
        Halfedge o0 = opposite(h0);
        Halfedge o1 = next(o0);

        // remove edge
        remove_edge(h0);

        // remove loops
        if (next(next(h1)) == h1)
            remove_loop(h1);
        if (next(next(o1)) == o1)
            remove_loop(o1);
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::remove_edge(Halfedge h)
    {
        Halfedge  hn = next(h);
        Halfedge  hp = prev(h);

        Halfedge  o  = opposite(h);
        Halfedge  on = next(o);
        Halfedge  op = prev(o);

        Face      fh = face(h);
        Face      fo = face(o);

        Vertex    vh = target(h);
        Vertex    vo = target(o);



        // halfedge -> vertex
        HalfedgeAroundVertexCirculator vh_it, vh_end;
        vh_it = vh_end = halfedges(vo);
        do
        {
            set_target(opposite(*vh_it), vh);
        }
        while (++vh_it != vh_end);


        // halfedge -> halfedge
        set_next(hp, hn);
        set_next(op, on);


        // face -> halfedge
        if (fh.is_valid())  set_halfedge(fh, hn);
        if (fo.is_valid())  set_halfedge(fo, on);


        // vertex -> halfedge
        if (out_halfedge(vh) == o)  set_out_halfedge(vh, hn);
        adjust_outgoing_halfedge(vh);
        set_out_halfedge(vo, Halfedge());


        // delete stuff
        if (!vdeleted_) vdeleted_ = vertex_property<bool>("v:deleted", false);
        if (!edeleted_) edeleted_ = edge_property<bool>("e:deleted", false);
        vdeleted_[vo]      = true; ++deleted_vertices_;
        edeleted_[edge(h)] = true; ++deleted_edges_;
        garbage_ = true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::remove_loop(Halfedge h)
    {
        Halfedge  h0 = h;
        Halfedge  h1 = next(h0);

        Halfedge  o0 = opposite(h0);
        Halfedge  o1 = opposite(h1);

        Vertex    v0 = target(h0);
        Vertex    v1 = target(h1);

        Face      fh = face(h0);
        Face      fo = face(o0);



        // is it a loop ?
        assert ((next(h1) == h0) && (h1 != o0));


        // halfedge -> halfedge
        set_next(h1, next(o0));
        set_next(prev(o0), h1);


        // halfedge -> face
        set_face(h1, fo);


        // vertex -> halfedge
        set_out_halfedge(v0, h1);  adjust_outgoing_halfedge(v0);
        set_out_halfedge(v1, o1);  adjust_outgoing_halfedge(v1);


        // face -> halfedge
        if (fo.is_valid() && halfedge(fo) == o0)
            set_halfedge(fo, h1);


        // delete stuff
        if (!edeleted_) edeleted_ = edge_property<bool>("e:deleted", false);
        if (!fdeleted_) fdeleted_ = face_property<bool>("f:deleted", false);
        if (fh.is_valid()) { fdeleted_[fh] = true; ++deleted_faces_; }
        edeleted_[edge(h0)] = true; ++deleted_edges_;
        garbage_ = true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::delete_vertex(Vertex v)
    {
        if (vdeleted_[v])  return;

        // collect incident faces
        std::vector<Face> incident_faces;
        incident_faces.reserve(6);

        FaceAroundVertexCirculator fc, fc_end;
        fc = fc_end = faces(v);

        if (fc)
        do
        {
            incident_faces.push_back(*fc);
        } while (++fc != fc_end);

        // delete incident faces
        for (auto f : incident_faces)
            delete_face(f);

        // mark v as deleted if not yet done by delete_face()
        if (!vdeleted_[v])
        {
            vdeleted_[v] = true;
            deleted_vertices_++;
            garbage_ = true;
        }
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::delete_edge(Edge e)
    {
        if (edeleted_[e])  return;

        Face f0 = face(halfedge(e, 0));
        Face f1 = face(halfedge(e, 1));

        if (f0.is_valid()) delete_face(f0);
        if (f1.is_valid()) delete_face(f1);
    }


    //-----------------------------------------------------------------------------

    void SurfaceMesh::delete_face(Face f)
    {
        if (fdeleted_[f])  return;

        // mark face deleted
        if (!fdeleted_[f])
        {
            fdeleted_[f] = true;
            deleted_faces_++;
        }

        // boundary edges of face f to be deleted
        std::vector<Edge> deleted_edges;
        deleted_edges.reserve(3);


        // vertices of face f for updating their outgoing halfedge
        std::vector<Vertex> vertices;
        vertices.reserve(3);


        // for all halfedges of face f do:
        //   1) invalidate face handle.
        //   2) collect all boundary halfedges, set them deleted
        //   3) store vertex handles
        HalfedgeAroundFaceCirculator hc, hc_end;
        hc = hc_end = halfedges(f);

        do
        {
            set_face(*hc, Face());

            if (is_border(opposite(*hc)))
                deleted_edges.push_back(edge(*hc));

            vertices.push_back(target(*hc));

        } while (++hc != hc_end);


        // delete all collected (half)edges
        // delete isolated vertices
        if (!deleted_edges.empty())
        {
            auto del_it(deleted_edges.begin()), del_end(deleted_edges.end());

            Halfedge h0, h1, next0, next1, prev0, prev1;
            Vertex   v0, v1;

            for (; del_it!=del_end; ++del_it)
            {
                h0    = halfedge(*del_it, 0);
                v0    = target(h0);
                next0 = next(h0);
                prev0 = prev(h0);

                h1    = halfedge(*del_it, 1);
                v1    = target(h1);
                next1 = next(h1);
                prev1 = prev(h1);

                // adjust next and prev handles
                set_next(prev0, next1);
                set_next(prev1, next0);

                // mark edge deleted
                if (!edeleted_[*del_it])
                {
                    edeleted_[*del_it] = true;
                    deleted_edges_++;
                }

                // update v0
                if (out_halfedge(v0) == h1)
                {
                    if (next0 == h1)
                    {
                        if (!vdeleted_[v0])
                        {
                            vdeleted_[v0] = true;
                            deleted_vertices_++;
                        }
                    }
                    else set_out_halfedge(v0, next0);
                }

                // update v1
                if (out_halfedge(v1) == h0)
                {
                    if (next1 == h0)
                    {
                        if (!vdeleted_[v1])
                        {
                            vdeleted_[v1] = true;
                            deleted_vertices_++;
                        }
                    }
                    else  set_out_halfedge(v1, next1);
                }
            }
        }


        // update outgoing halfedge handles of remaining vertices
        for (auto v : vertices)
            adjust_outgoing_halfedge(v);

        garbage_ = true;
    }


    //-----------------------------------------------------------------------------


    void SurfaceMesh::collect_garbage()
    {
        if (!garbage_)
            return;

        int  i, i0, i1,
        nV(static_cast<int>(vertices_size())),
        nE(static_cast<int>(edges_size())),
        nH(static_cast<int>(halfedges_size())),
        nF(static_cast<int>(faces_size()));

        Vertex    v;
        Halfedge  h;
        Face      f;

        // setup handle mapping
        VertexProperty<Vertex>      vmap = add_vertex_property<Vertex>("v:garbage-collection");
        HalfedgeProperty<Halfedge>  hmap = add_halfedge_property<Halfedge>("h:garbage-collection");
        FaceProperty<Face>          fmap = add_face_property<Face>("f:garbage-collection");
        for (i=0; i<nV; ++i)
            vmap[Vertex(i)] = Vertex(i);
        for (i=0; i<nH; ++i)
            hmap[Halfedge(i)] = Halfedge(i);
        for (i=0; i<nF; ++i)
            fmap[Face(i)] = Face(i);



        // remove deleted vertices
        if (nV > 0)
        {
            i0=0;  i1=nV-1;

            while (true)
            {
                // find first deleted and last un-deleted
                while (!vdeleted_[Vertex(i0)] && i0 < i1)  ++i0;
                while ( vdeleted_[Vertex(i1)] && i0 < i1)  --i1;
                if (i0 >= i1) break;

                // swap
                vprops_.swap(i0, i1);
            }

            // remember new size
            nV = vdeleted_[Vertex(i0)] ? i0 : i0+1;
        }


        // remove deleted edges
        if (nE > 0)
        {
            i0=0;  i1=nE-1;

            while (true)
            {
                // find first deleted and last un-deleted
                while (!edeleted_[Edge(i0)] && i0 < i1)  ++i0;
                while ( edeleted_[Edge(i1)] && i0 < i1)  --i1;
                if (i0 >= i1) break;

                // swap
                eprops_.swap(i0, i1);
                hprops_.swap(2*i0,   2*i1);
                hprops_.swap(2*i0+1, 2*i1+1);
            }

            // remember new size
            nE = edeleted_[Edge(i0)] ? i0 : i0+1;
            nH = 2*nE;
        }


        // remove deleted faces
        if (nF > 0)
        {
            i0=0;  i1=nF-1;

            while (true)
            {
                // find 1st deleted and last un-deleted
                while (!fdeleted_[Face(i0)] && i0 < i1)  ++i0;
                while ( fdeleted_[Face(i1)] && i0 < i1)  --i1;
                if (i0 >= i1) break;

                // swap
                fprops_.swap(i0, i1);
            }

            // remember new size
            nF = fdeleted_[Face(i0)] ? i0 : i0+1;
        }


        // update vertex connectivity
        for (i=0; i<nV; ++i)
        {
            v = Vertex(i);
            if (!is_isolated(v))
                set_out_halfedge(v, hmap[out_halfedge(v)]);
        }


        // update halfedge connectivity
        for (i=0; i<nH; ++i)
        {
            h = Halfedge(i);
            set_target(h, vmap[target(h)]);
            set_next(h, hmap[next(h)]);
            if (!is_border(h))
                set_face(h, fmap[face(h)]);
        }


        // update handles of faces
        for (i=0; i<nF; ++i)
        {
            f = Face(i);
            set_halfedge(f, hmap[halfedge(f)]);
        }


        // remove handle maps
        remove_vertex_property(vmap);
        remove_halfedge_property(hmap);
        remove_face_property(fmap);

        // finally resize arrays
        vprops_.resize(nV); vprops_.shrink_to_fit();
        hprops_.resize(nH); hprops_.shrink_to_fit();
        eprops_.resize(nE); eprops_.shrink_to_fit();
        fprops_.resize(nF); fprops_.shrink_to_fit();

        deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
        garbage_ = false;

#if 1
        // [Liangliang]: It seems the outgoing halfedges of the vertices may be broken after garbage collection, e.g.,
        // the index of a vertex's outgoing halfedge may go out of range in some cases (e.g., after deleting faces).
        // The reason was that the mesh may have an invalid state when elements were marked deleted but still exist.
        // This can be easily fixed by assigning a correct outgoing halfedge to each vertex.
        adjust_outgoing_halfedges();
#endif

    }


    bool SurfaceMesh::is_degenerate(Face f) const {
        Halfedge h = halfedge(f);
        Halfedge hend = h;

        vec3 p0 = vpoint_[target(h)];
        h = next(h);
        vec3 p1 = vpoint_[target(h)];
        h = next(h);
        vec3 p2 = vpoint_[target(h)];

        if (next(h) == hend) // face is a triangle
        {
            auto d1 = p2-p1; auto len1 = d1.length();
            if (len1 < std::numeric_limits<float>::min())
                return true;
            auto d2 = p0-p1; auto len2 = d2.length();
            if (len2 < std::numeric_limits<float>::min())
                return true;

            auto angle = geom::angle(d1, d2);
            angle = geom::to_degrees(std::abs(angle));
            if (std::abs(angle) < std::numeric_limits<float>::min())
                return true;

            return false;
        }

        else // face is a general polygon
        {
            int num_good = 0;
            hend = h;
            do
            {
                auto d1 = p2-p1; auto len1 = d1.length();
                if (len1 < std::numeric_limits<float>::min()) {
                    h  = next(h);
                    continue;
                }
                auto d2 = p0-p1; auto len2 = d2.length();
                if (len2 < std::numeric_limits<float>::min()) {
                    h  = next(h);
                    continue;
                }

                auto angle = geom::angle(d1, d2);
                angle = geom::to_degrees(std::abs(angle));
                if (std::abs(angle) < std::numeric_limits<float>::min()) {
                    h  = next(h);
                    continue;
                }

                ++num_good;
                h  = next(h);
                p0 = p1;
                p1 = p2;
                p2 = vpoint_[target(h)];
            }
            while (h != hend);

            return (num_good == 0);
        }
    }


    bool SurfaceMesh::can_join_edges(Vertex v) const {
        if (valence(v) != 2) {
            return false;
        }
        Halfedge h = out_halfedge(v);
        if (!is_border(h) && valence(face(h)) < 4) {
            return false;
        }
        h = opposite(h);
        if (!is_border(h) && valence(face(h)) < 4) {
            return false;
        }
        return true;
    }


    bool SurfaceMesh::join_edges(Vertex vt) {
        if (!can_join_edges(vt)) {
            return false;
        }

        auto hh = out_halfedge(vt);
        remove_edge(hh);
        return true;
    }


} // namespace easy3d
