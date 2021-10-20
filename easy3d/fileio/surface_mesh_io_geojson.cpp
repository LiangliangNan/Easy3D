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

#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/json/json.hpp>


// for convenience
using json = nlohmann::json;

namespace easy3d {

    namespace io {

        namespace details {

            // helper for extracting the coordinates of a polygon
            void extract_polygon(json::const_iterator it_coordinates, std::vector<double> &coordinates) {
                for (auto it = it_coordinates->begin(); it != it_coordinates->end(); ++it) {
                    if (it->is_array())
                        extract_polygon(it, coordinates);
                    else if (it->is_number_float())
                        coordinates.push_back(*it);
                }

                // remove very close points
                std::vector<double> results;
                double cur_x = 0;
                double cur_y = 0;
                for (auto i = 0; i < coordinates.size(); i += 2) {
                    if (i + 1 >= coordinates.size())
                        continue;
                    if (i == 0) {
                        cur_x = coordinates[i];
                        cur_y = coordinates[i + 1];
                        results.push_back(cur_x);
                        results.push_back(cur_y);
                    } else {
                        double x = coordinates[i];
                        double y = coordinates[i + 1];
                        if (std::abs(x - cur_x) > 1e-5 || std::abs(y - cur_y) > 1e-5) {
                            cur_x = x;
                            cur_y = y;
                            results.push_back(cur_x);
                            results.push_back(cur_y);
                        } else {
                            std::cout << "duplicated points: (" << x << ", " << y << ")" << std::endl;
                        }
                    }
                }
                coordinates = results;
            }
        }


        /// Reads Geojson format files using the single header file JSON library: https://github.com/nlohmann/json
        /// 2D polygons are stored as faces of a 3D surface mesh (all Z-coordinates are set to 0).
        /// If offset_wrt_first_point is true, apply a translation using the first point in the file.
        bool load_geojson(const std::string &file_name, SurfaceMesh *mesh, bool offset_wrt_first_point) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            if (!file_system::is_file(file_name)) {
                LOG(ERROR) << "file does not exist: " << file_name;
                return false;
            }

            std::ifstream in(file_name.c_str());
            if (in.fail()) {
                LOG(ERROR) << "Could not open file: " << file_name;
                return false;
            }

            mesh->clear();

            LOG(WARNING)
                    << "Note: only polygons are extracted from the GeoJSON file and all other attributes are ignored";

            // read a JSON file
            json object;
            in >> object;

            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();

            auto it_features = object.find("features");
            if (it_features == object.end())
                return false;
            if (!it_features->is_array())
                return false;

            bool first_point = true;
            double dx(0.0), dy(0.0), dz(0.0);
            for (std::size_t i = 0; i < it_features->size(); ++i) {
                const json &ei = it_features->at(i);
                if (!ei.is_object() || ei["type"] != "Feature")
                    continue;

                auto it_geometry = ei.find("geometry"); // you can also use if (iter.key() == "geometry")
                if (it_geometry == ei.end() || !it_geometry->is_object())
                    continue;

                auto it_coordinates = it_geometry->find("coordinates");
                if (it_coordinates == it_geometry->end() || !it_coordinates->is_array())
                    continue;

                auto it_type = it_geometry->find("type");
                if (it_type == it_geometry->end() || !it_type.value().is_string())
                    continue;

                if (it_type.value() == "MultiPolygon" || it_type.value() == "Polygon") {
                    std::vector<double> coordinates;
                    details::extract_polygon(it_coordinates, coordinates);

                    std::vector<SurfaceMesh::Vertex> face;
                    for (std::size_t j = 0; j < coordinates.size(); j += 2) {
                        if (first_point && offset_wrt_first_point) {
                            dx = coordinates[j];
                            dy = coordinates[j + 1];
                            first_point = false;
                        }

                        face.emplace_back(
                                builder.add_vertex(vec3(coordinates[j] - dx, coordinates[j + 1] - dy, 0))
                        );
                    }
                    builder.add_face(face);
                }
            }

            builder.end_surface();

            if (offset_wrt_first_point)
                LOG(WARNING)
                        << "Model offset w.r.t. the first point: " << dvec3(dx, dy, dz);

            return mesh->n_faces() > 0;
        }

    }

}