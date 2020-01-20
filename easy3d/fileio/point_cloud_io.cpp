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

#include <easy3d/fileio/point_cloud_io.h>

#include <clocale>
#include <fstream>

#include <easy3d/fileio/point_cloud_io_vg.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {


	PointCloud* PointCloudIO::load(const std::string& file_name)
	{
		std::setlocale(LC_NUMERIC, "C");

		PointCloud* cloud = new PointCloud;
		cloud->set_name(file_name);

		StopWatch w;
		bool success = false;

        const std::string& ext = file_system::extension(file_name, true);
        if (ext == "ply")
			success = io::load_ply(file_name, cloud);
		else if (ext == "bin")
			success = io::load_bin(file_name, cloud);
		else if (ext == "xyz")
			success = io::load_xyz(file_name, cloud);
		else if (ext == "bxyz")
			success = io::load_bxyz(file_name, cloud);
		else if (ext == "las" || ext == "laz")
			success = io::load_las(file_name, cloud);
        else if (ext == "vg")
            success = io::PointCloudIO_vg::load_vg(file_name, cloud);
        else if (ext == "bvg")
            success = io::PointCloudIO_vg::load_bvg(file_name, cloud);

        else if (ext.empty()){
            std::cerr << "unknown file format: no extention" << ext << std::endl;
            success = false;
        }
		else {
			std::cerr << "unknown file format: " << ext << std::endl;
			success = false;
		}

		if (!success || cloud->n_vertices() == 0) {
			delete cloud;
			return nullptr;
		}

        if (success)
            std::cout << "point cloud loaded ("
                      << "#v: " << cloud->vertices_size() << "). Time: "
                      << w.time_string() << std::endl;
        else
            std::cout << "load point cloud failed." << std::endl;

		return cloud;
	}


	bool PointCloudIO::save(const std::string& file_name, const PointCloud* cloud) {
		if (!cloud) {
			std::cerr << "Point cloud is null" << std::endl;
			return false;
		}

        StopWatch w;
        bool success = false;

        std::string final_name = file_name;
        const std::string& ext = file_system::extension(file_name, true);
        if (ext == "ply" || ext.empty()) {
            if (ext.empty()) {
                std::cerr << "No extention specified. Default to ply" << ext << std::endl;
                final_name = final_name + ".ply";
            }
            success = io::save_ply(final_name, cloud, true);
        }
		else if (ext == "bin")
            success = io::save_bin(final_name, cloud);
		else if (ext == "xyz")
            success = io::save_xyz(final_name, cloud);
		else if (ext == "bxyz")
            success = io::save_bxyz(final_name, cloud);
		else if (ext == "las" || ext == "laz")
            success = io::save_las(final_name, cloud);
        else if (ext == "vg")
            success = io::PointCloudIO_vg::save_vg(file_name, cloud);
        else if (ext == "bvg")
            success = io::PointCloudIO_vg::save_bvg(file_name, cloud);
		else {
			std::cerr << "unknown file format: " << ext << std::endl;
            success = false;
		}

        if (success) {
            std::cout << "save model done. time: " << w.time_string() << std::endl;
            return true;
        }
        else {
            std::cout << "save model failed" << std::endl;
            return false;
        }

	}

} // namespace easy3d
