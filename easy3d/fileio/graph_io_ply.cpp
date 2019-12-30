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


#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/core/graph.h>



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
            inline void add_vertex_properties(Graph* graph, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
                    if (p.size() != graph->vertices_size()) {
                        std::cerr << "vertex property size (" << p.size() << ") does not match number of vertices (" << graph->vertices_size() << ")" << std::endl;
						continue;
					}
					if (name.find("v:") == std::string::npos)
						name = "v:" + name;
                    auto prop = graph->vertex_property<T>(name);
					prop.vector() = p;
				}
			}


			template <typename T, typename PropertyT>
            inline void add_edge_properties(Graph* graph, const std::vector<PropertyT>& properties)
			{
				for (const auto& p : properties) {
                    std::string name = p.name;
                    if (p.size() != graph->edges_size()) {
                        std::cerr << "edge property size (" << p.size() << ") does not match number of edges (" << graph->edges_size() << ")" << std::endl;
						continue;
					}
					if (name.find("e:") == std::string::npos)
						name = "e:" + name;
                    auto prop = graph->edge_property<T>(name);
					prop.vector() = p;
				}
			}

		} // namespace details


        bool load_ply(const std::string& file_name, Graph* graph)
		{
            if (!graph) {
                std::cerr << "null graph pointer" << std::endl;
				return false;
			}

			std::vector<Element> elements;
			PlyReader reader;
			if (!reader.read(file_name, elements))
				return false;

			Vec3Property       coordinates;
			IntListProperty    edge_vertex_indices;

			for (std::size_t i = 0; i < elements.size(); ++i) {
				Element& e = elements[i];
                if (e.name == "vertex") {
					if (details::extract_named_property(e.vec3_properties, coordinates, "point"))
						continue;
					else {
						std::cerr << "vertex coordinates (x, y, z properties) do not exist" << std::endl;
						return false;
					}
				}
                else if (e.name == "edge") {
					if (details::extract_named_property(e.int_list_properties, edge_vertex_indices, "vertex_indices"))
						continue;
                    else {
						std::cerr << "edge properties might not be parsed correctly because \'vertex_indices\' does not defined on edges" << std::endl;
                        return false;
                    }
                }
			}

            for (auto p : coordinates)
                graph->add_vertex(p);

            for (auto e : edge_vertex_indices) {
                if (e.size() == 2)
                    graph->add_edge(Graph::Vertex(e[0]), Graph::Vertex(e[1]));
                else {
                    std::cerr << "The size of edge property \'vertex_indices\' is not 2" << std::endl;
                    continue;
                }
            }

			// now let's add the properties
			for (std::size_t i = 0; i < elements.size(); ++i) {
				Element& e = elements[i];
                if (e.name == "vertex") {
                    details::add_vertex_properties<vec3>(graph, e.vec3_properties);
                    details::add_vertex_properties<float>(graph, e.float_properties);
                    details::add_vertex_properties<int>(graph, e.int_properties);
                    details::add_vertex_properties< std::vector<int> >(graph, e.int_list_properties);
                    details::add_vertex_properties< std::vector<float> >(graph, e.float_list_properties);
				}
                else if (e.name == "face")
                    std::cerr << "The Graph has face information (ignored). Is it a mesh?" << std::endl;
                else if (e.name == "edge") {
                    details::add_edge_properties<vec3>(graph, e.vec3_properties);
                    details::add_edge_properties<float>(graph, e.float_properties);
                    details::add_edge_properties<int>(graph, e.int_properties);
                    details::add_edge_properties< std::vector<int> >(graph, e.int_list_properties);
                    details::add_edge_properties< std::vector<float> >(graph, e.float_list_properties);
				}
				else
                    std::cerr << "The Graph has unknown element: " << e.name << " (ignored)" << std::endl;
			}

            return graph->n_vertices() > 0;
		}


		namespace details {


			template <typename T>
            inline void collect_vertex_properties(const Graph* graph, std::vector< GenericProperty<T> >& properties) {
                const auto& all_properties = graph->vertex_properties();
				for (auto name : all_properties) {
                    auto prop = graph->get_vertex_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "v:")
							name = name.substr(2, name.length() - 1);
						GenericProperty<T> p("vertex", name, prop.vector());
						properties.push_back(p);
					}
				}
			}

			template <typename T>
            inline void collect_edge_properties(const Graph* graph, std::vector< GenericProperty<T> >& properties) {
                const auto& all_properties = graph->edge_properties();
				for (auto name : all_properties) {
                    auto prop = graph->get_edge_property<T>(name);
					if (prop) {
						if (name.substr(0, 2) == "e:")
							name = name.substr(2, name.length() - 1);
						GenericProperty<T> p("edge", name, prop.vector());
						properties.push_back(p);
					}
				}
			}


		} // namespace details


        bool save_ply(const std::string& file_name, const Graph* graph, bool binary) {
            if (!graph || graph->n_vertices() == 0) {
                std::cerr << "empty graph data" << std::endl;
				return false;
			}

			std::vector<Element> elements;

			//-----------------------------------------------------

			// element vertex
            Element element_vertex("vertex", graph->n_vertices());

			// attributes defined on element "vertex"
            details::collect_vertex_properties(graph, element_vertex.vec3_properties);
            details::collect_vertex_properties(graph, element_vertex.float_properties);
            details::collect_vertex_properties(graph, element_vertex.int_properties);
            details::collect_vertex_properties(graph, element_vertex.int_list_properties);
            details::collect_vertex_properties(graph, element_vertex.float_list_properties);

			elements.emplace_back(element_vertex);

			//-----------------------------------------------------

			// element edge
            Element element_edge("edge", graph->n_edges());

            // vertex_indices
            IntListProperty edge_vertex_indices;
            edge_vertex_indices.name = "vertex_indices";
            edge_vertex_indices.reserve(graph->edges_size());
            for (auto e : graph->edges()) {
                int id0 = graph->from_vertex(e).idx();
                int id1 = graph->to_vertex(e).idx();
                edge_vertex_indices.push_back({ id0, id1 });
            }
            element_edge.int_list_properties.emplace_back(edge_vertex_indices);

			// attributes defined on element "edge"
            details::collect_edge_properties(graph, element_edge.vec3_properties);
            details::collect_edge_properties(graph, element_edge.float_properties);
            details::collect_edge_properties(graph, element_edge.int_properties);
            details::collect_edge_properties(graph, element_edge.int_list_properties);
            details::collect_edge_properties(graph, element_edge.float_list_properties);

            elements.emplace_back(element_edge);

			//-----------------------------------------------------

			if (!binary)
				std::cout << "TODO: use binary format" << std::endl;

			PlyWriter writer;
			return writer.write(file_name, elements, "", binary);
		}


	} // namespace io

} // namespace easy3
