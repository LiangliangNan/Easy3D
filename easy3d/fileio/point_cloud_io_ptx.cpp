/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <easy3d/fileio/point_cloud_io_ptx.h>

#include <cassert>
#include <fstream>

#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file.h>
#include <easy3d/util/line_stream.h>


namespace easy3d {

	namespace io {

		PointCloudIO_ptx::PointCloudIO_ptx(const std::string& file_name)
			: input_(nullptr)
			, in_(nullptr)
			, file_name_(file_name)
			, cloud_index_(0)
		{
		}


		PointCloudIO_ptx::~PointCloudIO_ptx() {
			if (input_)
				delete input_;
			if (in_)
				delete in_;
		}

		// read a single point from the file
		PointCloud* PointCloudIO_ptx::load_next() {
			if (in_ == nullptr) {
				input_ = new std::ifstream(file_name_.c_str());
				if (input_->fail()) {
                    std::cerr << "could not open file \'" << file_name_ << "\'" << std::endl;
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

				in.get_line();
				if (in.eof())
					return nullptr;
				in >> width;

				if (in.eof() || in.fail()) {
					if (cloud_index_ == 0)
						std::cerr << "wrong file format" << std::endl;
					return nullptr;
				}
				if (height == 0 || width == 0) {
					std::cerr << "height == 0 || width == 0" << std::endl;
					return nullptr;
				}

				num = width * height;
				std::cout << "Loading scan #" << cloud_index_ << ": " << num << " points " << std::endl;

				//read sensor transformation matrix
				vec3 v3[4];
				for (int i = 0; i < 4; ++i) {
					in.get_line();
					in >> v3[i];
					if (in.fail()) {
						if (cloud_index_ == 0)
							std::cerr << "wrong file format" << std::endl;
						return nullptr;
					}
				}
				sensorTransD = mat4(vec4(v3[1], 0), vec4(v3[2], 0), vec4(v3[3], 0), vec4(v3[0], 1)); // the first vector is the scanner position

				//read cloud transformation matrix
				vec4 v4[4];
				for (int i = 0; i < 4; ++i) {
					in.get_line();
					in >> v4[i];
					if (in.fail()) {
						if (cloud_index_ == 0)
							std::cerr << "wrong file format" << std::endl;
						return nullptr;
					}
				}
				cloudTransD = mat4(v4[0], v4[1], v4[2], v4[3]);	// transposed in the file (i.e., last row is the translation)
			}

			//now we can read the grid cells
			PointCloud* cloud = new PointCloud;
			std::string cloud_name = file::base_name(file_name_) + "-#" + std::to_string(cloud_index_);
			cloud->set_name(cloud_name);

			PointCloud::VertexProperty<vec3> colors;;

			// read the first line, to test if has color information
			float intensity;
			vec3 p;
			in.get_line();
			in >> p >> intensity;
			if (in.fail()) {
				if (cloud_index_ == 0)
					std::cerr << "wrong file format" << std::endl;
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

			//	ProgressLogger progress(num);
			for (unsigned int i = 1; i < num; ++i) {
				//		if (progress.is_canceled()) {
				//			delete cloud;
				//          return nullptr;
				//		}
				//		progress.notify(i);

				in.get_line();
				in >> p >> intensity;
				v = cloud->add_vertex(cloudTransD * p);

				if (colors) {
					vec3 c;
					in >> c;
					colors[v] = c / 255.0f;
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
