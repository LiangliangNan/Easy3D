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

#include <cassert>
#include <algorithm>
#include <climits>  // for USHRT_MAX

#include <easy3d/core/point_cloud.h>
#include <3rd_party/LAStools/LASlib/inc/lasreader.hpp>
#include <3rd_party/LAStools/LASlib/inc/laswriter.hpp>


namespace easy3d {


	namespace io {


		bool load_las(const std::string& file_name, PointCloud* cloud)
		{
			LASreadOpener lasreadopener;
			lasreadopener.set_file_name(file_name.c_str(), true);

			LASreader* lasreader = lasreadopener.open();
			if (!lasreader || lasreader->npoints <= 0) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				lasreader->close();
				delete lasreader;
				return false;
			}

			std::size_t num = lasreader->npoints;
			std::cout << "reading " << num << " points..." << std::endl;

			// read the first point
			if (!lasreader->read_point()) {
				std::cerr << "failed reading point" << std::endl;
				lasreader->close();
				delete lasreader;
				return false;
			}

			PointCloud::VertexProperty<vec3> colors = cloud->add_vertex_property<vec3>("v:color");
			PointCloud::VertexProperty<int>  classification = cloud->add_vertex_property<int>("v:classification");

			// Liangliang: las format usually represent very large area of urban scenes and some coordinates
			//			   may have very large values. In order to render the points properly in OpenGL, I
			//			   record the relative positions to the first point stored in the file.
			LASpoint& p0 = lasreader->point;
			float r, g, b;
			if (p0.have_rgb) { // some file may have rgb
				r = float(p0.get_R()) / USHRT_MAX;
				g = float(p0.get_G()) / USHRT_MAX;
				b = float(p0.get_B()) / USHRT_MAX;
			}
			else // in case color doesn't exist, use intensity
				r = g = b = p0.intensity % 255 / 255.0f;

			PointCloud::Vertex v = cloud->add_vertex(vec3(0, 0, 0));
			colors[v] = vec3(r, g, b);
			classification[v] = p0.classification;

			// compute the actual coordinates as double floating point values
			p0.compute_coordinates();
			double x0 = p0.coordinates[0];
			double y0 = p0.coordinates[1];
			double z0 = p0.coordinates[2];
			std::cout << "first point (" << x0 << " " << y0 << " " << z0 << ")" << std::endl;

			// now we read the remaining points...
			while (lasreader->read_point()) {
				LASpoint& p = lasreader->point;

				// compute the actual coordinates as double floating point values
				p.compute_coordinates();
				double x = p.coordinates[0] - x0;
				double y = p.coordinates[1] - y0;
				double z = p.coordinates[2] - z0;

				float r, g, b;
				if (p.have_rgb) { // some file may have rgb
					r = float(p.get_R()) / USHRT_MAX;
					g = float(p.get_G()) / USHRT_MAX;
					b = float(p.get_B()) / USHRT_MAX;
				}
				else // in case color doesn't exist, use intensity
					r = g = b = p.intensity % 255 / 255.0f;

				v = cloud->add_vertex(vec3(float(x), float(y), float(z)));
				colors[v] = vec3(r, g, b);
				classification[v] = p.classification;
			}

			lasreader->close();
			delete lasreader;
			return cloud->n_vertices() > 0;
		}


		bool save_las(const std::string& file_name, const PointCloud* cloud) {
			if (!cloud) {
				std::cerr << "null input point cloud pointer" << std::endl;
				return false;
			}

			PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
			if (normals)
				std::cerr << "normals discarded when saving to LAS or LAZ format (future release may support)." << std::endl;

			LASwriteOpener laswriteopener;
			laswriteopener.set_file_name(file_name.c_str());

			// check output
			if (!laswriteopener.active()) {
				std::cerr << "could not save file" << std::endl;
				return false;
			}

			PointCloud::VertexProperty<vec3> points = cloud->get_vertex_property<vec3>("v:point");
			const Box3& box = cloud->bounding_box();
			const vec3& center = box.center();
			std::cout << "saving " << cloud->n_vertices() << " points..." << std::endl;

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
			lasheader.x_offset = center.x;	// box.x_min(); should also work
			lasheader.y_offset = center.y;	// box.y_min(); should also work
			lasheader.z_offset = center.z;	// box.z_min(); should also work

			std::cout << "scale factor: "
				<< lasheader.x_scale_factor << " "
				<< lasheader.y_scale_factor << " "
				<< lasheader.z_scale_factor << std::endl;
			std::cout << "offset: " <<
				lasheader.x_offset << " " <<
				lasheader.y_offset << " " <<
				lasheader.z_offset << std::endl;

			// we need a new LAS point type for adding RGB
			PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
			if (colors) {
				lasheader.point_data_format = 3;
				lasheader.point_data_record_length = 34;  // 28 + 6
			}
			else {
				lasheader.point_data_format = 1;
				lasheader.point_data_record_length = 28;
			}

			// init point 
			LASpoint laspoint;
			laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

			// open laswriter
			LASwriter* laswriter = laswriteopener.open(&lasheader);
			if (!laswriter) {
				std::cerr << "could not save file" << std::endl;
				return false;
			}

			// write points
			if (colors) {
				for (auto v : cloud->vertices()) {
					const vec3& p = points[v];
					laspoint.coordinates[0] = p[0];
					laspoint.coordinates[1] = p[1];
					laspoint.coordinates[2] = p[2];

					// populate the point
					laspoint.compute_XYZ();

					const vec3& c = colors[v];
					laspoint.set_R(static_cast<unsigned short>(c[0] * USHRT_MAX));
					laspoint.set_G(static_cast<unsigned short>(c[1] * USHRT_MAX));
					laspoint.set_B(static_cast<unsigned short>(c[2] * USHRT_MAX));

					laspoint.set_gps_time(0.0006 * v.idx());

					// write the point
					laswriter->write_point(&laspoint);

					// add it to the inventory
					laswriter->update_inventory(&laspoint);
				}
			}
			else {
				// if the model doesn't have color, I store the height values as the intensity
				const Box3& box = cloud->bounding_box();
                const float ht = box.range(2);
				for (auto v : cloud->vertices()) {
					const vec3& p = points[v];
					laspoint.coordinates[0] = p[0];
					laspoint.coordinates[1] = p[1];
					laspoint.coordinates[2] = p[2];

					// populate the point
					laspoint.compute_XYZ();
                    laspoint.set_intensity(static_cast<unsigned short>((p[2] - box.min(2)) / ht * 255));
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
			std::cout << total_bytes << " bytes for " << laswriter->npoints << " points" << std::endl;
			delete laswriter;

			return laswriter->npoints > 0;
		}

	}

}
