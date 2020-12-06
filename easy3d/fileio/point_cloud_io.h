/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_H

#include <iostream>
#include <vector>


namespace easy3d {


	class PointCloud;

	/**
	 * \brief File IO for point clouds.
     * \todo Add a general purpose file format 'map' capable of storing
     *       point clouds, graphs, and meshes with arbitrary types of
     *       properties (similar to PLY, each property is read/written
     *       as a whole. Check PLY and POLY formats as reference.
     *       When this is done, I don't need BIN and POLY any more).
     */

	class PointCloudIO
	{
	public:
        /**
         * \brief Reads a point cloud from file \p file_name. Returns true if succeeded.
         * \details File extension determines file format (bin, xyz/bxyz, ply, las/laz, vg/bvg)
         * and type (i.e. binary or ASCII).
         */
		static PointCloud* load(const std::string& file_name);

        /**
         * \brief Saves a point cloud to file \p file_name. Returns true if succeeded.
         * \details File extension determines file format (bin, xyz/bxyz, ply, las/laz, vg/bvg)
         * and type (i.e. binary or ASCII).
         */
		static bool	save(const std::string& file_name, const PointCloud* point_set);
	};


	namespace io {
		// three blocks storing points, colors (optional), and normals (optional)
		bool load_bin(const std::string& file_name, PointCloud* cloud);
		bool save_bin(const std::string& file_name, const PointCloud* cloud);

		// each line with only point: (x, y, z)
		bool load_xyz(const std::string& file_name, PointCloud* cloud);
		bool save_xyz(const std::string& file_name, const PointCloud* cloud);
		bool load_bxyz(const std::string& file_name, PointCloud* cloud);
		bool save_bxyz(const std::string& file_name, const PointCloud* cloud);

		bool load_ply(const std::string& file_name, PointCloud* cloud);
		bool save_ply(const std::string& file_name, const PointCloud* cloud, bool binary = true);

		// Read/Write both las and laz formats. Internally it uses the LASlib
        // of martin.isenburg@rapidlasso.com, see http://rapidlasso.com
		bool load_las(const std::string& file_name, PointCloud* cloud);
		bool save_las(const std::string& file_name, const PointCloud* cloud);
	};


    /// @example Tutorial_103_PointCloud_IO     \include Tutorial_103_PointCloud_IO/main.cpp


} // namespace easy3d


#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_H
