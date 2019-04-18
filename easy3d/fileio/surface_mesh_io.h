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


#ifndef EASY3D_FILEIO_MESH_IO_H
#define EASY3D_FILEIO_MESH_IO_H


#include <string>


namespace easy3d {

	class SurfaceMesh;

    // TODO: add a general purpose file format 'map' capable of storing
    //       both point clouds and meshes with arbitrary types of
    //       properties (similar to PLY, each property is read/written
    //       as a whole. Check PLY and POLY formats as reference.
    //       When this is done, I don't need BIN and POLY any more).

	class SurfaceMeshIO
	{
	public:
		// return nullptr if failed.
		static SurfaceMesh* load(const std::string& file_name);

		// save the mesh to a file. return false if failed.
		static bool	save(const std::string& file_name, const SurfaceMesh* mesh);
	};

	namespace io {

        bool load_poly(const std::string& file_name, SurfaceMesh* mesh);
        bool save_poly(const std::string& file_name, const SurfaceMesh* mesh);

        bool load_ply(const std::string& file_name, SurfaceMesh* mesh);
        bool save_ply(const std::string& file_name, const SurfaceMesh* mesh, bool binary = true);

		bool load_off(const std::string& file_name, SurfaceMesh* mesh);
		bool save_off(const std::string& file_name, const SurfaceMesh* mesh);

		bool load_obj(const std::string& file_name, SurfaceMesh* mesh);
		bool save_obj(const std::string& file_name, const SurfaceMesh* mesh);

		bool load_stl(const std::string& file_name, SurfaceMesh* mesh);
		bool save_stl(const std::string& file_name, const SurfaceMesh* mesh);

	} // namespace io

} // namespace easy3d

#endif // EASY3D_FILEIO_MESH_IO_H
