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

#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {

	namespace io {


		namespace details {

			template <typename PropertyT>
			inline bool extract_named_property(std::vector<PropertyT>& properties, PropertyT& wanted, const std::string& name) {
				typename std::vector< PropertyT >::iterator it = properties.begin();
				for (; it != properties.end(); ++it) {
                    const PropertyT& prop = *it;
                    if (prop.name == name) {
                        wanted = prop;
						properties.erase(it);
						return true;
					}
				}
				return false;
			}


			template <typename T, typename PropertyT>
			inline void add_vertex_properties(SurfaceMesh* mesh, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
					if (p.size() != mesh->n_vertices()) {
                        LOG(ERROR) << "vertex property size (" << p.size() << ") does not match number of vertices (" << mesh->n_vertices() << ")";
						continue;
					}
					if (name.find("v:") == std::string::npos)
						name = "v:" + name;
					auto prop = mesh->vertex_property<T>(name);
					prop.vector() = p;
				}
			}


			template <typename T, typename PropertyT>
			inline void add_face_properties(SurfaceMesh* mesh, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
					if (p.size() != mesh->n_faces()) {
                        LOG(ERROR) << "face property size (" << p.size() << ") does not match number of faces (" << mesh->n_faces() << ")";
						continue;
					}
					if (name.find("f:") == std::string::npos)
						name = "f:" + name;
					auto prop = mesh->face_property<T>(name);
					prop.vector() = p;
				}
			}


			template <typename T, typename PropertyT>
			inline void add_edge_properties(SurfaceMesh* mesh, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
					if (p.size() != mesh->n_edges()) {
                        LOG(ERROR) << "edge property size (" << p.size() << ") does not match number of edges (" << mesh->n_edges() << ")";
						continue;
					}
					if (name.find("e:") == std::string::npos)
						name = "e:" + name;
					auto prop = mesh->edge_property<T>(name);
					prop.vector() = p;
				}
			}

		} // namespace details


		bool load_ply(const std::string& file_name, SurfaceMesh* mesh)
		{
			if (!mesh) {
				LOG(ERROR) << "null mesh pointer";
				return false;
			}

			std::vector<Element> elements;
			PlyReader reader;
			if (!reader.read(file_name, elements))
				return false;

			Vec3Property       coordinates;
			IntListProperty    face_vertex_indices;
			IntListProperty    edge_vertex_indices;

			for (std::size_t i = 0; i < elements.size(); ++i) {
				Element& e = elements[i];
                if (e.name == "vertex") {
					if (details::extract_named_property(e.vec3_properties, coordinates, "point"))
						continue;
					else {
						LOG(ERROR) << "vertex coordinates (x, y, z properties) do not exist";
						return false;
					}
				}
                else if (e.name == "face") {
					if (details::extract_named_property(e.int_list_properties, face_vertex_indices, "vertex_indices"))
						continue;
					else {
						LOG(ERROR) << "\'vertex_indices\' does not defined on faces";
						return false;
					}
				}
                else if (e.name == "edge") {
					if (details::extract_named_property(e.int_list_properties, edge_vertex_indices, "vertex_indices"))
						continue;
					else
						LOG(ERROR) << "edge properties might not be parsed correctly because \'vertex_indices\' does not defined on edges";
				}
			}

			mesh->clear();

            ManifoldBuilder builder(mesh);
            builder.begin_surface();

            for (auto p : coordinates)
                builder.add_vertex(p);

			for (auto indices : face_vertex_indices) {
				std::vector<SurfaceMesh::Vertex> vts;
				for (auto id : indices)
                    vts.emplace_back(SurfaceMesh::Vertex(id));
				builder.add_face(vts);
			}

			// now let's add the properties
			for (std::size_t i = 0; i < elements.size(); ++i) {
				Element& e = elements[i];
                if (e.name == "vertex") {
					details::add_vertex_properties<vec3>(mesh, e.vec3_properties);
					details::add_vertex_properties<float>(mesh, e.float_properties);
					details::add_vertex_properties<int>(mesh, e.int_properties);
					details::add_vertex_properties< std::vector<int> >(mesh, e.int_list_properties);
					details::add_vertex_properties< std::vector<float> >(mesh, e.float_list_properties);
				}
                else if (e.name == "face") {
					details::add_face_properties<vec3>(mesh, e.vec3_properties);
					details::add_face_properties<float>(mesh, e.float_properties);
					details::add_face_properties<int>(mesh, e.int_properties);
					details::add_face_properties< std::vector<int> >(mesh, e.int_list_properties);
					details::add_face_properties< std::vector<float> >(mesh, e.float_list_properties);
				}
                else if (e.name == "edge") {
					details::add_edge_properties<vec3>(mesh, e.vec3_properties);
					details::add_edge_properties<float>(mesh, e.float_properties);
					details::add_edge_properties<int>(mesh, e.int_properties);
					details::add_edge_properties< std::vector<int> >(mesh, e.int_list_properties);
					details::add_edge_properties< std::vector<float> >(mesh, e.float_list_properties);
				}
				else {
				    const std::string name = "element-" + e.name;
                    auto prop = mesh->add_model_property<Element>(name, Element(""));
                    prop.vector().push_back(e);
                    LOG(WARNING) << "unknown element '" << e.name
                                 << "' with the following properties has been stored as model property '" << name << "'"
                                 << e.property_statistics();
                }
			}

			builder.end_surface();
			return mesh->n_faces() > 0;
		}


		namespace details {


			template <typename T>
			inline void collect_vertex_properties(const SurfaceMesh* mesh, std::vector< GenericProperty<T> >& properties) {
				const auto& all_properties = mesh->vertex_properties();
				for (auto name : all_properties) {
					auto prop = mesh->get_vertex_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "v:")
							name = name.substr(2, name.length() - 1);
						properties.emplace_back(GenericProperty<T>(name, prop.vector()));
					}
				}
			}

			template <typename T>
			inline void collect_face_properties(const SurfaceMesh* mesh, std::vector< GenericProperty<T> >& properties) {
				const auto& all_properties = mesh->face_properties();
				for (auto name : all_properties) {
					auto prop = mesh->get_face_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "f:")
							name = name.substr(2, name.length() - 1);
						properties.emplace_back(GenericProperty<T>(name, prop.vector()));
					}
				}
			}


			template <typename T>
			inline void collect_edge_properties(const SurfaceMesh* mesh, std::vector< GenericProperty<T> >& properties) {
				const auto& all_properties = mesh->edge_properties();
				for (auto name : all_properties) {
					auto prop = mesh->get_edge_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "e:")
							name = name.substr(2, name.length() - 1);
						properties.emplace_back(GenericProperty<T>(name, prop.vector()));
					}
				}
			}


		} // namespace details


		bool save_ply(const std::string& file_name, const SurfaceMesh* mesh, bool binary) {
			if (!mesh || mesh->n_vertices() == 0 || mesh->n_faces() == 0) {
				LOG(ERROR) << "empty mesh data";
				return false;
			}

			std::vector<Element> elements;

			//-----------------------------------------------------

			// element vertex
			Element element_vertex("vertex", mesh->n_vertices());

			// attributes defined on element "vertex"
			details::collect_vertex_properties(mesh, element_vertex.vec3_properties);
			details::collect_vertex_properties(mesh, element_vertex.float_properties);
			details::collect_vertex_properties(mesh, element_vertex.int_properties);
			details::collect_vertex_properties(mesh, element_vertex.int_list_properties);
			details::collect_vertex_properties(mesh, element_vertex.float_list_properties);

			elements.emplace_back(element_vertex);

			//-----------------------------------------------------

			// element face
			Element element_face("face", mesh->n_faces());

			// vertex_indices
			IntListProperty face_vertex_indices;
            face_vertex_indices.name = "vertex_indices";
			face_vertex_indices.reserve(mesh->n_faces());
			for (auto f : mesh->faces()) {
				std::vector<int> indices;
				for (auto v : mesh->vertices(f))
					indices.push_back(v.idx());
				face_vertex_indices.emplace_back(indices);
			}
			element_face.int_list_properties.emplace_back(face_vertex_indices);

			// attributes defined on element "face"
			details::collect_face_properties(mesh, element_face.vec3_properties);
			details::collect_face_properties(mesh, element_face.float_properties);
			details::collect_face_properties(mesh, element_face.int_properties);
			details::collect_face_properties(mesh, element_face.int_list_properties);
			details::collect_face_properties(mesh, element_face.float_list_properties);

            elements.emplace_back(element_face);

			//-----------------------------------------------------

			// element edge
			Element element_edge("edge", mesh->n_edges());

			// attributes defined on element "edge"
			details::collect_edge_properties(mesh, element_edge.vec3_properties);
			details::collect_edge_properties(mesh, element_edge.float_properties);
			details::collect_edge_properties(mesh, element_edge.int_properties);
			details::collect_edge_properties(mesh, element_edge.int_list_properties);
			details::collect_edge_properties(mesh, element_edge.float_list_properties);
			// if edge property exist, we need to create the edge element which
			// must have an extra int list property "vertex_indices"
			if (element_edge.vec3_properties.size() > 0 ||
				element_edge.float_properties.size() > 0 ||
				element_edge.int_properties.size() > 0 ||
				element_edge.int_list_properties.size() > 0 ||
				element_edge.float_list_properties.size() > 0)
			{
				// vertex_indices
				IntListProperty edge_vertex_indices;
                edge_vertex_indices.name = "vertex_indices";
				edge_vertex_indices.reserve(mesh->n_edges());
				for (auto e : mesh->edges()) {
					int id0 = mesh->vertex(e, 0).idx();
					int id1 = mesh->vertex(e, 1).idx();
					edge_vertex_indices.push_back({ id0, id1 });
				}
				element_edge.int_list_properties.emplace_back(edge_vertex_indices);
                elements.emplace_back(element_edge);
			}

			//-----------------------------------------------------

			DLOG_IF(WARNING, !binary) << "you're writing an ASCII ply file. Use binary format for better performance";

			PlyWriter writer;
			return writer.write(file_name, elements, "", binary);
		}


	} // namespace io

} // namespace easy3
