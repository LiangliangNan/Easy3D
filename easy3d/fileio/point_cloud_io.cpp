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

#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/point_cloud_io_vg.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {


	PointCloud* PointCloudIO::load(const std::string& file_name)
	{
		auto cloud = new PointCloud;
		cloud->set_name(file_name);

		StopWatch w;
		bool success = false;

        const std::string &ext = file_system::extension(file_name, true);
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

        else if (ext.empty()) {
            LOG(ERROR) << "unknown file format: no extension";
            success = false;
        } else {
            LOG(ERROR) << "unknown file format: " << ext;
            success = false;
        }

		if (!success || cloud->n_vertices() == 0) {
            LOG(WARNING) << "load point cloud failed: " << file_name;
			delete cloud;
			return nullptr;
		}

        LOG(INFO) << "point cloud loaded ("
                  << "#vertex: " << cloud->n_vertices() << "). "
                  << w.time_string();

		return cloud;
	}


	bool PointCloudIO::save(const std::string& file_name, const PointCloud* cloud) {
		if (!cloud) {
			LOG(ERROR) << "Point cloud is null";
			return false;
		}

        StopWatch w;
        bool success = false;

        std::string final_name = file_name;
        const std::string &ext = file_system::extension(file_name, true);
        if (ext == "ply" || ext.empty()) {
            if (ext.empty()) {
                LOG(ERROR) << "No extension specified. Default to ply";
                final_name = final_name + ".ply";
            }
            success = io::save_ply(final_name, cloud, true);
        } else if (ext == "bin")
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
            LOG(ERROR) << "unknown file format: " << ext;
            success = false;
        }

        if (success) {
            LOG(INFO) << "save model done. " << w.time_string();
            return true;
        }
        else {
            LOG(INFO) << "save model failed";
            return false;
        }

	}

} // namespace easy3d
