/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_FILEIO_POLY_MESH_IO_H
#define EASY3D_FILEIO_POLY_MESH_IO_H


#include <string>


namespace easy3d {

	class PolyMesh;


    /**
     * \brief Load and save polyhedral meshes using extension-selected formats.
     * \details The convenience interface currently supports the ASCII \c .plm format.
     * \class PolyMeshIO easy3d/fileio/poly_mesh_io.h
     */
	class PolyMeshIO
	{
	public:

        /**
         * \brief Reads a polyhedral mesh from a file.
         * \param file_name Input \c .plm path.
         * \return Newly allocated mesh owned by the caller, or nullptr if loading fails.
         */
		static PolyMesh* load(const std::string& file_name);

        /**
         * \brief Saves a polyhedral mesh to a file.
         * \param file_name Output \c .plm path.
         * \param mesh Polyhedral mesh to write.
         * \return True on success; false for an unsupported extension or I/O/format failure.
         */
		static bool	save(const std::string& file_name, const PolyMesh* mesh);
	};


    namespace io {

        /// Read a polyhedral mesh from Easy3D's binary \c .pm format.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or format error.
        bool load_pm(const std::string& file_name, PolyMesh* mesh);
        /// Save a polyhedral mesh in Easy3D's binary \c .pm format.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
        bool save_pm(const std::string& file_name, const PolyMesh* mesh);

        /// Read a polyhedral mesh from Easy3D's ASCII \c .plm format.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or format error.
        bool load_plm(const std::string& file_name, PolyMesh* mesh);
        /// Save a polyhedral mesh in Easy3D's ASCII \c .plm format.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
        bool save_plm(const std::string& file_name, const PolyMesh* mesh);

        /// Read a polyhedral mesh from the TetGen/Medit ASCII \c .mesh format.
        /// \param file_name Input path.
        /// \param mesh Destination mesh.
        /// \return True on success; false on an I/O or parse error.
        bool load_mesh(const std::string& file_name, PolyMesh* mesh);
        /// Save a polyhedral mesh to the TetGen/Medit ASCII \c .mesh format.
        /// \param file_name Output path.
        /// \param mesh Mesh to write.
        /// \return True on success; false on an I/O error.
        bool save_mesh(const std::string& file_name, const PolyMesh* mesh);

    } // namespace io

} // namespace easy3d

#endif // EASY3D_FILEIO_POLY_MESH_IO_H
