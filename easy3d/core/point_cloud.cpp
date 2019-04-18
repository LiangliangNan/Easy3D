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


#include <easy3d/core/point_cloud.h>

#include <cmath>


//== NAMESPACE ================================================================


namespace easy3d {


//== IMPLEMENTATION ===========================================================


PointCloud::PointCloud()
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


PointCloud::~PointCloud()
{
}


//-----------------------------------------------------------------------------


PointCloud& PointCloud::operator=(const PointCloud& rhs)
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


PointCloud& PointCloud::assign(const PointCloud& rhs)
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


void PointCloud::clear()
{
    vprops_.resize(0);

    free_memory();

    deleted_vertices_ = 0;
    garbage_ = false;
}


//-----------------------------------------------------------------------------


void PointCloud::free_memory()
{
    vprops_.free_memory();
}


//-----------------------------------------------------------------------------


void PointCloud::property_stats() const
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


PointCloud::Vertex PointCloud::add_vertex(const vec3& p)
{
    Vertex v = new_vertex();
    vpoint_[v] = p;
    return v;
}


//-----------------------------------------------------------------------------


void PointCloud::delete_vertex(Vertex v)
{
    if (vdeleted_[v])  return;

    // mark v as deleted
    vdeleted_[v] = true;
    deleted_vertices_++;
    garbage_ = true;
}


//-----------------------------------------------------------------------------


void PointCloud::garbage_collection()
{
	int  nV(vertices_size());

    // setup handle mapping
    VertexProperty<Vertex> vmap = add_vertex_property<Vertex>("v:garbage-collection");
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
