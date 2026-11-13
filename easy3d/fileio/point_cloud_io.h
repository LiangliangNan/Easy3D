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

#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_H

#include <iostream>
#include <vector>


namespace easy3d {


	class PointCloud;

	/**
	 * \brief Load and save point clouds using extension-selected formats.
     * \details Supported extensions are \c bin, \c xyz, \c bxyz, \c ply, \c las, \c laz, \c vg, and \c bvg.
     * Extension matching is case-insensitive. Loading an empty or unsupported file fails.
     * \class PointCloudIO easy3d/fileio/point_cloud_io.h
     */
	class PointCloudIO
	{
	public:
        /**
         * \brief Reads a point cloud from file \p file_name.
         * \details The extension determines the format. Loading an empty or unsupported file fails.
         * \param file_name Path to the input file.
         * \return Newly allocated point cloud owned by the caller, or nullptr if the extension is unsupported,
         *         loading fails, or the file contains no vertices.
         */
		static PointCloud* load(const std::string& file_name);

        /**
         * \brief Saves a point_cloud to a file.
         * \details The extension selects the format. If the path has no extension, \c .ply is appended and binary
         *          PLY is written.
         * \param file_name Output path.
         * \param cloud Point cloud to write; must not be null.
         * \return True if the selected format was written successfully; false for a null cloud, unsupported
         *         extension, or I/O/format failure.
         */
		static bool	save(const std::string& file_name, const PointCloud* cloud);
	};


	namespace io {

	    /// \brief Read a point cloud from Easy3D's binary \c bin format.
	    /// \param file_name Input path.
	    /// \param cloud Destination point cloud.
	    /// \return True on success; false on an I/O or format error.
		bool load_bin(const std::string& file_name, PointCloud* cloud);
        /// \brief Save a point cloud in Easy3D's binary \c bin format.
        /// \param file_name Output path.
        /// \param cloud Point cloud to write.
        /// \return True on success; false on an I/O error.
		bool save_bin(const std::string& file_name, const PointCloud* cloud);

        /// \brief Reads point cloud from an \c xyz format file.
        /// \details Each data line contains three floating-point coordinates.
        /// \param file_name Input path.
        /// \param cloud Destination point cloud.
        /// \return True on success; false on an I/O or parse error.
		bool load_xyz(const std::string& file_name, PointCloud* cloud);
		/// \brief Saves a point cloud to an \c xyz format file.
		/// \details Writes one line of three coordinates per point.
        /// \param file_name Output path.
        /// \param cloud Point cloud to write.
        /// \return True on success; false on an I/O error.
		bool save_xyz(const std::string& file_name, const PointCloud* cloud);

        /// \brief Reads point cloud from a binary \c xyz format file.
        /// \param file_name Input path.
        /// \param cloud Destination point cloud.
        /// \return True on success; false on an I/O or format error.
		bool load_bxyz(const std::string& file_name, PointCloud* cloud);
        /// \brief Saves a point cloud to a binary \c xyz format file.
        /// \param file_name Output path.
        /// \param cloud Point cloud to write.
        /// \return True on success; false on an I/O error.
		bool save_bxyz(const std::string& file_name, const PointCloud* cloud);

        /// \brief Reads point cloud from a \c ply format file.
        /// \param file_name Input PLY path.
        /// \param cloud Destination point cloud.
        /// \return True on success; false on an I/O or parse error.
		bool load_ply(const std::string& file_name, PointCloud* cloud);
        /// \brief Saves a point cloud to a PLY file.
        /// \param file_name Output path.
        /// \param cloud Point cloud to write.
        /// \param binary True to write binary PLY; false to write ASCII PLY.
        /// \return True on success; false on an I/O or format error.
		bool save_ply(const std::string& file_name, const PointCloud* cloud, bool binary = true);

        /// \brief Read a point cloud from an \c las or \c laz file.
        /// \details Requires LASlib-backed support available in the build.
        /// \param file_name Input path.
        /// \param cloud Destination point cloud.
        /// \return True on success; false when unsupported or on I/O/parse failure.
        bool load_las(const std::string &file_name, PointCloud *cloud);
        /// \brief Save a point cloud to an \c las or \c laz file.
        /// \details Requires LASlib-backed support available in the build.
        /// \param file_name Output path.
        /// \param cloud Point cloud to write.
        /// \return True on success; false when unsupported or on I/O failure.
		bool save_las(const std::string& file_name, const PointCloud* cloud);

	} // namespace io


} // namespace easy3d


#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_H
