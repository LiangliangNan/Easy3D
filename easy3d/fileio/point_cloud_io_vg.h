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

#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_VG_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_VG_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;

    namespace io {

        /**
         * \brief Implementation of file input/output operations for vertex group (VG) format PointCloud.
         * \class PointCloudIO_vg easy3d/fileio/point_cloud_io_vg.h
         *
         * \details The vertex group (VG) format stores the point cloud and its segmentation information. A segment is
         * represented by a set of indices of the points in the segment.
         *
         *  File format specification
         *      \code
         *       num_points: N   // N is an integer denoting the number of points
         *       x1  y1  z1	// 3 floating point numbers
         *       ...
         *       xN  yN  zN
         *
         *       // the colors of the points
         *       num_colors: N      // N is an integer denoting the number of colors (can be 0; if not, it must equal to num_points)
         *       r1 g1 b1	        // 3 floating point numbers
         *       ...
         *       rN gN bN
         *
         *       // the normals of the points
         *       num_normals: N     // N is an integer denoting the number of normals (can be 0; if not, it must equal to num_points)
         *       nx1  ny1  nz1	    // 3 floating point numbers
         *       ...
         *       nxN  nyN  nzN
         *
         *       // now we store the segmentation information
         *       num_groups: M      // M is an integer denoting the number of segments/primitives/objects in this point cloud (can be 0)
         *
         *       // now the information for the 1st segment/primitive/object
         *       group_type: type           // integer denoting the of the segment (0: PLANE, 1: CYLINDER, 2: SPHERE, 3: CONE, 4: TORUS, 5: GENERAL)
         *       num_group_parameters: NUM_GROUP_PARAMETERS    // integer number denoting the number of floating point values representing the segment (e.g., 4 for planes)
         *       group_parameters: float[NUM_GROUP_PARAMETERS] // a sequence of NUM_GROUP_PARAMETERS floating point numbers (e.g., a, b, c, and d for a plane)
         *       group_label: label         // the label (a string) of the segment
         *       group_color: r g b         // 3 floating point numbers denoting the color of this segment
         *       group_num_points: N        // N is an integer denoting the number of points in this segment (can be 0)
         *       id1 ... idN                // N integer numbers denoting the indices of the points in this segment
         *       num_children: num          // a segment/primitive/object may contain subsegment (that has the same representation as this segment)
         *       ...
         *       group_type: type           // integer denoting the of the segment (0: PLANE, 1: CYLINDER, 2: SPHERE, 3: CONE, 4: TORUS, 5: GENERAL)
         *       num_group_parameters: NUM_GROUP_PARAMETERS    // integer number denoting the number of floating point values representing the segment (e.g., 4 for planes)
         *       group_parameters: float[NUM_GROUP_PARAMETERS] // a sequence of NUM_GROUP_PARAMETERS floating point numbers (e.g., a, b, c, and d for a plane)
         *       group_label: label         // the label (a string) of the segment
         *       group_color: r g b         // 3 floating point numbers denoting the color of this segment
         *       group_num_points: N        // N is an integer denoting the number of points in this segment (can be 0)
         *       id1 ... idN                // N integer numbers denoting the indices of the points in this segment
         *       num_children: num          // a segment/primitive/object may contain subsegment (that has the same representation as this segment)
         *       ...
         *       \endcode
         */
        class PointCloudIO_vg
        {
        public:

            /**
             * \brief Reads a point cloud from an ASCII file.
             * \param file_name The file name.
             * \param cloud The point cloud to write to.
             * \return The status of the operation
             *      \arg true if succeeded
             *      \arg false if failed
             */
            static bool load_vg(const std::string& file_name, PointCloud* cloud);
            /**
             * \brief Saves a point_cloud to an ASCII file.
             * \param file_name The file name.
             * \param cloud The point cloud.
             * \return The status of the operation
             *      \arg true if succeeded
             *      \arg false if failed
             */
            static bool save_vg(const std::string& file_name, const PointCloud* cloud);

            /**
             * \brief Reads a point cloud from a binary file.
             * \param file_name The file name.
             * \param cloud The point cloud to write to.
             * \return The status of the operation
             *      \arg true if succeeded
             *      \arg false if failed
             */
            static bool load_bvg(const std::string& file_name, PointCloud* cloud);
            /**
             * \brief Saves a point_cloud to a binary file.
             * \param file_name The file name.
             * \param cloud The point cloud.
             * \return The status of the operation
             *      \arg true if succeeded
             *      \arg false if failed
             */
            static bool save_bvg(const std::string& file_name, const PointCloud* cloud);

        private:

            class VertexGroup : public std::vector<int>
            {
            public:
                explicit VertexGroup(int type = UNKNOWN)
                    : primitive_type_(type)
                    , primitive_index_(-1)
                    , label_("unknown")
                    , color_(0.3f, 0.6f, 1.0f)
                {
                }
                ~VertexGroup() = default;

                enum PrimType { // keep the values the same as in RANSAC
                    PLANE = 0,
                    SPHERE = 1,
                    CYLINDER = 2,
                    CONE = 3,
                    TORUS = 4,
                    GENERAL = 5,
                    UNKNOWN = -1
                };

                //      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
                //      - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
                //        primitive_type must be UNKNOWN).
                int primitive_type_;
                int primitive_index_;

                std::string		label_;
                vec3			color_;
                std::vector<VertexGroup>	children_;
            };


            static void read_ascii_group(std::istream& input, VertexGroup& g);
            static void write_ascii_group(std::ostream& output, const VertexGroup& g);

            static void read_binary_group(std::istream& input, VertexGroup& g);
            static void write_binary_group(std::ostream& output, const VertexGroup& g);

            static int num_group_parameters(int type);

            static std::vector<float> get_group_parameters(const VertexGroup& g);
            static void assign_group_parameters(VertexGroup& g, std::vector<float>& para);

            static void collect_groups(const PointCloud* cloud, std::vector<VertexGroup>& groups);
        };

    } // namespace io

} // namespace easy3d

#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_VG_H

