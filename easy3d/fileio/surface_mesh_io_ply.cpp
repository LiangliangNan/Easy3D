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
#include <easy3d/fileio/translator.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    // \cond
	namespace io {


		namespace internal {

			template <typename PropertyT>
			inline bool extract_named_property(std::vector<PropertyT>& properties, PropertyT& wanted, const std::string& name) {
				for (auto it = properties.begin(); it != properties.end(); ++it) {
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

		} // namespace internal


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
            FloatListProperty  face_halfedge_texcoords;
			IntListProperty    edge_vertex_indices;

			const Element* element_vertex = nullptr;
			for (auto& e : elements) {
                if (e.name == "vertex") {
                    element_vertex = &e;
					if (internal::extract_named_property(e.vec3_properties, coordinates, "point"))
						continue;
					else {
						LOG(ERROR) << "vertex coordinates (x, y, z properties) do not exist";
						return false;
					}
				}
                else if (e.name == "face") {
                    if (internal::extract_named_property(e.int_list_properties, face_vertex_indices, "vertex_indices") ||
                        internal::extract_named_property(e.int_list_properties, face_vertex_indices, "vertex_index")) {
                        internal::extract_named_property(e.float_list_properties, face_halfedge_texcoords, "texcoord");
                        continue;
                    } else {
                        LOG(ERROR)
                                << "edge properties might not be parsed correctly because both 'vertex_indices' and 'vertex_index' not defined on faces";
                        return false;
                    }
				}
                else if (e.name == "edge") {
                    if (internal::extract_named_property(e.int_list_properties, edge_vertex_indices, "vertex_indices") ||
                        internal::extract_named_property(e.int_list_properties, edge_vertex_indices, "vertex_index"))
                        continue;
                    else {
                        LOG(ERROR) << "edge properties might not be parsed correctly because both 'vertex_indices' and 'vertex_index' not defined on edges";
                        // return false; // no return because the model can still be visualized
                    }
				}
			}

			mesh->clear();

            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();

            // add vertices
            for (auto p : coordinates)
                builder.add_vertex(p);

            if (element_vertex) {// add vertex properties
                // NOTE: to properly handle non-manifold meshes, vertex properties must be added before adding the faces
                internal::add_vertex_properties<vec3>(mesh, element_vertex->vec3_properties);
                internal::add_vertex_properties<vec2>(mesh, element_vertex->vec2_properties);
                internal::add_vertex_properties<float>(mesh, element_vertex->float_properties);
                internal::add_vertex_properties<int>(mesh, element_vertex->int_properties);
                internal::add_vertex_properties<std::vector<int> >(mesh, element_vertex->int_list_properties);
                internal::add_vertex_properties<std::vector<float> >(mesh, element_vertex->float_list_properties);
            } else {
                LOG(ERROR) << "element 'vertex' not found";
            }

            // add faces

            // create texture coordinate property if texture coordinates present
            SurfaceMesh::HalfedgeProperty<vec2> prop_texcoords;
            if (face_halfedge_texcoords.size() == face_vertex_indices.size())
                prop_texcoords = mesh->add_halfedge_property<vec2>("h:texcoord");

            // find the face's halfedge that points to v.
            auto find_face_halfedge = [](SurfaceMesh *mesh, SurfaceMesh::Face face,
                                         SurfaceMesh::Vertex v) -> SurfaceMesh::Halfedge {
                for (auto h : mesh->halfedges(face)) {
                    if (mesh->target(h) == v)
                        return h;
                }
                LOG_N_TIMES(3, ERROR) << "could not find a halfedge pointing to " << v << " in face " << face
                                      << ". " << COUNTER;
                return SurfaceMesh::Halfedge();
            };

            for (std::size_t i=0; i<face_vertex_indices.size(); ++i) {
                const auto& indices = face_vertex_indices[i];
				std::vector<SurfaceMesh::Vertex> vts;
				for (auto id : indices)
                    vts.emplace_back(SurfaceMesh::Vertex(id));
                auto face = builder.add_face(vts);

                // now let's add the texcoords (defined on halfedges)
                if (face.is_valid() && prop_texcoords) {
                    const auto& face_texcoords = face_halfedge_texcoords[i];
                    if (face_texcoords.size() == vts.size() * 2) { // 2 coordinates per vertex
                        auto begin = find_face_halfedge(mesh, face, builder.face_vertices()[0]);
                        auto cur = begin;
                        unsigned int texcord_idx = 0;
                        do {
                            prop_texcoords[cur] = vec2(face_texcoords[texcord_idx], face_texcoords[texcord_idx + 1]);
                            texcord_idx += 2;
                            cur = mesh->next(cur);
                        } while (cur != begin);
                    }
                }
			}

			// now let's add the remained properties
			for (const auto& e : elements) {
                if (e.name == "vertex")
                    continue;   // the vertex property has already been added
                else if (e.name == "face") {
                    internal::add_face_properties<vec3>(mesh, e.vec3_properties);
                    internal::add_face_properties<vec2>(mesh, e.vec2_properties);
                    internal::add_face_properties<float>(mesh, e.float_properties);
                    internal::add_face_properties<int>(mesh, e.int_properties);
                    internal::add_face_properties<std::vector<int> >(mesh, e.int_list_properties);
                    internal::add_face_properties<std::vector<float> >(mesh, e.float_list_properties);
                } else if (e.name == "edge") {
                    internal::add_edge_properties<vec3>(mesh, e.vec3_properties);
                    internal::add_edge_properties<vec2>(mesh, e.vec2_properties);
                    internal::add_edge_properties<float>(mesh, e.float_properties);
                    internal::add_edge_properties<int>(mesh, e.int_properties);
                    internal::add_edge_properties<std::vector<int> >(mesh, e.int_list_properties);
                    internal::add_edge_properties<std::vector<float> >(mesh, e.float_list_properties);
                } else {
                    const std::string name = "element-" + e.name;
                    auto prop = mesh->add_model_property<Element>(name, Element(""));
                    prop.vector().push_back(e);
                    LOG(WARNING) << "unknown element '" << e.name
                                 << "' with the following properties has been stored as a model property '" << name
                                 << "'"
                                 << e.property_statistics();
                }
			}

			builder.end_surface();

            if (Translator::instance()->status() == Translator::TRANSLATE_USE_FIRST_POINT) {
                auto& points = mesh->get_vertex_property<vec3>("v:point").vector();

                // the first point
                const vec3 p0 = points[0];
                const dvec3 origin(p0.data());
                Translator::instance()->set_translation(origin);

                for (auto& p: points)
                    p -= p0;

                auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. the first vertex (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            } else if (Translator::instance()->status() == Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET) {
                const dvec3 &origin = Translator::instance()->translation();
                auto& points = mesh->get_vertex_property<vec3>("v:point").vector();
                for (auto& p: points) {
                    p.x -= static_cast<float>(origin.x);
                    p.y -= static_cast<float>(origin.y);
                    p.z -= static_cast<float>(origin.z);
                }

                auto trans = mesh->add_model_property<dvec3>("translation", dvec3(0, 0, 0));
                trans[0] = origin;
                LOG(INFO) << "model translated w.r.t. last known reference point (" << origin
                          << "), stored as ModelProperty<dvec3>(\"translation\")";
            }

            return mesh->n_faces() > 0;
		}


		namespace internal {


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


		} // namespace internal


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
			internal::collect_vertex_properties(mesh, element_vertex.vec3_properties);
            internal::collect_vertex_properties(mesh, element_vertex.vec2_properties);
			internal::collect_vertex_properties(mesh, element_vertex.float_properties);
			internal::collect_vertex_properties(mesh, element_vertex.int_properties);
			internal::collect_vertex_properties(mesh, element_vertex.int_list_properties);
			internal::collect_vertex_properties(mesh, element_vertex.float_list_properties);

            auto trans = mesh->get_model_property<dvec3>("translation");
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
				for (auto h : mesh->halfedges(f)) {
                    indices.push_back(mesh->target(h).idx());
                }
				face_vertex_indices.push_back(indices);
			}
			element_face.int_list_properties.emplace_back(face_vertex_indices);

            // texture coordinates (defined on halfedges)
            auto texcoord = mesh->get_halfedge_property<vec2>("h:texcoord");
            if (texcoord) {
                FloatListProperty face_halfedge_texcoords;
                face_halfedge_texcoords.name = "texcoord";
                for (auto f: mesh->faces()) {
                    std::vector<float> texcoords;
                    for (auto h: mesh->halfedges(f)) {
                        const auto& tex = texcoord[h];
                        texcoords.push_back(tex.x);
                        texcoords.push_back(tex.y);
                    }
                    face_halfedge_texcoords.push_back(texcoords);
                }
                element_face.float_list_properties.push_back(face_halfedge_texcoords);
            }

			// attributes defined on element "face"
			internal::collect_face_properties(mesh, element_face.vec3_properties);
            internal::collect_face_properties(mesh, element_face.vec2_properties);
			internal::collect_face_properties(mesh, element_face.float_properties);
			internal::collect_face_properties(mesh, element_face.int_properties);
			internal::collect_face_properties(mesh, element_face.int_list_properties);
			internal::collect_face_properties(mesh, element_face.float_list_properties);

            elements.emplace_back(element_face);

			//-----------------------------------------------------

			// element edge
			Element element_edge("edge", mesh->n_edges());

			// attributes defined on element "edge"
			internal::collect_edge_properties(mesh, element_edge.vec3_properties);
            internal::collect_edge_properties(mesh, element_edge.vec2_properties);
			internal::collect_edge_properties(mesh, element_edge.float_properties);
			internal::collect_edge_properties(mesh, element_edge.int_properties);
			internal::collect_edge_properties(mesh, element_edge.int_list_properties);
			internal::collect_edge_properties(mesh, element_edge.float_list_properties);
			// if edge property exist, we need to create the edge element which
			// must have an extra int list property "vertex_indices"
			if (!element_edge.vec3_properties.empty() ||
			    !element_edge.vec2_properties.empty() ||
				!element_edge.float_properties.empty() ||
				!element_edge.int_properties.empty() ||
				!element_edge.int_list_properties.empty() ||
				!element_edge.float_list_properties.empty())
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

            binary = binary && (file_name.find("ascii") == std::string::npos);
            LOG_IF(!binary, WARNING) << "you're writing an ASCII ply file. Use binary format for better performance";

            return PlyWriter::write(file_name, elements, "", binary);
		}


	} // namespace io
    // \endcond

} // namespace easy3
