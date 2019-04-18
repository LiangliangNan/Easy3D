/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications.
 *		- Surface_mesh (version 1.1)
 * The original code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *
 *----------------------------------------------------------*/


//== INCLUDES =================================================================


#include <easy3d/core/surface_mesh.h>

#include <cmath>


//== NAMESPACE ================================================================


namespace easy3d {


//== IMPLEMENTATION ===========================================================


SurfaceMesh::
SurfaceMesh()
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


SurfaceMesh::
~SurfaceMesh()
{
}


//-----------------------------------------------------------------------------


SurfaceMesh&
SurfaceMesh::
operator=(const SurfaceMesh& rhs)
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


SurfaceMesh&
SurfaceMesh::
assign(const SurfaceMesh& rhs)
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


void
SurfaceMesh::
clear()
{
    vprops_.resize(0);
    hprops_.resize(0);
    eprops_.resize(0);
    fprops_.resize(0);
    mprops_.resize(0);

    free_memory();

    deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
    garbage_ = false;
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
free_memory()
{
    vprops_.free_memory();
    hprops_.free_memory();
    eprops_.free_memory();
    fprops_.free_memory();
    mprops_.free_memory();
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
reserve(unsigned int nvertices,
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


void
SurfaceMesh::
property_stats() const
{
    std::vector<std::string> props;

    std::cout << "vertex properties:\n";
    props = vertex_properties();
    for (unsigned int i=0; i<props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;

    std::cout << "halfedge properties:\n";
    props = halfedge_properties();
    for (unsigned int i=0; i<props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;

    std::cout << "edge properties:\n";
    props = edge_properties();
    for (unsigned int i=0; i<props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;

    std::cout << "face properties:\n";
    props = face_properties();
    for (unsigned int i=0; i<props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;

	std::cout << "model properties:\n";
	props = model_properties();
	for (unsigned int i = 0; i < props.size(); ++i)
		std::cout << "\t" << props[i] << std::endl;
}


//-----------------------------------------------------------------------------


SurfaceMesh::Vertex
SurfaceMesh::
add_vertex(const vec3& p)
{
    Vertex v = new_vertex();
    vpoint_[v] = p;
    return v;
}


//-----------------------------------------------------------------------------


SurfaceMesh::Halfedge
SurfaceMesh::
find_halfedge(Vertex start, Vertex end) const
{
    assert(is_valid(start) && is_valid(end));

    Halfedge h  = halfedge(start);
    const Halfedge hh = h;

    if (h.is_valid())
    {
        do
        {
            if (to_vertex(h) == end)
                return h;
            h = cw_rotated_halfedge(h);
        }
        while (h != hh);
    }

    return Halfedge();
}


//-----------------------------------------------------------------------------


SurfaceMesh::Edge
SurfaceMesh::
find_edge(Vertex a, Vertex b) const
{
    Halfedge h = find_halfedge(a,b);
    return h.is_valid() ? edge(h) : Edge();
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
adjust_outgoing_halfedge(Vertex v)
{
    Halfedge h  = halfedge(v);
    const Halfedge hh = h;

    if (h.is_valid())
    {
        do
        {
            if (is_boundary(h))
            {
                set_halfedge(v, h);
                return;
            }
            h = cw_rotated_halfedge(h);
        }
        while (h != hh);
    }
}


//-----------------------------------------------------------------------------


SurfaceMesh::Face
SurfaceMesh::
add_triangle(Vertex v0, Vertex v1, Vertex v2)
{
    add_face_vertices_.resize(3);
    add_face_vertices_[0] = v0;
    add_face_vertices_[1] = v1;
    add_face_vertices_[2] = v2;
    return add_face(add_face_vertices_);
}


//-----------------------------------------------------------------------------


SurfaceMesh::Face
SurfaceMesh::
add_quad(Vertex v0, Vertex v1, Vertex v2, Vertex v3)
{
    add_face_vertices_.resize(4);
    add_face_vertices_[0] = v0;
    add_face_vertices_[1] = v1;
    add_face_vertices_[2] = v2;
    add_face_vertices_[3] = v3;
    return add_face(add_face_vertices_);
}


//-----------------------------------------------------------------------------


SurfaceMesh::Face
SurfaceMesh::
add_face(const std::vector<Vertex>& vertices)
{
    const std::size_t n(vertices.size());
    assert (n > 2);

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
        if ( !is_boundary(vertices[i]) )
        {
            std::cerr << "SurfaceMesh::add_face: complex vertex\n";
            return Face();
        }

        halfedges[i] = find_halfedge(vertices[i], vertices[ii]);
        is_new[i]    = !halfedges[i].is_valid();

        if (!is_new[i] && !is_boundary(halfedges[i]))
        {
            std::cerr << "SurfaceMesh::add_face: complex edge\n";
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

            if (next_halfedge(inner_prev) != inner_next)
            {
                // here comes the ugly part... we have to relink a whole patch

                // search a free gap
                // free gap will be between boundary_prev and boundary_next
                outer_prev = opposite_halfedge(inner_next);
                outer_next = opposite_halfedge(inner_prev);
                boundary_prev = outer_prev;
                do
                {
                    boundary_prev = opposite_halfedge(next_halfedge(boundary_prev));
                }
                while (!is_boundary(boundary_prev) || boundary_prev==inner_prev);
                boundary_next = next_halfedge(boundary_prev);
                assert(is_boundary(boundary_prev));
                assert(is_boundary(boundary_next));


                // ok ?
                if (boundary_next == inner_next)
                {
                    std::cerr << "SurfaceMesh::add_face: patch re-linking failed\n";
                    return Face();
                }

                // other halfedges' handles
                patch_start = next_halfedge(inner_prev);
                patch_end   = prev_halfedge(inner_next);

                // relink
                next_cache.push_back(NextCacheEntry(boundary_prev, patch_start));
                next_cache.push_back(NextCacheEntry(patch_end, boundary_next));
                next_cache.push_back(NextCacheEntry(inner_prev, inner_next));
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
            outer_prev = opposite_halfedge(inner_next);
            outer_next = opposite_halfedge(inner_prev);

            // set outer links
            switch (id)
            {
                case 1: // prev is new, next is old
                    boundary_prev = prev_halfedge(inner_next);
                    next_cache.push_back(NextCacheEntry(boundary_prev, outer_next));
                    set_halfedge(v, outer_next);
                    break;

                case 2: // next is new, prev is old
                    boundary_next = next_halfedge(inner_prev);
                    next_cache.push_back(NextCacheEntry(outer_prev, boundary_next));
                    set_halfedge(v, boundary_next);
                    break;

                case 3: // both are new
                    if (!halfedge(v).is_valid())
                    {
                        set_halfedge(v, outer_next);
                        next_cache.push_back(NextCacheEntry(outer_prev, outer_next));
                    }
                    else
                    {
                        boundary_next = halfedge(v);
                        boundary_prev = prev_halfedge(boundary_next);
                        next_cache.push_back(NextCacheEntry(boundary_prev, outer_next));
                        next_cache.push_back(NextCacheEntry(outer_prev, boundary_next));
                    }
                    break;
            }

            // set inner link
            next_cache.push_back(NextCacheEntry(inner_prev, inner_next));
        }
        else needs_adjust[ii] = (halfedge(v) == inner_next);


        // set face handle
        set_face(halfedges[i], f);
    }



    // process next halfedge cache
    NextCache::const_iterator ncIt(next_cache.begin()), ncEnd(next_cache.end());
    for (; ncIt != ncEnd; ++ncIt)
    {
        set_next_halfedge(ncIt->first, ncIt->second);
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


unsigned int
SurfaceMesh::
valence(Vertex v) const
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


unsigned int
SurfaceMesh::
valence(Face f) const
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


bool
SurfaceMesh::
is_triangle_mesh() const
{
    FaceIterator fit=faces_begin(), fend=faces_end();
    for (; fit!=fend; ++fit)
        if (valence(*fit) != 3)
            return false;

    return true;
}


//-----------------------------------------------------------------------------


bool
SurfaceMesh::
is_quad_mesh() const
{
    FaceIterator fit=faces_begin(), fend=faces_end();
    for (; fit!=fend; ++fit)
        if (valence(*fit) != 4)
            return false;

    return true;
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
triangulate()
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


void
SurfaceMesh::
triangulate(Face f)
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
    Vertex   start_v = from_vertex(base_h);
    Halfedge next_h  = next_halfedge(base_h);

    while (to_vertex(next_halfedge(next_h)) != start_v)
    {
        Halfedge next_next_h(next_halfedge(next_h));

        Face new_f = new_face();
        set_halfedge(new_f, base_h);

        Halfedge new_h = new_edge(to_vertex(next_h), start_v);

        set_next_halfedge(base_h, next_h);
        set_next_halfedge(next_h, new_h);
        set_next_halfedge(new_h,  base_h);

        set_face(base_h, new_f);
        set_face(next_h, new_f);
        set_face(new_h,  new_f);

        base_h = opposite_halfedge(new_h);
        next_h = next_next_h;
    }
    set_halfedge(f, base_h);  //the last face takes the handle _fh

    set_next_halfedge(base_h, next_h);
    set_next_halfedge(next_halfedge(next_h), base_h);

    set_face(base_h, f);
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
update_face_normals()
{
    if (!fnormal_)
        fnormal_ = face_property<vec3>("f:normal");

    FaceIterator fit, fend=faces_end();

    for (fit=faces_begin(); fit!=fend; ++fit)
        fnormal_[*fit] = compute_face_normal(*fit);
}


//-----------------------------------------------------------------------------


vec3
SurfaceMesh::
compute_face_normal(Face f) const
{
    Halfedge h = halfedge(f);
    Halfedge hend = h;

    vec3 p0 = vpoint_[to_vertex(h)];
    h = next_halfedge(h);
    vec3 p1 = vpoint_[to_vertex(h)];
    h = next_halfedge(h);
    vec3 p2 = vpoint_[to_vertex(h)];

    if (next_halfedge(h) == hend) // face is a triangle
    {
        return cross(p2-=p1, p0-=p1).normalize();
    }

    else // face is a general polygon
    {
        vec3 n(0,0,0);

        hend = h;
        do
        {
            n += cross(p2-p1, p0-p1);
            h  = next_halfedge(h);
            p0 = p1;
            p1 = p2;
            p2 = vpoint_[to_vertex(h)];
        }
        while (h != hend);

        return n.normalize();
    }
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
update_vertex_normals()
{
    if (!vnormal_)
        vnormal_ = vertex_property<vec3>("v:normal");

    VertexIterator vit, vend=vertices_end();

    for (vit=vertices_begin(); vit!=vend; ++vit)
        vnormal_[*vit] = compute_vertex_normal(*vit);
}


//-----------------------------------------------------------------------------


vec3
SurfaceMesh::
compute_vertex_normal(Vertex v) const
{
    vec3     nn(0,0,0);
    Halfedge  h = halfedge(v);

    if (h.is_valid())
    {
        const Halfedge hend = h;
        const vec3 p0 = vpoint_[v];

        vec3   n, p1, p2;
        float  cosine, angle, denom;

        do
        {
            if (!is_boundary(h))
            {
                p1 = vpoint_[to_vertex(h)];
                p1 -= p0;

                p2 = vpoint_[from_vertex(prev_halfedge(h))];
                p2 -= p0;

                // check whether we can robustly compute angle
                denom = sqrt(dot(p1,p1)*dot(p2,p2));
                if (denom > std::numeric_limits<float>::min())
                {
                    cosine = dot(p1,p2) / denom;
                    if      (cosine < -1.0) cosine = -1.0;
                    else if (cosine >  1.0) cosine =  1.0;
                    angle = acos(cosine);

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

            h  = cw_rotated_halfedge(h);
        }
        while (h != hend);

        nn.normalize();
    }

    return nn;
}


//-----------------------------------------------------------------------------


float
SurfaceMesh::
edge_length(Edge e) const
{
    return norm(vpoint_[vertex(e,0)] - vpoint_[vertex(e,1)]);
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
split(Face f, Vertex v)
{
    /*
     Split an arbitrary face into triangles by connecting each vertex of fh to vh.
     - fh will remain valid (it will become one of the triangles)
     - the halfedge handles of the new triangles will point to the old halfeges
     */

    Halfedge hend = halfedge(f);
    Halfedge h    = next_halfedge(hend);

    Halfedge hold = new_edge(to_vertex(hend), v);

    set_next_halfedge(hend, hold);
    set_face(hold, f);

    hold = opposite_halfedge(hold);

    while (h != hend)
    {
        Halfedge hnext = next_halfedge(h);

        Face fnew = new_face();
        set_halfedge(fnew, h);

        Halfedge hnew = new_edge(to_vertex(h), v);

        set_next_halfedge(hnew, hold);
        set_next_halfedge(hold, h);
        set_next_halfedge(h,    hnew);

        set_face(hnew, fnew);
        set_face(hold, fnew);
        set_face(h,    fnew);

        hold = opposite_halfedge(hnew);

        h = hnext;
    }

    set_next_halfedge(hold, hend);
    set_next_halfedge(next_halfedge(hend), hold);

    set_face(hold, f);

    set_halfedge(v, hold);
}


//-----------------------------------------------------------------------------


SurfaceMesh::Halfedge
SurfaceMesh::
split(Edge e, Vertex v)
{
    Halfedge h0 = halfedge(e, 0);
    Halfedge o0 = halfedge(e, 1);

    Vertex   v2 = to_vertex(o0);

    Halfedge e1 = new_edge(v, v2);
    Halfedge t1 = opposite_halfedge(e1);

    Face     f0 = face(h0);
    Face     f3 = face(o0);

    set_halfedge(v, h0);
    set_vertex(o0, v);

    if (!is_boundary(h0))
    {
        Halfedge h1 = next_halfedge(h0);
        Halfedge h2 = next_halfedge(h1);

        Vertex   v1 = to_vertex(h1);

        Halfedge e0 = new_edge(v, v1);
        Halfedge t0 = opposite_halfedge(e0);

        Face f1 = new_face();
        set_halfedge(f0, h0);
        set_halfedge(f1, h2);

        set_face(h1, f0);
        set_face(t0, f0);
        set_face(h0, f0);

        set_face(h2, f1);
        set_face(t1, f1);
        set_face(e0, f1);

        set_next_halfedge(h0, h1);
        set_next_halfedge(h1, t0);
        set_next_halfedge(t0, h0);

        set_next_halfedge(e0, h2);
        set_next_halfedge(h2, t1);
        set_next_halfedge(t1, e0);
    }
    else
    {
        set_next_halfedge(prev_halfedge(h0), t1);
        set_next_halfedge(t1, h0);
        // halfedge handle of _vh already is h0
    }


    if (!is_boundary(o0))
    {
        Halfedge o1 = next_halfedge(o0);
        Halfedge o2 = next_halfedge(o1);

        Vertex v3 = to_vertex(o1);

        Halfedge e2 = new_edge(v, v3);
        Halfedge t2 = opposite_halfedge(e2);

        Face f2 = new_face();
        set_halfedge(f2, o1);
        set_halfedge(f3, o0);

        set_face(o1, f2);
        set_face(t2, f2);
        set_face(e1, f2);

        set_face(o2, f3);
        set_face(o0, f3);
        set_face(e2, f3);

        set_next_halfedge(e1, o1);
        set_next_halfedge(o1, t2);
        set_next_halfedge(t2, e1);

        set_next_halfedge(o0, e2);
        set_next_halfedge(e2, o2);
        set_next_halfedge(o2, o0);
    }
    else
    {
        set_next_halfedge(e1, next_halfedge(o0));
        set_next_halfedge(o0, e1);
        set_halfedge(v, e1);
    }

    if (halfedge(v2) == h0)
        set_halfedge(v2, t1);

    return t1;
}


//-----------------------------------------------------------------------------


SurfaceMesh::Halfedge
SurfaceMesh::
insert_vertex(Halfedge h0, Vertex v)
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

    Halfedge h2 = next_halfedge(h0);
    Halfedge o0 = opposite_halfedge(h0);
    Halfedge o2 = prev_halfedge(o0);
    Vertex   v2 = to_vertex(h0);
    Face     fh = face(h0);
    Face     fo = face(o0);

    Halfedge h1 = new_edge(v, v2);
    Halfedge o1 = opposite_halfedge(h1);

    // adjust halfedge connectivity
    set_next_halfedge(h1, h2);
    set_next_halfedge(h0, h1);
    set_vertex(h0, v);
    set_vertex(h1, v2);
    set_face(h1, fh);

    set_next_halfedge(o1, o0);
    set_next_halfedge(o2, o1);
    set_vertex(o1, v);
    set_face(o1, fo);

    // adjust vertex connectivity
    set_halfedge(v2, o1);
    adjust_outgoing_halfedge(v2);
    set_halfedge(v, h1);
    adjust_outgoing_halfedge(v);

    // adjust face connectivity
    if (fh.is_valid()) set_halfedge(fh, h0);
    if (fo.is_valid()) set_halfedge(fo, o1);

    return o1;
}


//-----------------------------------------------------------------------------


SurfaceMesh::Halfedge
SurfaceMesh::
insert_edge(Halfedge h0, Halfedge h1)
{
    assert(face(h0) == face(h1));
    assert(face(h0).is_valid());

    Vertex   v0 = to_vertex(h0);
    Vertex   v1 = to_vertex(h1);

    Halfedge h2 = next_halfedge(h0);
    Halfedge h3 = next_halfedge(h1);

    Halfedge h4 = new_edge(v0, v1);
    Halfedge h5 = opposite_halfedge(h4);

    Face     f0 = face(h0);
    Face     f1 = new_face();

    set_halfedge(f0, h0);
    set_halfedge(f1, h1);

    set_next_halfedge(h0, h4);
    set_next_halfedge(h4, h3);
    set_face(h4, f0);

    set_next_halfedge(h1, h5);
    set_next_halfedge(h5, h2);
    Halfedge h = h2;
    do
    {
        set_face(h, f1);
        h = next_halfedge(h);
    }
    while (h != h2);

    return h4;
}


//-----------------------------------------------------------------------------


bool
SurfaceMesh::
is_flip_ok(Edge e) const
{
    // boundary edges cannot be flipped
    if (is_boundary(e)) return false;

    // check if the flipped edge is already present in the mesh

    Halfedge h0 = halfedge(e, 0);
    Halfedge h1 = halfedge(e, 1);

    Vertex v0 = to_vertex(next_halfedge(h0));
    Vertex v1 = to_vertex(next_halfedge(h1));

    if (v0 == v1)   // this is generally a bad sign !!!
        return false;

    if (find_halfedge(v0, v1).is_valid())
        return false;

    return true;
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
flip(Edge e)
{
    // CAUTION : Flipping a halfedge may result in
    // a non-manifold mesh, hence check for yourself
    // whether this operation is allowed or not!

    //let's make it sure it is actually checked
    assert(is_flip_ok(e));

    Halfedge a0 = halfedge(e, 0);
    Halfedge b0 = halfedge(e, 1);

    Halfedge a1 = next_halfedge(a0);
    Halfedge a2 = next_halfedge(a1);

    Halfedge b1 = next_halfedge(b0);
    Halfedge b2 = next_halfedge(b1);

    Vertex   va0 = to_vertex(a0);
    Vertex   va1 = to_vertex(a1);

    Vertex   vb0 = to_vertex(b0);
    Vertex   vb1 = to_vertex(b1);

    Face     fa  = face(a0);
    Face     fb  = face(b0);

    set_vertex(a0, va1);
    set_vertex(b0, vb1);

    set_next_halfedge(a0, a2);
    set_next_halfedge(a2, b1);
    set_next_halfedge(b1, a0);

    set_next_halfedge(b0, b2);
    set_next_halfedge(b2, a1);
    set_next_halfedge(a1, b0);

    set_face(a1, fb);
    set_face(b1, fa);

    set_halfedge(fa, a0);
    set_halfedge(fb, b0);

    if (halfedge(va0) == b0)
        set_halfedge(va0, a1);
    if (halfedge(vb0) == a0)
        set_halfedge(vb0, b1);
}


//-----------------------------------------------------------------------------


bool
SurfaceMesh::
is_collapse_ok(Halfedge v0v1)
{
    Halfedge  v1v0(opposite_halfedge(v0v1));
    Vertex    v0(to_vertex(v1v0));
    Vertex    v1(to_vertex(v0v1));
    Vertex    vv, vl, vr;
    Halfedge  h1, h2;


    // the edges v1-vl and vl-v0 must not be both boundary edges
    if (!is_boundary(v0v1))
    {
        vl = to_vertex(next_halfedge(v0v1));
        h1 = next_halfedge(v0v1);
        h2 = next_halfedge(h1);
        if (is_boundary(opposite_halfedge(h1)) && is_boundary(opposite_halfedge(h2)))
            return false;
    }


    // the edges v0-vr and vr-v1 must not be both boundary edges
    if (!is_boundary(v1v0))
    {
        vr = to_vertex(next_halfedge(v1v0));
        h1 = next_halfedge(v1v0);
        h2 = next_halfedge(h1);
        if (is_boundary(opposite_halfedge(h1)) && is_boundary(opposite_halfedge(h2)))
            return false;
    }


    // if vl and vr are equal or both invalid -> fail
    if (vl == vr) return false;


    // edge between two boundary vertices should be a boundary edge
    if ( is_boundary(v0) && is_boundary(v1) &&
        !is_boundary(v0v1) && !is_boundary(v1v0))
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


void
SurfaceMesh::
collapse(Halfedge h)
{
    Halfedge h0 = h;
    Halfedge h1 = prev_halfedge(h0);
    Halfedge o0 = opposite_halfedge(h0);
    Halfedge o1 = next_halfedge(o0);

    // remove edge
    remove_edge(h0);

    // remove loops
    if (next_halfedge(next_halfedge(h1)) == h1)
        remove_loop(h1);
    if (next_halfedge(next_halfedge(o1)) == o1)
        remove_loop(o1);
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
remove_edge(Halfedge h)
{
    Halfedge  hn = next_halfedge(h);
    Halfedge  hp = prev_halfedge(h);

    Halfedge  o  = opposite_halfedge(h);
    Halfedge  on = next_halfedge(o);
    Halfedge  op = prev_halfedge(o);

    Face      fh = face(h);
    Face      fo = face(o);

    Vertex    vh = to_vertex(h);
    Vertex    vo = to_vertex(o);



    // halfedge -> vertex
    HalfedgeAroundVertexCirculator vh_it, vh_end;
    vh_it = vh_end = halfedges(vo);
    do
    {
        set_vertex(opposite_halfedge(*vh_it), vh);
    }
    while (++vh_it != vh_end);


    // halfedge -> halfedge
    set_next_halfedge(hp, hn);
    set_next_halfedge(op, on);


    // face -> halfedge
    if (fh.is_valid())  set_halfedge(fh, hn);
    if (fo.is_valid())  set_halfedge(fo, on);


    // vertex -> halfedge
    if (halfedge(vh) == o)  set_halfedge(vh, hn);
    adjust_outgoing_halfedge(vh);
    set_halfedge(vo, Halfedge());


    // delete stuff
    if (!vdeleted_) vdeleted_ = vertex_property<bool>("v:deleted", false);
    if (!edeleted_) edeleted_ = edge_property<bool>("e:deleted", false);
    vdeleted_[vo]      = true; ++deleted_vertices_;
    edeleted_[edge(h)] = true; ++deleted_edges_;
    garbage_ = true;
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
remove_loop(Halfedge h)
{
    Halfedge  h0 = h;
    Halfedge  h1 = next_halfedge(h0);

    Halfedge  o0 = opposite_halfedge(h0);
    Halfedge  o1 = opposite_halfedge(h1);

    Vertex    v0 = to_vertex(h0);
    Vertex    v1 = to_vertex(h1);

    Face      fh = face(h0);
    Face      fo = face(o0);



    // is it a loop ?
    assert ((next_halfedge(h1) == h0) && (h1 != o0));


    // halfedge -> halfedge
    set_next_halfedge(h1, next_halfedge(o0));
    set_next_halfedge(prev_halfedge(o0), h1);


    // halfedge -> face
    set_face(h1, fo);


    // vertex -> halfedge
    set_halfedge(v0, h1);  adjust_outgoing_halfedge(v0);
    set_halfedge(v1, o1);  adjust_outgoing_halfedge(v1);


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


void
SurfaceMesh::
delete_vertex(Vertex v)
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
    std::vector<Face>::iterator fit(incident_faces.begin()),
                                fend(incident_faces.end());

    for (; fit != fend; ++fit)
        delete_face(*fit);

    // mark v as deleted if not yet done by delete_face()
    if (!vdeleted_[v])
    {
        vdeleted_[v] = true;
        deleted_vertices_++;
        garbage_ = true;
    }
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
delete_edge(Edge e)
{
    if (edeleted_[e])  return;

    Face f0 = face(halfedge(e, 0));
    Face f1 = face(halfedge(e, 1));

    if (f0.is_valid()) delete_face(f0);
    if (f1.is_valid()) delete_face(f1);
}


//-----------------------------------------------------------------------------

void
SurfaceMesh::
delete_face(Face f)
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

        if (is_boundary(opposite_halfedge(*hc)))
            deleted_edges.push_back(edge(*hc));

        vertices.push_back(to_vertex(*hc));

    } while (++hc != hc_end);


    // delete all collected (half)edges
    // delete isolated vertices
    if (!deleted_edges.empty())
    {
        std::vector<Edge>::iterator del_it(deleted_edges.begin()),
                                    del_end(deleted_edges.end());

        Halfedge h0, h1, next0, next1, prev0, prev1;
        Vertex   v0, v1;

        for (; del_it!=del_end; ++del_it)
        {
            h0    = halfedge(*del_it, 0);
            v0    = to_vertex(h0);
            next0 = next_halfedge(h0);
            prev0 = prev_halfedge(h0);

            h1    = halfedge(*del_it, 1);
            v1    = to_vertex(h1);
            next1 = next_halfedge(h1);
            prev1 = prev_halfedge(h1);

            // adjust next and prev handles
            set_next_halfedge(prev0, next1);
            set_next_halfedge(prev1, next0);

            // mark edge deleted
            if (!edeleted_[*del_it])
            {
                edeleted_[*del_it] = true;
                deleted_edges_++;
            }

            // update v0
            if (halfedge(v0) == h1)
            {
                if (next0 == h1)
                {
                    if (!vdeleted_[v0])
                    {
                        vdeleted_[v0] = true;
                        deleted_vertices_++;
                    }
                }
                else set_halfedge(v0, next0);
            }

            // update v1
            if (halfedge(v1) == h0)
            {
                if (next1 == h0)
                {
                    if (!vdeleted_[v1])
                    {
                        vdeleted_[v1] = true;
                        deleted_vertices_++;
                    }
                }
                else  set_halfedge(v1, next1);
            }
        }
    }


    // update outgoing halfedge handles of remaining vertices
    std::vector<Vertex>::iterator v_it(vertices.begin()),
                                  v_end(vertices.end());
    for (; v_it!=v_end; ++v_it)
        adjust_outgoing_halfedge(*v_it);

    garbage_ = true;
}


//-----------------------------------------------------------------------------


void
SurfaceMesh::
garbage_collection()
{
    int  i, i0, i1,
    nV(vertices_size()),
    nE(edges_size()),
    nH(halfedges_size()),
    nF(faces_size());

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

        while (1)
        {
            // find first deleted and last un-deleted
            while (!vdeleted_[Vertex(i0)] && i0 < i1)  ++i0;
            while ( vdeleted_[Vertex(i1)] && i0 < i1)  --i1;
            if (i0 >= i1) break;

            // swap
            vprops_.swap(i0, i1);
        };

        // remember new size
        nV = vdeleted_[Vertex(i0)] ? i0 : i0+1;
    }


    // remove deleted edges
    if (nE > 0)
    {
        i0=0;  i1=nE-1;

        while (1)
        {
            // find first deleted and last un-deleted
            while (!edeleted_[Edge(i0)] && i0 < i1)  ++i0;
            while ( edeleted_[Edge(i1)] && i0 < i1)  --i1;
            if (i0 >= i1) break;

            // swap
            eprops_.swap(i0, i1);
            hprops_.swap(2*i0,   2*i1);
            hprops_.swap(2*i0+1, 2*i1+1);
        };

        // remember new size
        nE = edeleted_[Edge(i0)] ? i0 : i0+1;
        nH = 2*nE;
    }


    // remove deleted faces
    if (nF > 0)
    {
        i0=0;  i1=nF-1;

        while (1)
        {
            // find 1st deleted and last un-deleted
            while (!fdeleted_[Face(i0)] && i0 < i1)  ++i0;
            while ( fdeleted_[Face(i1)] && i0 < i1)  --i1;
            if (i0 >= i1) break;

            // swap
            fprops_.swap(i0, i1);
        };

        // remember new size
        nF = fdeleted_[Face(i0)] ? i0 : i0+1;
    }


    // update vertex connectivity
    for (i=0; i<nV; ++i)
    {
        v = Vertex(i);
        if (!is_isolated(v))
            set_halfedge(v, hmap[halfedge(v)]);
    }


    // update halfedge connectivity
    for (i=0; i<nH; ++i)
    {
        h = Halfedge(i);
        set_vertex(h, vmap[to_vertex(h)]);
        set_next_halfedge(h, hmap[next_halfedge(h)]);
        if (!is_boundary(h))
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
    vprops_.resize(nV); vprops_.free_memory();
    hprops_.resize(nH); hprops_.free_memory();
    eprops_.resize(nE); eprops_.free_memory();
    fprops_.resize(nF); fprops_.free_memory();

    deleted_vertices_ = deleted_edges_ = deleted_faces_ = 0;
    garbage_ = false;
}


//=============================================================================
} // namespace easy3d
//=============================================================================
