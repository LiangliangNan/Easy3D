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
                VertexGroup(int type = VG_UNKNOWN)
                    : primitive_type_(type)
                    , label_("unknown")
                    , color_(0.3f, 0.6f, 1.0f)
                    , parent_(nullptr)
                {
                }
                ~VertexGroup() {}

                enum {  // primitive type
                    VG_PLANE = 0,
                    VG_SPHERE = 1,
                    VG_CYLINDER = 2,
                    VG_CONE = 3,
                    VG_TORUS = 4,
                    VG_GENERAL = 5,
                    VG_UNKNOWN = 6
                };
                //      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
                //      - "v:primitive_index" (0, 1, 2...)
                int primitive_type_;
                int primitive_index_;

                std::string		label_;
                vec3			color_;
                VertexGroup*	parent_;
                std::vector<VertexGroup>	children_;
            };


            static VertexGroup read_ascii_group(std::istream& input);
            static void write_ascii_group(std::ostream& output, const VertexGroup& g);

            static VertexGroup read_binary_group(std::istream& input);
            static void write_binary_group(std::ostream& output, const VertexGroup& g);

            static std::size_t num_group_parameters(int type);

            static std::vector<float> get_group_parameters(const VertexGroup& g);
            static void assign_group_parameters(VertexGroup& g, std::vector<float>& para);

            static std::vector<VertexGroup> collect_groups(const PointCloud* cloud);
        };

    } // namespace io

} // namespace easy3d

#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_VG_H

