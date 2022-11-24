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

#include <easy3d/fileio/point_cloud_io_ptx.h>

#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/progress.h>


namespace easy3d {

	namespace io {

        /// TODO: Translator not implemented

		PointCloudIO_ptx::PointCloudIO_ptx(const std::string& file_name)
			: input_(nullptr)
			, in_(nullptr)
			, file_name_(file_name)
			, cloud_index_(0)
		{
		}


        PointCloudIO_ptx::~PointCloudIO_ptx() {
            delete input_;
            delete in_;
        }


		// read a single point from the file
		PointCloud* PointCloudIO_ptx::load_next() {
			if (in_ == nullptr) {
				input_ = new std::ifstream(file_name_.c_str());
				if (input_->fail()) {
				    LOG(ERROR) << "could not open file: " << file_name_;
					return nullptr;
				}
				in_ = new LineInputStream(*input_);
			}

			unsigned int num = 0;
			mat4 sensorTransD, cloudTransD;

			LineInputStream in = *in_;
			//read header
			{
				unsigned int width = 0, height = 0;
				in.get_line();
                if (in.eof())
                    return nullptr;

				in >> height;
                if (in.fail() || in.eof()) {
                    LOG_N_TIMES(3, ERROR) << "failed reading \'height\' from file header. " << COUNTER;
                }

				in.get_line();
                if (in.fail() || in.eof()) {
                    LOG_N_TIMES(3, ERROR) << "failed reading file header. Probably wrong file format. " << COUNTER;
                    return nullptr;
                }

				in >> width;
                if (in.fail() || in.eof()) {
                    LOG_N_TIMES(3, ERROR) << "failed reading \'width\' from file header. " << COUNTER;
                    return nullptr;
                }
				if (height == 0 || width == 0) {
                    LOG_N_TIMES(3, ERROR) << "unrecognized file format: height == 0 || width == 0. " << COUNTER;
					return nullptr;
				}

				num = width * height;
                const std::string simple_name = file_system::simple_name(file_name_) + "-#" + std::to_string(cloud_index_);
                LOG(INFO) << "loading sub scan " << simple_name  << " with " << num << " points...";

				//read sensor transformation matrix
				vec3 v3[4];
				for (auto& v : v3) {
					in.get_line();
					in >> v;
					if (in.fail()) {
                        LOG_N_TIMES(3, ERROR) << "failed reading sensor transformation matrix. " << COUNTER;
						return nullptr;
					}
				}
				sensorTransD = mat4(vec4(v3[1], 0), vec4(v3[2], 0), vec4(v3[3], 0), vec4(v3[0], 1)); // the first vector is the scanner position

				//read cloud transformation matrix
				vec4 v4[4];
				for (auto& v : v4) {
					in.get_line();
					in >> v;
					if (in.fail()) {
                        LOG_N_TIMES(3, ERROR) << "failed reading point cloud transformation matrix. " << COUNTER;
						return nullptr;
					}
				}
				cloudTransD = mat4(v4[0], v4[1], v4[2], v4[3]);	// transposed in the file (i.e., last row is the translation)
			}

			//now we can read the grid cells
			auto cloud = new PointCloud;
            const std::string& cloud_name = file_system::name_less_extension(file_name_) + "-#" + std::to_string(cloud_index_);
			cloud->set_name(cloud_name);

			PointCloud::VertexProperty<vec3> colors;

			// read the first line to test if it has color information
			float intensity;
			vec3 p;
			in.get_line();
			in >> p >> intensity;
			if (in.fail()) {
                LOG_N_TIMES(3, ERROR) << "failed reading the first point. " << COUNTER;
				delete cloud;
				return nullptr;
			}
			PointCloud::Vertex v = cloud->add_vertex(cloudTransD * p);// apply the transformation

			if (!in.eol()) {
				vec3 c;
				in >> c;
				if (!in.fail()) { // has color
					colors = cloud->add_vertex_property<vec3>("v:color");
					colors[v] = c / 255.0f;
				}
			}

			ProgressLogger progress(num, true, false);
			for (unsigned int i = 1; i < num; ++i) {
                if (progress.is_canceled()) {
                    LOG(WARNING) << "loading point cloud file cancelled";
                    delete cloud;
                    return nullptr;
                }
                progress.notify(i);

				in.get_line();
				in >> p >> intensity;
                if (!in.fail())
                    v = cloud->add_vertex(cloudTransD * p);
                else {
                    LOG_N_TIMES(3, ERROR) << "failed reading the " << i << "_th point. " << COUNTER;
                    delete cloud;
                    return nullptr;
                }

				if (colors) {
					vec3 c;
					in >> c;
                    if (!in.fail())
                        colors[v] = c / 255.0f;
                    else {
                        LOG_N_TIMES(3, ERROR) << "failed reading color of the " << i << "_th point. " << COUNTER;
                        delete cloud;
                        return nullptr;
                    }
				}
			}

			if (cloud->n_vertices() > 1) {
				++cloud_index_;
				return cloud;
			}
			else {
				delete cloud;
				return nullptr;
			}
		}

	} // namespace io

} // namespace easy3d
