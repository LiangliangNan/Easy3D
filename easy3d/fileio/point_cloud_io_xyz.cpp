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


#include <easy3d/fileio/point_cloud_io.h>

#include <fstream>

#include <easy3d/core/point_cloud.h>
#include <easy3d/util/line_stream.h>


namespace easy3d {

	namespace io {

		bool load_xyz(const std::string& file_name, PointCloud* cloud) {
			std::ifstream input(file_name.c_str());
			if (input.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				return false;
			}

			io::LineInputStream in(input);

			// get length of file
			// input.seekg(0, input.end);
			// std::streamoff length = input.tellg();
			// input.seekg(0, input.beg);
			// ProgressLogger progress(length);

			vec3 p;
			while (!input.eof()) {
				in.get_line();;
				if (in.current_line()[0] != '#') {
					in >> p;
					if (!in.fail()) {
						cloud->add_vertex(p);

						//  std::streamoff pos = input.tellg();
						//	progress.notify(pos);
					}
				}
			}

			return true;
		}


		bool save_xyz(const std::string& file_name, const PointCloud* cloud) {
			std::ofstream output(file_name.c_str());
			if (output.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				return false;
			}
			output.precision(16);

			PointCloud::VertexProperty<vec3> points = cloud->get_vertex_property<vec3>("v:point");

			//   ProgressLogger progress(cloud->n_vertices());
			for (auto v : cloud->vertices()) {
				output << points[v] << std::endl;
				//		progress.next();
			}

			return true;
		}


		bool load_bxyz(const std::string& file_name, PointCloud* cloud) {
			std::ifstream input(file_name.c_str(), std::fstream::binary);
			if (input.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				return false;
			}

			std::size_t element_per_point = 3;
			std::size_t element_size = sizeof(float) * element_per_point;
			// check size of types
			if (sizeof(vec3) != sizeof(float) * 3) {
				std::cerr << "point coordinates must be \'float\'" << std::endl;
				return false;
			}

			// get length of file
			input.seekg(0, input.end);
			std::streamoff length = input.tellg();
			input.seekg(0, input.beg);

			// num of points in the file
			std::size_t num = length / element_size;
			if (num <= 0)
				return false;

			cloud->resize(static_cast<unsigned int>(num));

			PointCloud::VertexProperty<vec3> points = cloud->get_vertex_property<vec3>("v:point");

			input.read((char*)points.data(), num * element_size);	// read the entire blocks
			if (input.fail()) {
				std::cerr
					<< input.gcount() << " bytes of the block could not be read" << std::endl;
				return false;
			}
			return true;
		}


		bool save_bxyz(const std::string& file_name, const PointCloud* cloud) {
			// open file
			std::ofstream output(file_name.c_str(), std::fstream::binary);
			if (output.fail()) {
				std::cerr << "could not open file\'" << file_name << "\'" << std::endl;
				return false;
			}

			// check size of types
			if (sizeof(vec3) != sizeof(float) * 3) {
				std::cerr << "point coordinates must be \'float\'" << std::endl;
				return false;
			}

			PointCloud::VertexProperty<vec3> points = cloud->get_vertex_property<vec3>("v:point");

			output.write((const char*)points.data(), sizeof(vec3) * cloud->n_vertices());
			return true;
		}

	} // namespace io

} // namespace easy3d
