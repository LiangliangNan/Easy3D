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


#ifndef EASY3D_IO_H
#define EASY3D_IO_H


//== INCLUDES =================================================================


#include <easy3d/point_cloud.h>
#include <easy3d/surface_mesh.h>

#include <string>


//== NAMESPACE ================================================================


namespace easy3d {

//=============================================================================

bool read_cloud(PointCloud& cloud, const std::string& filename);
bool write_cloud(const PointCloud& cloud, const std::string& filename);

//=============================================================================


bool read_mesh(SurfaceMesh& mesh, const std::string& filename);
bool read_off(SurfaceMesh& mesh, const std::string& filename);
bool read_obj(SurfaceMesh& mesh, const std::string& filename);
bool read_poly(SurfaceMesh& mesh, const std::string& filename);
bool read_stl(SurfaceMesh& mesh, const std::string& filename);

bool write_mesh(const SurfaceMesh& mesh, const std::string& filename);
bool write_off(const SurfaceMesh& mesh, const std::string& filename);
bool write_obj(const SurfaceMesh& mesh, const std::string& filename);
bool write_poly(const SurfaceMesh& mesh, const std::string& filename);
bool write_stl(const SurfaceMesh& mesh, const std::string& filename);


//=============================================================================
} // namespace easy3d
//=============================================================================
#endif // EASY3D_IO_H
//=============================================================================
