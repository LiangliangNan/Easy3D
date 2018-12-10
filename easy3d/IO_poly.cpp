/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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


#include <easy3d/IO.h>

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


bool read_poly(SurfaceMesh& mesh, const std::string& filename)
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
    SurfaceMesh::VertexProperty<SurfaceMesh::VertexConnectivity>      vconn = mesh.vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
    SurfaceMesh::HalfedgeProperty<SurfaceMesh::HalfedgeConnectivity>  hconn = mesh.halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
    SurfaceMesh::FaceProperty<SurfaceMesh::FaceConnectivity>          fconn = mesh.face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
    SurfaceMesh::VertexProperty<vec3>                                  point = mesh.vertex_property<vec3>("v:point");

    // read properties from file
    fread((char*)vconn.data(), sizeof(SurfaceMesh::VertexConnectivity),   nv, in);
    fread((char*)hconn.data(), sizeof(SurfaceMesh::HalfedgeConnectivity), nh, in);
    fread((char*)fconn.data(), sizeof(SurfaceMesh::FaceConnectivity),     nf, in);
    fread((char*)point.data(), sizeof(vec3),                               nv, in);

    fclose(in);
    return mesh.n_faces() > 0;
}


//-----------------------------------------------------------------------------


bool write_poly(const SurfaceMesh& mesh, const std::string& filename)
{
    // check for colors
    auto color = mesh.get_vertex_property<vec3>("v:color");
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
    SurfaceMesh::VertexProperty<SurfaceMesh::VertexConnectivity>      vconn = mesh.get_vertex_property<SurfaceMesh::VertexConnectivity>("v:connectivity");
    SurfaceMesh::HalfedgeProperty<SurfaceMesh::HalfedgeConnectivity>  hconn = mesh.get_halfedge_property<SurfaceMesh::HalfedgeConnectivity>("h:connectivity");
    SurfaceMesh::FaceProperty<SurfaceMesh::FaceConnectivity>          fconn = mesh.get_face_property<SurfaceMesh::FaceConnectivity>("f:connectivity");
    SurfaceMesh::VertexProperty<vec3>                                  point = mesh.get_vertex_property<vec3>("v:point");


    // write properties to file
    fwrite((char*)vconn.data(), sizeof(SurfaceMesh::VertexConnectivity),   nv, out);
    fwrite((char*)hconn.data(), sizeof(SurfaceMesh::HalfedgeConnectivity), nh, out);
    fwrite((char*)fconn.data(), sizeof(SurfaceMesh::FaceConnectivity),     nf, out);
    fwrite((char*)point.data(), sizeof(vec3),                               nv, out);

    if (has_colors) fwrite((char*)color.data(), sizeof(vec3), nv, out);

    fclose(out);

    return true;
}


//=============================================================================
} // namespace surface_mesh
//=============================================================================
