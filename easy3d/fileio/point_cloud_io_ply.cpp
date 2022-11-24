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
#include <easy3d/fileio/translator.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/types.h>
#include <easy3d/util/logging.h>


namespace easy3d {

	namespace io {

        namespace internal {

			template <typename T, typename PropertyT>
			inline void add_properties(PointCloud* cloud, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
					if (name.find("v:") == std::string::npos)
						name = "v:" + name;
					auto prop = cloud->vertex_property<T>(name);
					prop.vector() = p;
				}
			}

		} // namespace internal

		bool load_ply(const std::string& file_name, PointCloud* cloud) {
			std::vector<Element> elements;
			PlyReader reader;
			if (!reader.read(file_name, elements))
				return false;

            for (const auto& e : elements) {
                if (e.name == "vertex") {
                    cloud->resize(static_cast<unsigned int>(e.num_instances));
                    break;
                }
            }

            for (const auto& e : elements) {
                if (e.name == "vertex") {
                    internal::add_properties<vec3>(cloud, e.vec3_properties);
                    internal::add_properties<vec2>(cloud, e.vec2_properties);
                    internal::add_properties<float>(cloud, e.float_properties);
                    internal::add_properties<int>(cloud, e.int_properties);
                    internal::add_properties< std::vector<int> >(cloud, e.int_list_properties);
                    internal::add_properties< std::vector<float> >(cloud, e.float_list_properties);
                }
                else {
                    const std::string name = "element-" + e.name;
                    auto prop = cloud->add_model_property<Element>(name, Element(""));
                    prop.vector().push_back(e);
                    LOG(WARNING) << "unknown element '" << e.name
                                 << "' with the following properties has been stored as a model property '" << name << "'"
                                 << e.property_statistics();
                }
			}

            if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                auto& points = cloud->get_vertex_property<vec3>("v:point").vector();

                // the first point
                const vec3 p0 = points[0];
                const dvec3 origin(p0.data());
                Translator::instance()->set_translation(origin);

                for (auto& p: points)
                    p -= p0;

                auto trans = cloud->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. the first vertex (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                const dvec3 &origin = Translator::instance()->translation();
                auto& points = cloud->get_vertex_property<vec3>("v:point").vector();
                for (auto& p: points) {
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


		namespace internal {

			template <typename T>
			inline void collect_properties(const PointCloud* cloud, std::vector< GenericProperty<T> >& properties) {
				const auto& all_properties = cloud->vertex_properties();
				for (auto name : all_properties) {
					auto prop = cloud->get_vertex_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "v:")
							name = name.substr(2, name.length() - 1);
						properties.emplace_back(GenericProperty<T>(name, prop.vector()));
					}
				}
			}

		} // namespace internal

		bool save_ply(const std::string& file_name, const PointCloud* cloud, bool binary/* = true*/) {
			if (!cloud || cloud->n_vertices() == 0) {
				LOG(ERROR) << "empty point cloud data";
				return false;
			}

			std::size_t num = cloud->n_vertices();
			Element element_vertex("vertex", num);

			internal::collect_properties(cloud, element_vertex.vec3_properties);
            internal::collect_properties(cloud, element_vertex.vec2_properties);
			internal::collect_properties(cloud, element_vertex.float_properties);
			internal::collect_properties(cloud, element_vertex.int_properties);
			internal::collect_properties(cloud, element_vertex.int_list_properties);
			internal::collect_properties(cloud, element_vertex.float_list_properties);

            auto trans = cloud->get_model_property<dvec3>("translation");
            if (trans) { // has translation
                const dvec3& origin = trans[0];
                for (auto& prop : element_vertex.vec3_properties) {
                    if (prop.name == "point") {
                        for (auto& v : prop) {
                            v.x += static_cast<float>(origin.x);
                            v.y += static_cast<float>(origin.y);
                            v.z += static_cast<float>(origin.z);
                        }
                    }
                }
            }

			std::vector<Element> elements;
            elements.emplace_back(element_vertex);

            binary = binary && (file_name.find("ascii") == std::string::npos);
            LOG_IF(!binary, WARNING) << "you're writing an ASCII ply file. Use binary format for better performance";

            return PlyWriter::write(file_name, elements, "", binary);
		}


	} // namespace io

} // namespace easy3d
