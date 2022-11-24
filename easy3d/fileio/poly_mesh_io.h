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

#ifndef EASY3D_FILEIO_POLY_MESH_IO_H
#define EASY3D_FILEIO_POLY_MESH_IO_H


#include <string>


namespace easy3d {

	class PolyMesh;


    /**
     * \brief Implementation of file input/output operations for PolyMesh.
     * \class PolyMeshIO easy3d/fileio/poly_mesh_io.h
     */
	class PolyMeshIO
	{
	public:

        /**
         * \brief Reads a polyhedral mesh from a file.
         * \details File extension determines file format (now only '*.plm' format is supported).
         * \param file_name The file name.
         * \return The pointer of the polyhedral mesh (nullptr if failed).
         */
		static PolyMesh* load(const std::string& file_name);

        /**
         * \brief Saves a polyhedral mesh to a file.
         * \details File extension determines file format (now only '*.plm' format is supported).
         * \param file_name The file name.
         * \param mesh The polyhedral mesh.
         * \return The status of the operation
         *      \arg true if succeeded
         *      \arg false if failed
         */
		static bool	save(const std::string& file_name, const PolyMesh* mesh);
	};


    namespace io {

        /// Reads a polyhedral mesh from a \p PM format file. This is the built-in binary format of Easy3D.
        bool load_pm(const std::string& file_name, PolyMesh* mesh);
        /// Saves a polyhedral mesh to a \p PM format file. This is the built-in binary format of Easy3D.
        bool save_pm(const std::string& file_name, const PolyMesh* mesh);

        /// Reads a polyhedral mesh from a \p PLM format file. This is the built-in ASCII format of Easy3D.
        bool load_plm(const std::string& file_name, PolyMesh* mesh);
        /// Saves a polyhedral mesh to a \p PLM format file. This is the built-in ASCII format of Easy3D.
        bool save_plm(const std::string& file_name, const PolyMesh* mesh);

        /// Reads a polyhedral mesh from a \p MESH format file. This ASCII format is supported by Tetgen and Medit.
        bool load_mesh(const std::string& file_name, PolyMesh* mesh);
        /// Saves a polyhedral mesh to a \p MESH format file. This ASCII format is supported by Tetgen and Medit.
        bool save_mesh(const std::string& file_name, const PolyMesh* mesh);

    } // namespace io

} // namespace easy3d

#endif // EASY3D_FILEIO_POLY_MESH_IO_H
