/********************************************************************
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
 ********************************************************************/

#ifndef EASY3D_FILEIO_SURFACE_MESH_IO_H
#define EASY3D_FILEIO_SURFACE_MESH_IO_H


#include <string>


namespace easy3d {

	class SurfaceMesh;


    /**
     * \brief Implementation of file input/output operations for SurfaceMesh.
     * \class SurfaceMeshIO easy3d/fileio/surface_mesh_io.h
     */
	class SurfaceMeshIO
	{
	public:

        /**
         * \brief Reads a surface mesh from a file.
         * \details File extension determines file format (ply, obj, off, stl, poly) and type (i.e. binary or ASCII).
         * \param file_name The file name.
         * \return The pointer of the surface mesh (nullptr if failed).
         */
		static SurfaceMesh* load(const std::string& file_name);

        /**
         * \brief Saves a surface mesh to a file.
         * \details File extension determines file format (ply, obj, off, stl, poly) and type (i.e. binary or ASCII).
         * \param file_name The file name.
         * \param mesh The surface mesh.
         * \return The status of the operation
         *      \arg true if succeeded
         *      \arg false if failed
         */
		static bool	save(const std::string& file_name, const SurfaceMesh* mesh);
	};

	namespace io {

        /// Reads a surface mesh from a \p SM format file.
        bool load_sm(const std::string& file_name, SurfaceMesh* mesh);
        /// Saves a surface mesh to a \p SM format file.
        bool save_sm(const std::string& file_name, const SurfaceMesh* mesh);

        /// Reads a surface mesh from a \p PLY format file.
        bool load_ply(const std::string& file_name, SurfaceMesh* mesh);
        /// Saves a surface mesh to a \p PLY format file.
        bool save_ply(const std::string& file_name, const SurfaceMesh* mesh, bool binary = true);

        /// Reads a surface mesh from a \p OFF format file.
		bool load_off(const std::string& file_name, SurfaceMesh* mesh);
        /// Saves a surface mesh to a \p OFF format file.
		bool save_off(const std::string& file_name, const SurfaceMesh* mesh);

        /// Reads a surface mesh from a \p OBJ format file.
		bool load_obj(const std::string& file_name, SurfaceMesh* mesh);
        /// Saves a surface mesh to a \p OBJ format file.
		bool save_obj(const std::string& file_name, const SurfaceMesh* mesh);

        /// Reads a surface mesh from a \p STL format file.
		bool load_stl(const std::string& file_name, SurfaceMesh* mesh);
        /// Saves a surface mesh to a \p STL format file.
		bool save_stl(const std::string& file_name, const SurfaceMesh* mesh);

		/// Reads a set of triangles (each line has coordinates of 3 points)
		/// Mainly used for easily saving triangles for debugging.
        bool load_trilist(const std::string& file_name, SurfaceMesh* mesh);

        /// Reads Geojson format files. 2D polygons are stored as faces of a 3D surface mesh
        /// (all Z-coordinates are set to 0).
        bool load_geojson(const std::string& file_name, SurfaceMesh* mesh);

	} // namespace io

} // namespace easy3d

#endif // EASY3D_FILEIO_SURFACE_MESH_IO_H
