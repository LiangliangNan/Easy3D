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

#include <fstream>

#include <easy3d/fileio/translator.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/progress.h>


namespace easy3d {

    // \cond
	namespace io {

		bool load_xyz(const std::string& file_name, PointCloud* cloud) {
			std::ifstream input(file_name.c_str());
			if (input.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
				return false;
			}

			io::LineInputStream in(input);

			// get length of file
            input.seekg(0, input.end);
            std::streamoff length = input.tellg();
            input.seekg(0, input.beg);
            ProgressLogger progress(length, true, false);

			dvec3 v;
            std::vector<dvec3> points;
			while (!input.eof()) {
			    if (progress.is_canceled()) {
                    LOG(WARNING) << "saving point cloud file cancelled";
                    return false;
                }
				in.get_line();
				if (in.current_line()[0] != '#') {
					in >> v;
					if (!in.fail()) {
                        points.push_back(v);
                        std::streamoff pos = input.tellg();
                        progress.notify(pos);
					}
				}
			}

            if (Translator::instance()->status() == Translator::DISABLED) {
                for (const auto& p : points)
                    cloud->add_vertex(vec3(p.data()));
            }
            else if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                dvec3 origin = points[0];
                Translator::instance()->set_translation(origin);

                for (const auto &p: points)
                    cloud->add_vertex(vec3(static_cast<float>(p.x - origin.x), static_cast<float>(p.y - origin.y), static_cast<float>(p.z - origin.z)));
                auto trans = cloud->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. the first vertex (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            }
            else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                const dvec3 &origin = Translator::instance()->translation();
                for (const auto &p: points)
                    cloud->add_vertex(vec3(static_cast<float>(p.x - origin.x), static_cast<float>(p.y - origin.y), static_cast<float>(p.z - origin.z)));
                auto trans = cloud->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. last known reference point (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            }

            return cloud->n_vertices() > 0;
		}


		bool save_xyz(const std::string& file_name, const PointCloud* cloud) {
			std::ofstream output(file_name.c_str());
			if (output.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
				return false;
			}
			output.precision(16);

			auto points = cloud->get_vertex_property<vec3>("v:point");
            auto trans = cloud->get_model_property<dvec3>("translation");

            ProgressLogger progress(cloud->n_vertices(), true, false);
            if (trans) { // has translation
                const dvec3 &origin = trans[0];
                for (auto v: cloud->vertices()) {
                    if (progress.is_canceled()) {
                        LOG(WARNING) << "saving point cloud file cancelled";
                        return false;
                    }
                    output << points[v].x + origin.x << " "
                           << points[v].y + origin.y << " "
                           << points[v].z + origin.z << std::endl;
                    progress.next();
                }
            }
            else {
                for (auto v: cloud->vertices()) {
                    if (progress.is_canceled()) {
                        LOG(WARNING) << "saving point cloud file cancelled";
                        return false;
                    }
                    output << points[v] << std::endl;
                    progress.next();
                }
            }

			return true;
		}


 		bool load_bxyz(const std::string& file_name, PointCloud* cloud) {
			std::ifstream input(file_name.c_str(), std::fstream::binary);
			if (input.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
				return false;
			}

			std::size_t element_per_point = 3;
			std::size_t element_size = sizeof(float) * element_per_point;

			// get length of file
			input.seekg(0, input.end);
			std::streamoff length = input.tellg();
			input.seekg(0, input.beg);

			// num of points in the file
			std::size_t num = length / element_size;
			if (num <= 0)
				return false;

			cloud->resize(static_cast<unsigned int>(num));
			auto points = cloud->get_vertex_property<vec3>("v:point");

            input.read((char *) points.data(), static_cast<long>(num * element_size));    // read the entire blocks
            if (input.fail()) {
                LOG(ERROR) << input.gcount() << " bytes of the block could not be read";
                return false;
            }

            if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                auto& positions = points.vector();

                // the first point
                const vec3 p0 = positions[0];
                const dvec3 origin(p0.data());
                Translator::instance()->set_translation(origin);

                for (auto& p : positions)
                    p -= p0;

                auto trans = cloud->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. the first vertex (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                const dvec3 &origin = Translator::instance()->translation();
                auto& pts = cloud->get_vertex_property<vec3>("v:point").vector();
                for (auto& p: pts) {
                    p.x -= static_cast<float>(origin.x);
                    p.y -= static_cast<float>(origin.y);
                    p.z -= static_cast<float>(origin.z);
                }

                auto trans = cloud->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. last known reference point (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            }

			return cloud->n_vertices() > 0;
		}


		bool save_bxyz(const std::string& file_name, const PointCloud* cloud) {
			// open file
			std::ofstream output(file_name.c_str(), std::fstream::binary);
			if (output.fail()) {
				LOG(ERROR) << "could not open file: " << file_name;
				return false;
			}

			auto points = cloud->get_vertex_property<vec3>("v:point");
            auto trans = cloud->get_model_property<dvec3>("translation");
            if (trans) { // has translation
                const dvec3& origin = trans[0];
                for (auto v : cloud->vertices()) {
                    const vec3& p = points[v];
                    for (unsigned short i=0; i<3; ++i) {
                        auto value = static_cast<float>(p[i] + origin[i]);
                        output.write((char *)&value, sizeof(float));
                    }
                }
            }
            else
                output.write((char*)points.data(), static_cast<long>(cloud->n_vertices() * sizeof(vec3)));
			return true;
		}

	} // namespace io
	// \endcond

} // namespace easy3d
