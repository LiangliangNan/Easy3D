/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#include <easy3d/fileio/point_cloud_io.h>

#include <cassert>
#include <algorithm>
#include <climits>  // for USHRT_MAX

#include <easy3d/core/point_cloud.h>
#include <3rd_party/lastools/LASlib/inc/lasreader.hpp>
#include <3rd_party/lastools/LASlib/inc/laswriter.hpp>


namespace easy3d {


    // \cond
    namespace io {


        bool load_las(const std::string &file_name, PointCloud *cloud, bool transform, const std::string &prop_name) {
            LASreadOpener lasreadopener;
            lasreadopener.set_file_name(file_name.c_str(), true);

            LASreader *lasreader = lasreadopener.open();
            if (!lasreader || lasreader->npoints <= 0) {
                LOG(ERROR) << "could not open file: " << file_name;
                lasreader->close();
                delete lasreader;
                return false;
            }

            std::size_t num = lasreader->npoints;
            LOG(INFO) << "reading " << num << " points...";

            // read the first point
            if (!lasreader->read_point()) {
                LOG(ERROR) << "failed reading point";
                lasreader->close();
                delete lasreader;
                return false;
            }

            LASpoint &p0 = lasreader->point;
            // compute the actual coordinates as double floating point values
            p0.compute_coordinates();
            double x0 = p0.coordinates[0];
            double y0 = p0.coordinates[1];
            double z0 = p0.coordinates[2];
            LOG_IF((!transform) && (x0 > 1e4 || y0 > 1e4 || z0 > 1e4), WARNING)
                << "model has large coordinates (first point: "
                << x0 << " " << y0 << " " << z0
                << ") and some decimals may be lost. Hint: transform the model w.r.t. its first point";

            float r = p0.have_rgb ? float(p0.get_R()) / USHRT_MAX : p0.intensity % 255 / 255.0f;
            float g = p0.have_rgb ? float(p0.get_G()) / USHRT_MAX : p0.intensity % 255 / 255.0f;
            float b = p0.have_rgb ? float(p0.get_B()) / USHRT_MAX : p0.intensity % 255 / 255.0f;

            auto colors = cloud->add_vertex_property<vec3>("v:color");
            auto classification = cloud->add_vertex_property<int>("v:classification");
            auto v = cloud->add_vertex(transform ? vec3(0, 0, 0) : vec3(float(x0), float(y0), float(z0)));
            colors[v] = vec3(r, g, b);
            classification[v] = p0.classification;

            // now we read the remaining points...
            while (lasreader->read_point()) {
                LASpoint &p = lasreader->point;

                // compute the actual coordinates as double floating point values
                p.compute_coordinates();
                double x = transform ? p.coordinates[0] - x0 : p.coordinates[0];
                double y = transform ? p.coordinates[1] - y0 : p.coordinates[1];
                double z = transform ? p.coordinates[2] - z0 : p.coordinates[2];
                v = cloud->add_vertex(vec3(float(x), float(y), float(z)));

                r = p.have_rgb ? float(p.get_R()) / USHRT_MAX : p.intensity % 255 / 255.0f;
                g = p.have_rgb ? float(p.get_G()) / USHRT_MAX : p.intensity % 255 / 255.0f;
                b = p.have_rgb ? float(p.get_B()) / USHRT_MAX : p.intensity % 255 / 255.0f;

                colors[v] = vec3(r, g, b);
                classification[v] = p.classification;
            }

            auto trans = cloud->add_model_property<mat4>("transformation", mat4::identity());
            trans[0] = mat4::translation(-x0, -y0, -z0);
            LOG_IF(transform, WARNING) << "model transformed w.r.t. its first point ("
                                       << x0 << " " << y0 << " " << z0
                                       << "), stored as ModelProperty<mat4>(\"transformation\")";

            lasreader->close();
            delete lasreader;
            return cloud->n_vertices() > 0;
        }


        bool save_las(const std::string &file_name, const PointCloud *cloud) {
            if (!cloud) {
                LOG(ERROR) << "null input point cloud pointer";
                return false;
            }

            PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
            if (normals)
                LOG(ERROR) << "normals discarded when saving to LAS or LAZ format (future release may support).";

            LASwriteOpener laswriteopener;
            laswriteopener.set_file_name(file_name.c_str());

            // check output
            if (!laswriteopener.active()) {
                LOG(ERROR) << "could not save file";
                return false;
            }

            auto points = cloud->get_vertex_property<vec3>("v:point");
            const Box3 &box = cloud->bounding_box();
            const vec3 &center = box.center();
            LOG(INFO) << "saving " << cloud->n_vertices() << " points...";

            // init header
            // to set a 'accurate enough' scale factor, I follow the suggestion here:
            // saves the cloud with a very high relative accuracy (= scaling) of 10^-9 multiplied by the
            // cloud bounding-box dimension (10^-9 because the integer saved in LAS files is coded on 32
            // bits which roughly corresponds to +/-2.10^9). http://www.danielgm.net/cc/forum/viewtopic.php?t=1103
            LASheader lasheader;
            //optimal scale (for accuracy) --> 1e-9 because the maximum integer is roughly +/-2e+9
            lasheader.x_scale_factor = 1.0e-9 * std::max<double>(box.range(0), DBL_EPSILON);
            lasheader.y_scale_factor = 1.0e-9 * std::max<double>(box.range(1), DBL_EPSILON);
            lasheader.z_scale_factor = 1.0e-9 * std::max<double>(box.range(2), DBL_EPSILON);
            lasheader.x_offset = center.x;    // box.x_min(); should also work
            lasheader.y_offset = center.y;    // box.y_min(); should also work
            lasheader.z_offset = center.z;    // box.z_min(); should also work

            LOG(INFO) << "scale factor: "
                      << lasheader.x_scale_factor << " "
                      << lasheader.y_scale_factor << " "
                      << lasheader.z_scale_factor;
            LOG(INFO) << "offset: " <<
                      lasheader.x_offset << " " <<
                      lasheader.y_offset << " " <<
                      lasheader.z_offset;

            // we need a new LAS point type for adding RGB
            PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
            if (colors) {
                lasheader.point_data_format = 3;
                lasheader.point_data_record_length = 34;  // 28 + 6
            } else {
                lasheader.point_data_format = 1;
                lasheader.point_data_record_length = 28;
            }

            // init point
            LASpoint laspoint;
            laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

            // open laswriter
            LASwriter *laswriter = laswriteopener.open(&lasheader);
            if (!laswriter) {
                LOG(ERROR) << "could not save file";
                return false;
            }

            // write points
            if (colors) {
                for (auto v : cloud->vertices()) {
                    const vec3 &p = points[v];
                    laspoint.coordinates[0] = p[0];
                    laspoint.coordinates[1] = p[1];
                    laspoint.coordinates[2] = p[2];

                    // populate the point
                    laspoint.compute_XYZ();

                    const vec3 &c = colors[v];
                    laspoint.set_R(static_cast<unsigned short>(c[0] * USHRT_MAX));
                    laspoint.set_G(static_cast<unsigned short>(c[1] * USHRT_MAX));
                    laspoint.set_B(static_cast<unsigned short>(c[2] * USHRT_MAX));

                    laspoint.set_gps_time(0.0006 * v.idx());

                    // write the point
                    laswriter->write_point(&laspoint);

                    // add it to the inventory
                    laswriter->update_inventory(&laspoint);
                }
            } else {
                // if the model doesn't have color, I store the height values as the intensity
                const Box3 &box = cloud->bounding_box();
                const float ht = box.range(2);
                for (auto v : cloud->vertices()) {
                    const vec3 &p = points[v];
                    laspoint.coordinates[0] = p[0];
                    laspoint.coordinates[1] = p[1];
                    laspoint.coordinates[2] = p[2];

                    // populate the point
                    laspoint.compute_XYZ();
                    laspoint.set_intensity(static_cast<unsigned short>((p[2] - box.min_coord(2)) / ht * 255));
                    laspoint.set_gps_time(0.0006 * v.idx());

                    // write the point
                    laswriter->write_point(&laspoint);

                    // add it to the inventory
                    laswriter->update_inventory(&laspoint);
                }
            }

            // update the header
            laswriter->update_header(&lasheader, TRUE);

            // close the writer
            I64 total_bytes = laswriter->close();
            LOG(INFO) << total_bytes << " bytes for " << laswriter->npoints << " points";
            delete laswriter;

            return laswriter->npoints > 0;
        }

    }
    // \endcond
}
