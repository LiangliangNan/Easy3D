//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011-2016 by Graphics & Geometry Group, Bielefeld University
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


#include <easy3d/model/IO.h>

#include <cstdio>


//== NAMESPACES ===============================================================


namespace easy3d {


//== IMPLEMENTATION ===========================================================


// helper function
template <typename T> size_t read(FILE* in, T& t)
{
    return fread((char*)&t, 1, sizeof(t), in);
}


// helper function
template <typename T> size_t write(FILE* out, T& t)
{
    return fwrite((char*)&t, 1, sizeof(t), out);
}


//-----------------------------------------------------------------------------


bool read_poly(Surface_mesh& mesh, const std::string& filename)
{
    // open file (in binary mode)
    FILE* in = fopen(filename.c_str(), "rb");
    if (!in) return false;


    // clear mesh
    mesh.clear();


    // how many elements?
    unsigned int nv, ne, nh, nf;
    read(in, nv);
    read(in, ne);
    read(in, nf);
    nh = 2*ne;

    // resize containers
	mesh.resize(nv, ne, nf);

    // get properties
    Surface_mesh::Vertex_property<Surface_mesh::Vertex_connectivity>      vconn = mesh.vertex_property<Surface_mesh::Vertex_connectivity>("v:connectivity");
    Surface_mesh::Halfedge_property<Surface_mesh::Halfedge_connectivity>  hconn = mesh.halfedge_property<Surface_mesh::Halfedge_connectivity>("h:connectivity");
    Surface_mesh::Face_property<Surface_mesh::Face_connectivity>          fconn = mesh.face_property<Surface_mesh::Face_connectivity>("f:connectivity");
    Surface_mesh::Vertex_property<Point>                                  point = mesh.vertex_property<Point>("v:point");

    // read properties from file
    fread((char*)vconn.data(), sizeof(Surface_mesh::Vertex_connectivity),   nv, in);
    fread((char*)hconn.data(), sizeof(Surface_mesh::Halfedge_connectivity), nh, in);
    fread((char*)fconn.data(), sizeof(Surface_mesh::Face_connectivity),     nf, in);
    fread((char*)point.data(), sizeof(Point),                               nv, in);

    fclose(in);
    return mesh.n_faces() > 0;
}


//-----------------------------------------------------------------------------


bool write_poly(const Surface_mesh& mesh, const std::string& filename)
{
    // check for colors
    auto color = mesh.get_vertex_property<Color>("v:color");
    bool has_colors = color;


    // open file (in binary mode)
    FILE* out = fopen(filename.c_str(), "wb");
    if (!out) return false;


    // how many elements?
    unsigned int nv, ne, nh, nf;
    nv = mesh.n_vertices();
    ne = mesh.n_edges();
    nh = mesh.n_halfedges();
    nf = mesh.n_faces();

    write(out, nv);
    write(out, ne);
    write(out, nf);
    write(out, has_colors);
    nh = 2*ne;


    // get properties
    Surface_mesh::Vertex_property<Surface_mesh::Vertex_connectivity>      vconn = mesh.get_vertex_property<Surface_mesh::Vertex_connectivity>("v:connectivity");
    Surface_mesh::Halfedge_property<Surface_mesh::Halfedge_connectivity>  hconn = mesh.get_halfedge_property<Surface_mesh::Halfedge_connectivity>("h:connectivity");
    Surface_mesh::Face_property<Surface_mesh::Face_connectivity>          fconn = mesh.get_face_property<Surface_mesh::Face_connectivity>("f:connectivity");
    Surface_mesh::Vertex_property<Point>                                  point = mesh.get_vertex_property<Point>("v:point");


    // write properties to file
    fwrite((char*)vconn.data(), sizeof(Surface_mesh::Vertex_connectivity),   nv, out);
    fwrite((char*)hconn.data(), sizeof(Surface_mesh::Halfedge_connectivity), nh, out);
    fwrite((char*)fconn.data(), sizeof(Surface_mesh::Face_connectivity),     nf, out);
    fwrite((char*)point.data(), sizeof(Point),                               nv, out);

    if (has_colors) fwrite((char*)color.data(), sizeof(Color), nv, out);

    fclose(out);

    return true;
}


//=============================================================================
} // namespace surface_mesh
//=============================================================================
