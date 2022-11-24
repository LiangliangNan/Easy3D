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

#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_H

#include <iostream>
#include <vector>


namespace easy3d {


	class PointCloud;

	/**
	 * \brief Implementation of file input/output operations for PointCloud.
     * \class PointCloudIO easy3d/fileio/point_cloud_io.h
     */
	class PointCloudIO
	{
	public:
        /**
         * \brief Reads a point cloud from file \p file_name.
         * \details File extension determines file format (bin, xyz/bxyz, ply, las/laz, vg/bvg)
         * and type (i.e. binary or ASCII).
         * \return The pointer of the point cloud (nullptr if failed).
         */
		static PointCloud* load(const std::string& file_name);

        /**
         * \brief Saves a point_cloud to a file.
         * \details File extension determines file format (bin, xyz/bxyz, ply, las/laz, vg/bvg) and type (i.e. binary
         * or ASCII).
         * \param file_name The file name.
         * \param cloud The point cloud.
         * \return The status of the operation
         *      \arg true if succeeded
         *      \arg false if failed
         */
		static bool	save(const std::string& file_name, const PointCloud* cloud);
	};


	namespace io {

	    /// \brief Reads point cloud from a \c bin format file.
	    /// \details A typical \c bin format file contains three blocks storing points, colors (optional),
	    /// and normals (optional).
		bool load_bin(const std::string& file_name, PointCloud* cloud);
        /// \brief Saves a point cloud to a \c bin format file.
        /// \details A typical \c bin format file contains three blocks storing points, colors (optional),
        /// and normals (optional).
		bool save_bin(const std::string& file_name, const PointCloud* cloud);

        /// \brief Reads point cloud from an \c xyz format file.
        /// \details Each line of an \c xyz file contains three floating point numbers representing the \p x, \p y, and
        /// \p z coordinates of a point.
		bool load_xyz(const std::string& file_name, PointCloud* cloud);
		bool save_xyz(const std::string& file_name, const PointCloud* cloud);
        /// \brief Reads point cloud from a binary \c xyz format file.
		bool load_bxyz(const std::string& file_name, PointCloud* cloud);
        /// \brief Saves a point cloud to a binary \c xyz format file.
		bool save_bxyz(const std::string& file_name, const PointCloud* cloud);

        /// \brief Reads point cloud from a \c ply format file.
		bool load_ply(const std::string& file_name, PointCloud* cloud);
        /// \brief Saves a point cloud to a \c ply format file.
		bool save_ply(const std::string& file_name, const PointCloud* cloud, bool binary = true);

        /// \brief Reads point cloud from an \c las/laz format file.
        ///     Internally the method uses the LASlib of martin.isenburg@rapidlasso.com. See http://rapidlasso.com
        bool load_las(const std::string &file_name, PointCloud *cloud);
        /// \brief Saves a point cloud to an \c LAS/LAS format file.
        /// \details Internally it uses the LASlib of martin.isenburg@rapidlasso.com. See http://rapidlasso.com
		bool save_las(const std::string& file_name, const PointCloud* cloud);

	} // namespace io


} // namespace easy3d


#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_H
