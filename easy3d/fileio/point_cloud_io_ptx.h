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

#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_PTX_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_PTX_H

#include <string>

namespace easy3d {

	class PointCloud;

	namespace io {

		class LineInputStream;

		/**
         * \brief Implementation of file input/output operations for ASCII Cyclone point cloud export format (PTX).
         * \class PointCloudIO_ptx easy3d/fileio/point_cloud_io_ptx.h
         *
         *  \details PTX is an ASCII based interchange format for point cloud data. It utilizes the concept of separate scans,
         *  each with points defined in their own coordinate system and a 'Registration' of all of those point clouds
         *  into a single coordinate system. The point data for each cloud is stored in its original coordinate system
         *  and a matrix of transforms for each point cloud are provided as header information.
         *
         *  A PTX file can have one or more point cloud. Each point cloud starts with a header. Each following line
         *  then describes one point in the cloud. Cyclone exports PTX with 7 columns when the cloud has RGB values
         *  from the digital camera (x, y, z, intensity, red, green, blue). Red, Green, Blue have the integer range [0, 255].
         *  A cloud in PTX has 4 columns (x, y, z, intensity) when the cloud does not have RGB values. PTX intensities
         *  use the decimal range [0, 1]. Individual values on the same line are separated by a blank space. The coordinate
         *  unit is always in meters. PTX can only be used on "[[KB:132:gridded]]" clouds (directly from scans), not unordered
         *  or unified clouds. Each cloud is "fully populated", in that even missing points are represented (as a line with
         *  "0 0 0" for the XYZ coordinate). This gives the importing software enough information to restore the original
         *  scan-line ordering of the cloud (for example, to estimate normal vectors).
         *
         *  PTX point cloud header:
         *      \code
         *      number of columns
         *      number of rows
         *      st1 st2 st3 ; scanner registered position
         *      sx1 sx2 sx3 ; scanner registered axis 'X'
         *      sy1 sy2 sy3 ; scanner registered axis 'Y'
         *      sz1 sz2 sz3 ; scanner registered axis 'Z'
         *      r11 r12 r13 0 ; transformation matrix
         *      r21 r22 r23 0 ; this is a simple rotation and translation 4x4 matrix
         *      r31 r32 r33 0 ; just apply to each point to get the transformed coordinate
         *      tr1 tr2 tr3 1 ; use double-precision variables
         *      \endcode
         *
         *  The first four lines of three numbers each are the position and primary axes of the scanner after any
         *  registration/transformation. The next four lines of four numbers each may look similar in some cases,
         *  but if you have a non-identity UCS when the PTX was exported, the numbers will look different. If the
         *  cloud was untransformed by a registration (or not registered), the first four lines of three numbers
         *  each would be 0,0,0; 1,0,0; 0,1,0; 0,0,1. The 4x4 matrix may not be identity if there is a UCS applied.
         *
         *  Note that a cloud that was registered may have a 4x4 identity matrix due to a UCS that is set to that
         *  scanner's registered position. For both formats, in a point cloud with color (i.e., the R G B values a
         *  re present for each point in that cloud), the RGB value (0, 0, 0) is reserved to mean "no color". An
         *  example where this might happen would be if a digital camera did not cover that point. That point then
         *  has "no color", even though other points may have color.
         *
         *  Example data file:
         *      \code
         *      4.148210 12.076586 1.381819 0.461036 89 87 112
         *      4.147804 12.075443 1.382720 0.458854 89 87 112
         *      4.148891 12.078566 1.384060 0.461967 87 86 111   End coordinate list (cloud #1)
         *      40                                               Scan 40 x 40 degree (cloud #2)
         *      40
         *      4.151261 1.736411 -8.110131                      Begin transformation (cloud #2)
         *      -0.434947 -0.017233 -0.900291
         *      -0.240839 0.965618 0.097871
         *      0.867650 0.259394 -0.424143
         *      -0.434947 -0.017233 -0.900291 0
         *      -0.240839 0.965618 0.097871 0
         *      0.867650 0.259394 -0.424143 0
         *      4.151261 1.736411 -8.110131 1
         *      4.146983 12.074142 1.134998 0.488502 76 74 91     Begin coordinate list (cloud #2)
         *      4.147541 12.075727 1.136227 0.486915 76 74 91     Format: X Y Z intensity R G B
         *      4.147919 12.076833 1.137371 0.486976 76 74 91     Intensity range: 0 - 1
         *      4.148849 12.079544 1.138655 0.485328 76 74 91
         *      4.146931 12.073991 1.139137 0.486076 76 74 91
         *      4.148440 12.078399 1.140532 0.488441 79 76 93
         *      \endcode
         *
         *  \note
         *      - PTX files hold the intensities in a range 0-1
         *      - PTS files are -2048 to 2047 as integers (no fractions)
         *      - If you need to convert PTS to PTX intensity values you need to add 2048 and divide by 4096.
         *
         *  See http://w3.leica-geosystems.com/kb/?guid=5532D590-114C-43CD-A55F-FE79E5937CB2
         *
         *  Example usage:
		 *      \code
		 *		PointCloudIO_ptx serializer(file_name);
		 *		PointCloud* model = nullptr;
		 *		while (model = serializer.load_next()) {
		 *			addModel(model);
		 *		}
		 *		\endcode
		 */

		class PointCloudIO_ptx
		{
		public:
			explicit PointCloudIO_ptx(const std::string& file_name);
			~PointCloudIO_ptx();

			/// \brief Reads a single point cloud from the file.
			PointCloud* load_next();

		private:
			std::ifstream*		input_;
			LineInputStream*	in_;

			std::string			file_name_;
			int					cloud_index_;
		};

	} // namespace io

} // namespace easy3d

#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_PTX_H

