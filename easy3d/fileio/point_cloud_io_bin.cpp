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


namespace easy3d {


	namespace io {


		// three blocks storing points, colors (optional), and normals (optional)
		bool load_bin(const std::string& file_name, PointCloud* cloud) {
			std::ifstream input(file_name.c_str(), std::fstream::binary);
			if (input.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				return false;
			}

            unsigned int num = 0;
            input.read((char*)(&num), sizeof(unsigned int));
			if (num <= 0) {
				std::cerr << "no point exists in file\'" << file_name << "\'" << std::endl;
				return false;
			}
			cloud->resize(num);

			// read the points block
			PointCloud::VertexProperty<vec3> points = cloud->vertex_property<vec3>("v:point");
			input.read((char*)points.data(), num * sizeof(vec3));

			// read the colors block if exists
            input.read((char*)(&num), sizeof(unsigned int));
			if (num > 0) {
				PointCloud::VertexProperty<vec3> colors = cloud->vertex_property<vec3>("v:color");
				input.read((char*)colors.data(), num * sizeof(vec3));
			}

			// read the normals block if exists
            input.read((char*)(&num), sizeof(unsigned int));
			if (num > 0) {
				PointCloud::VertexProperty<vec3> normals = cloud->vertex_property<vec3>("v:normal");
				input.read((char*)normals.data(), num * sizeof(vec3));
			}

			return cloud->n_vertices() > 0;
		}


		bool save_bin(const std::string& file_name, const PointCloud* cloud) {
			// open file
			std::ofstream output(file_name.c_str(), std::fstream::binary);
			if (output.fail()) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				return false;
			}

            unsigned int num = cloud->vertices_size();

			// write the points block
			auto points = cloud->get_vertex_property<vec3>("v:point");
            output.write((char*)&num, sizeof(unsigned int));
			output.write((char*)points.data(), num * sizeof(vec3));

			auto colors = cloud->get_vertex_property<vec3>("v:color");
			if (colors) {
                output.write((char*)&num, sizeof(unsigned int));
				output.write((char*)colors.data(), num * sizeof(vec3));
			}
            else {
                num = 0;
                output.write((char*)&num, sizeof(unsigned int));
            }

			auto normals = cloud->get_vertex_property<vec3>("v:normal");
			if (normals) {
                output.write((char*)&num, sizeof(unsigned int));
				output.write((char*)normals.data(), num * sizeof(vec3));
			}
            else {
                num = 0;
                output.write((char*)&num, sizeof(unsigned int));
            }

			return true;
		}

	} // namespace io

} // namespace easy3d
