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

#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_VG_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_VG_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;

    namespace io {


        class PointCloudIO_vg
        {
        public:

            // labeled vertex groups
            static bool load_vg(const std::string& file_name, PointCloud* cloud);
            static bool save_vg(const std::string& file_name, const PointCloud* cloud);

            static bool load_bvg(const std::string& file_name, PointCloud* cloud);
            static bool save_bvg(const std::string& file_name, const PointCloud* cloud);

        private:

            class VertexGroup : public std::vector<int>
            {
            public:
                VertexGroup(int type = UNKNOWN)
                    : primitive_type_(type)
                    , primitive_index_(-1)
                    , label_("unknown")
                    , color_(0.3f, 0.6f, 1.0f)
                    , parent_(nullptr)
                {
                }
                ~VertexGroup() {}

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
                //        primtive_type must be UNKNOWN.
                int primitive_type_;
                int primitive_index_;

                std::string		label_;
                vec3			color_;
                VertexGroup*	parent_;
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

