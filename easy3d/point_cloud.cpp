//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011-2013 by Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================


//== INCLUDES =================================================================


#include <easy3d/point_cloud.h>
#include <easy3d/IO.h>

#include <cmath>


//== NAMESPACE ================================================================


namespace easy3d {


//== IMPLEMENTATION ===========================================================


Point_cloud::Point_cloud()
{
    // allocate standard properties
    // same list is used in operator=() and assign()
    vpoint_   = add_vertex_property<vec3>("v:point");
    vdeleted_ = add_vertex_property<bool>("v:deleted", false);

	mprops_.push_back();

    deleted_vertices_ = 0;
    garbage_ = false;
}


//-----------------------------------------------------------------------------


Point_cloud::~Point_cloud()
{
}


//-----------------------------------------------------------------------------


Point_cloud& Point_cloud::operator=(const Point_cloud& rhs)
{
    if (this != &rhs)
    {
        // deep copy of property containers
        vprops_ = rhs.vprops_;
		mprops_ = rhs.mprops_;

        // property handles contain pointers, have to be reassigned
        vdeleted_ = vertex_property<bool>("v:deleted");
        vpoint_   = vertex_property<vec3>("v:point");

        // how many elements are deleted?
        deleted_vertices_ = rhs.deleted_vertices_;
        garbage_          = rhs.garbage_;
    }

    return *this;
}


//-----------------------------------------------------------------------------


Point_cloud& Point_cloud::assign(const Point_cloud& rhs)
{
    if (this != &rhs)
    {
        // clear properties
        vprops_.clear();
		mprops_.clear();

        // allocate standard properties
        vpoint_   = add_vertex_property<vec3>("v:point");
        vdeleted_ = add_vertex_property<bool>("v:deleted", false);

        // copy properties from other cloud
		vpoint_.array() = rhs.vpoint_.array();
		vdeleted_.array() = rhs.vdeleted_.array();

        // resize (needed by property containers)
        vprops_.resize(rhs.vertices_size());
		mprops_.resize(1);

        // how many elements are deleted?
        deleted_vertices_ = rhs.deleted_vertices_;
        garbage_          = rhs.garbage_;
    }

    return *this;
}


//-----------------------------------------------------------------------------


bool Point_cloud::read(const std::string& filename)
{
	return read_cloud(*this, filename);
}


//-----------------------------------------------------------------------------


bool
Point_cloud::
write(const std::string& filename) const
{
	return write_cloud(*this, filename);
}


//-----------------------------------------------------------------------------


void Point_cloud::clear()
{
    vprops_.resize(0);

    free_memory();

    deleted_vertices_ = 0;
    garbage_ = false;
}


//-----------------------------------------------------------------------------


void Point_cloud::free_memory()
{
    vprops_.free_memory();
}


//-----------------------------------------------------------------------------


void Point_cloud::property_stats() const
{
    std::vector<std::string> props = vertex_properties();

    std::cout << "vertex properties:\n";
    for (unsigned int i=0; i<props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;

	std::cout << "model properties:\n";
	props = model_properties();
	for (unsigned int i = 0; i < props.size(); ++i)
		std::cout << "\t" << props[i] << std::endl;
}


//-----------------------------------------------------------------------------


Point_cloud::Vertex Point_cloud::add_vertex(const vec3& p)
{
    Vertex v = new_vertex();
    vpoint_[v] = p;
    return v;
}


//-----------------------------------------------------------------------------


void Point_cloud::delete_vertex(Vertex v)
{
    if (vdeleted_[v])  return;

    // mark v as deleted
    vdeleted_[v] = true;
    deleted_vertices_++;
    garbage_ = true;
}


//-----------------------------------------------------------------------------


void Point_cloud::garbage_collection()
{
	int  nV(vertices_size());

    // setup handle mapping
    Vertex_property<Vertex> vmap = add_vertex_property<Vertex>("v:garbage-collection");
    for (int i=0; i<nV; ++i)
        vmap[Vertex(i)] = Vertex(i);

    // remove deleted vertices
    if (nV > 0)
    {
		int i0 = 0;
		int i1 = nV - 1;

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

    // remove handle maps
    remove_vertex_property(vmap);

    // finally resize arrays
    vprops_.resize(nV); 
	vprops_.free_memory();

    deleted_vertices_ = 0;
    garbage_ = false;
}


//=============================================================================
} // namespace point_cloud
//=============================================================================
