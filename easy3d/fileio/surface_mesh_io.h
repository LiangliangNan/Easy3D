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
     * \brief Load and save surface meshes using extension-selected formats.
     * \details Supported extensions are \c sm, \c ply, \c off, \c obj, \c stl, and \c geojson for loading;
     *          only \c sm, \c ply, \c off, \c obj, and \c stl are supported for saving. Extension matching is
     *          case-insensitive.
     * \class SurfaceMeshIO easy3d/fileio/surface_mesh_io.h
     */
	class SurfaceMeshIO
	{
	public:

        /**
         * \brief Reads a surface mesh from a file.
         * \details The extension determines the format. The returned mesh is owned by the caller.
         * \param file_name Input path.
         * \return Newly allocated mesh, or nullptr if the extension is unsupported or loading fails.
         */
		static SurfaceMesh* load(const std::string& file_name);

        /**
         * \brief Saves a surface mesh to a file.
         * \details The extension determines the format. PLY output is binary by default; use io::save_ply() to
         *          choose ASCII output.
         * \param file_name Output path.
         * \param mesh Mesh to write.
         * \return True on success; false for an unsupported extension or I/O/format failure.
         */
		static bool	save(const std::string& file_name, const SurfaceMesh* mesh);
	};

	namespace io {

        /// Read a surface mesh from Easy3D's \c .sm format.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or format error.
        bool load_sm(const std::string& file_name, SurfaceMesh* mesh);
        /// Save a surface mesh in Easy3D's \c .sm format.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
        bool save_sm(const std::string& file_name, const SurfaceMesh* mesh);

        /// Read a surface mesh from PLY.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
        bool load_ply(const std::string& file_name, SurfaceMesh* mesh);
        /// Save a surface mesh to PLY.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \param binary True to write binary PLY; false to write ASCII PLY.
        /// \return True on success; false on an I/O or format error.
        bool save_ply(const std::string& file_name, const SurfaceMesh* mesh, bool binary = true);

        /// Read a surface mesh from OFF.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
		bool load_off(const std::string& file_name, SurfaceMesh* mesh);
        /// Save a surface mesh to OFF.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
		bool save_off(const std::string& file_name, const SurfaceMesh* mesh);

        /// Read a surface mesh from OBJ.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
		bool load_obj(const std::string& file_name, SurfaceMesh* mesh);
        /// Save a surface mesh to OBJ.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
		bool save_obj(const std::string& file_name, const SurfaceMesh* mesh);

        /// Read a surface mesh from STL.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
		bool load_stl(const std::string& file_name, SurfaceMesh* mesh);
        /// Save a surface mesh to STL.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
		bool save_stl(const std::string& file_name, const SurfaceMesh* mesh);

		/// Read triangles from a text file, with three point coordinates per line.
		/// Mainly used for easily saving triangles for debugging.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
        bool load_trilist(const std::string& file_name, SurfaceMesh* mesh);

        /// Reads Geojson format files. 2D polygons are stored as faces of a 3D surface mesh
        /// (all Z-coordinates are set to 0).
        /// \param file_name Input GeoJSON path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
        bool load_geojson(const std::string& file_name, SurfaceMesh* mesh);

	} // namespace io

} // namespace easy3d

#endif // EASY3D_FILEIO_SURFACE_MESH_IO_H
