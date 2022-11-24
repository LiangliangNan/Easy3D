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

#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/principal_axes.h>
#include <easy3d/kdtree/kdtree_search_nanoflann.h>

#include <easy3d/util/stop_watch.h>


#ifdef HAS_BOOST

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

#ifdef VISUALIZE_MST_FOR_DEBUGGING
#include <easy3d/core/constant.h>
#include <easy3d/core/random.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#endif

#endif


namespace easy3d {

    bool PointCloudNormals::estimate(PointCloud *cloud, unsigned int k /* = 16 */,
                                     bool compute_curvature /* = false */) {
        if (!cloud) {
            LOG(ERROR) << "empty input point cloud";
            return false;
        }

        StopWatch w;
        w.start();

        LOG(INFO) << "building kd_tree...";
        KdTreeSearch_NanoFLANN kdtree(cloud);
        LOG(INFO) << "done. " << w.time_string();

        int num = static_cast<int>(cloud->n_vertices());
        const std::vector<vec3> &points = cloud->points();
        std::vector<vec3> &normals = cloud->vertex_property<vec3>("v:normal").vector();

        std::vector<float> *curvatures = nullptr;
        if (compute_curvature)
            curvatures = &(cloud->vertex_property<float>("v:curvature").vector());

        w.restart();
        LOG(INFO) << "estimating normals...";

#pragma omp parallel for
        for (int i = 0; i < num; ++i) {
            const vec3 &p = points[i];
            std::vector<int> neighbors;
            kdtree.find_closest_k_points(p, static_cast<int>(k), neighbors);

            PrincipalAxes<3> pca;
            pca.begin();
            for (auto idx : neighbors)
                pca.add(points[idx]);
            pca.end();

            // the eigen vector corresponding to the smallest eigen value
            normals[i] = pca.axis<float>(2);
            if (normals[i].z < 0) // almost have positive Z
                normals[i] = -normals[i];

            if (compute_curvature)
                (*curvatures)[i] = float(
                        pca.eigen_value(2) / (pca.eigen_value(0) + pca.eigen_value(1) + pca.eigen_value(2)));
        }

        LOG(INFO) << "done. " << w.time_string();
        return true;
    }

#ifdef HAS_BOOST

    namespace internal {

        struct VertexProperty {
            explicit VertexProperty(easy3d::PointCloud::Vertex v = easy3d::PointCloud::Vertex(-1)) : vertex(v) {}
            easy3d::PointCloud::Vertex vertex;
        };

        struct EdgeProperty {
            explicit EdgeProperty(float w = -std::numeric_limits<float>::max()) : weight(w) {}
            float weight;
        };

        /// RiemannianGraph encodes:
        /// - the adjacency relations of vertices in a K neighbors.
        /// - vertices contain the corresponding input point cloud vertex
        /// - the edge weight = edge weight = 1 - | normal1 * normal2 |
        class RiemannianGraph
                : public boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProperty, EdgeProperty> {
        public:
            typedef RiemannianGraph::vertex_descriptor Vertex;
            typedef RiemannianGraph::edge_descriptor Edge;

        public:
            Vertex top;
        };

        // builds the graph
        void build_graph(easy3d::PointCloud *cloud, const KdTreeSearch *tree, unsigned int k, RiemannianGraph &graph) {
            // Step 1: create the vertices of the graph

            // we need to remember the vertex descriptor of each point.
            // This is needed in a later stage to add edges.
            auto vertex_descriptors = cloud->vertex_property<RiemannianGraph::Vertex>("v:descriptors");

            for (auto v : cloud->vertices())
                vertex_descriptors[v] = boost::add_vertex(VertexProperty(v), graph);

            // Step 2: create the edges connecting neighboring points.

            auto points = cloud->get_vertex_property<easy3d::vec3>("v:point");
            auto normals = cloud->get_vertex_property<easy3d::vec3>("v:normal");
            for (auto v : cloud->vertices()) {
                const easy3d::vec3 &p = points[v];

                // The indices of the neighbors of v (NOTE: the result include v itself).
                std::vector<int> neighbor_indices;
                tree->find_closest_k_points(p, static_cast<int>(k), neighbor_indices);
                if (neighbor_indices.size() < k)
                    continue; // in extreme cases, a point cloud can have less than K points

                // now let's create the edges
                for (auto index : neighbor_indices) {
                    if (index == v.idx())
                        continue; // this is actually the current vertex

                    easy3d::PointCloud::Vertex v2(index);
                    RiemannianGraph::Vertex vd1 = vertex_descriptors[v];
                    RiemannianGraph::Vertex vd2 = vertex_descriptors[v2];
                    std::pair<RiemannianGraph::Edge, bool> ed = boost::edge(vd1, vd2, graph);
                    if (ed.second)
                        continue; // the edge already exists.

                    const easy3d::vec3 &n1 = normals[v];
                    const easy3d::vec3 &n2 = normals[v2];
                    float weight = 1.0f - std::abs(dot(n1, n2));
                    if (weight < 0)
                        weight = 0; // safety check

                    // add an edge to the graph: add_edge()
                    boost::add_edge(vd1, vd2, EdgeProperty(weight), graph);
                }
            }

            cloud->remove_vertex_property(vertex_descriptors);
        }


        /// MST_Graph encodes:
        /// - the adjacency relations of vertices in a Minimum Spanning Tree.
        /// - vertices contain the corresponding input point cloud vertex
        /// - a boolean indicating if the normal is oriented.
        struct MST_VertexProperty : public VertexProperty {
            explicit MST_VertexProperty(easy3d::PointCloud::Vertex v = easy3d::PointCloud::Vertex(-1), bool oriented = false)
                    : VertexProperty(v), is_oriented(oriented) {}

            bool is_oriented;
        };

        class MST_Graph : public boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, MST_VertexProperty> {
        public:
            typedef typename MST_Graph::vertex_descriptor Vertex;
            typedef typename MST_Graph::edge_descriptor Edge;

        public:
            explicit MST_Graph(PointCloud::VertexProperty<vec3> nmap) : normal_map(nmap) {}

            PointCloud::VertexProperty<vec3> normal_map;

            Vertex root;
        };


        void extract_minimum_spanning_tree(const RiemannianGraph &graph, MST_Graph &mst) {
            RiemannianGraph::Vertex top = graph.top;

            std::vector<internal::RiemannianGraph::Vertex> predecessor(boost::num_vertices(graph));
            boost::prim_minimum_spanning_tree(
                    graph,
                    &predecessor[0],
                    boost::weight_map(boost::get(&internal::EdgeProperty::weight, graph)).root_vertex(top)
            );

            // we create a directed graph (i.e., MST_Graph) to represent the MST.
            // add vertices
            for (std::size_t i = 0; i < predecessor.size(); ++i) {
                // the current vertex: graph[i].vertex
                PointCloud::Vertex v = graph[i].vertex;

                if (predecessor[i] == i) { // is either the root or unreachable
                    // the root or unreachable vertices are marked "oriented".
                    auto vt = boost::add_vertex(MST_VertexProperty(v, true), mst);
                    if (i == top)
                        mst.root = vt;
                } else {
                    // other vertices are marked "oriented".
                    boost::add_vertex(MST_VertexProperty(v, false), mst);
                }
            }

            // add edges
            for (std::size_t i = 0; i < predecessor.size(); ++i) {
                if (predecessor[i] == i)  // is either the root or not reachable
                    continue;
                else {
                    // the parent: predecessor[i]
                    // current vertex: i
                    boost::add_edge(predecessor[i], i, mst);
                }
            }
        }

        template<class Graph>
        class BfsVisitor : public boost::default_bfs_visitor {
        public:
            typedef typename Graph::edge_descriptor EdgeDescriptor;

            explicit BfsVisitor(std::function<void(EdgeDescriptor e, Graph &g)> fun)
                    : func_(fun) {}

            // Note: boost requires const graph
            void examine_edge(EdgeDescriptor edge, const Graph &graph) const {
                func_(edge, const_cast<Graph &>(graph));
            }

        private:
            std::function<void(EdgeDescriptor edge, Graph &graph)> func_;
        };


        /// find the top vertex (the one with the largest Z value) in the graph.
        /// orient the normal of the point with maximum Z towards +Z axis.
        void find_top_vertex(PointCloud *cloud, RiemannianGraph &graph) {
            float top_z = -std::numeric_limits<float>::max();
            auto points = cloud->get_vertex_property<vec3>("v:point");

            auto vi = boost::vertices(graph);
            for (auto vit = vi.first; vit != vi.second; ++vit) {
                RiemannianGraph::Vertex vd = *vit;
                PointCloud::Vertex v = graph[vd].vertex;
                if (points[v].z > top_z) {
                    graph.top = vd;
                    top_z = points[v].z;
                }
            }

            PointCloud::Vertex top = graph[graph.top].vertex;
            if (top.idx() != -1) {
                auto normals = cloud->get_vertex_property<vec3>("v:normal");
                if (normals[top].z < 0)
                    normals[top] = -normals[top];
            }
        }


        // extract the connected components of a graph. The results are stored in a set of graphs
        std::vector<RiemannianGraph> connected_components(PointCloud *cloud, const RiemannianGraph &riemannian_graph) {
            std::vector<std::size_t> labels(boost::num_vertices(riemannian_graph));
            std::size_t num = boost::connected_components(riemannian_graph, &labels[0]);

            std::vector<RiemannianGraph> components(num);

            // we need to remember the vertex descriptor of each point.
            // This is needed in a later stage to add edges.
            auto vertex_descriptors = cloud->vertex_property<RiemannianGraph::Vertex>("v:descriptors");

            for (std::size_t i = 0; i < boost::num_vertices(riemannian_graph); ++i) {
                std::size_t idx = labels[i];
                RiemannianGraph &graph = components[idx];
                PointCloud::Vertex v = riemannian_graph[i].vertex;
                vertex_descriptors[v] = boost::add_vertex(VertexProperty(v), graph);
            }

            // add the edges for each graph
            auto ei = boost::edges(riemannian_graph);
            for (auto eit = ei.first; eit != ei.second; ++eit) {
                auto vd1 = boost::source(*eit, riemannian_graph);
                auto vd2 = boost::target(*eit, riemannian_graph);
                if (labels[vd1] == labels[vd2]) { // they belong to the same component
                    std::size_t idx = labels[vd1];
                    RiemannianGraph &graph = components[idx];
                    auto p1 = riemannian_graph[vd1].vertex;
                    auto p2 = riemannian_graph[vd2].vertex;
                    auto s = vertex_descriptors[p1];
                    auto t = vertex_descriptors[p2];
                    boost::add_edge(s, t, EdgeProperty(riemannian_graph[*eit].weight), graph);
                }
            }

            cloud->remove_vertex_property(vertex_descriptors);

            for (auto &graph : components)
                find_top_vertex(cloud, graph);

            return components;
        }

        /// Propagate the normal orientation from the source vertex to the target vertex
        /// It does not orient normals that are already oriented.
        /// It does not propagate the orientation if the angle between 2 normals > angle_max.
        /// \pre Normals must be unit vectors
        /// \pre `0 < angle_max <= PI/2`
        void propagate_normal(MST_Graph::Edge edge, MST_Graph &graph) {
            const static double angle_max = M_PI * 0.5;

            // Gets source
            MST_Graph::Vertex source_vertex = boost::source(edge, graph);
            const bool source_normal_is_oriented = graph[source_vertex].is_oriented;
            PointCloud::Vertex v_source = graph[source_vertex].vertex;
            const vec3 &source_normal = graph.normal_map[v_source];

            // Gets target
            MST_Graph::Vertex target_vertex = boost::target(edge, graph);
            bool &target_normal_is_oriented = graph[target_vertex].is_oriented;
            PointCloud::Vertex v_target = graph[target_vertex].vertex;
            vec3 &target_normal = graph.normal_map[v_target];

            if (!target_normal_is_oriented) {
                float normals_dot = dot(source_normal, target_normal);
                if (normals_dot < 0) {
                    target_normal = -target_normal;
                }

                // Is orientation robust?
                target_normal_is_oriented = source_normal_is_oriented &&
                                            (std::abs(normals_dot) >=
                                             std::cos(angle_max)); // oriented iff angle <= m_angle_max
            }
        }
    }


    bool PointCloudNormals::reorient(PointCloud *cloud, unsigned int k) {
        if (!cloud) {
            LOG(ERROR) << "empty input point cloud";
            return false;
        }

        auto normals = cloud->get_vertex_property<vec3>("v:normal");
        if (!normals) {
            LOG(ERROR) << "normal information does not exist";
            return false;
        }

        StopWatch w;
        w.start();

        LOG(INFO) << "building kd_tree...";
        KdTreeSearch_NanoFLANN kdtree(cloud);
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "constructing graph...";
        internal::RiemannianGraph riemannian_graph;
        internal::build_graph(cloud, &kdtree, k, riemannian_graph);

        // a point clouds might be in multiple clusters, so we have to extract the connected components
        // first. After that, we can reorient all the components one by one.
        std::vector<internal::RiemannianGraph> components
                = internal::connected_components(cloud, riemannian_graph);
        LOG(INFO) << "done. #vertices: " << boost::num_vertices(riemannian_graph)
                  << ", #edges: " << boost::num_edges(riemannian_graph)
                  << ", #components: " << components.size()
                  << ". " << w.time_string();

        w.restart();
        LOG(INFO) << "extract minimum spanning tree...";
        std::vector<internal::MST_Graph> ms_trees;
        for (const auto &graph : components) {
            ms_trees.emplace_back(cloud->get_vertex_property<vec3>("v:normal"));
            internal::MST_Graph &mst = ms_trees.back();
            internal::extract_minimum_spanning_tree(graph, mst);
        }
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "propagate...";
        for (const auto &mst : ms_trees) {
            // Traverse the point set along the MST to propagate source_point's orientation
            internal::BfsVisitor<internal::MST_Graph> bfsVisitor(internal::propagate_normal);
            boost::breadth_first_search(mst, mst.root, boost::visitor(bfsVisitor));
        }
        LOG(INFO) << "done. " << w.time_string();

#ifdef VISUALIZE_MST_FOR_DEBUGGING
        // for debugging: create a drawable to visualize the MST_Graph
        if (cloud->renderer()) {
            LinesDrawable *mst_graph = cloud->renderer()->get_lines_drawable("mst_graph");
            if (!mst_graph)
                mst_graph = cloud->renderer()->add_lines_drawable("mst_graph");

            auto point_prop = cloud->get_vertex_property<vec3>("v:point");
            std::vector<vec3> points, colors;
            std::cout << "num MST: " << ms_trees.size() << std::endl;
            for (const auto &mst: ms_trees) {
                const vec3 &c = random_color(); // give each mst a unique color
                auto ei = boost::edges(mst);
                for (auto eit = ei.first; eit != ei.second; ++eit) {
                    auto sd = boost::source(*eit, mst);
                    auto td = boost::target(*eit, mst);
                    PointCloud::Vertex sv = mst[sd].vertex;
                    PointCloud::Vertex tv = mst[td].vertex;
                    points.push_back(point_prop[sv]);
                    colors.push_back(c);
                    points.push_back(point_prop[tv]);
                    colors.push_back(c);
                }
            }

            mst_graph->update_vertex_buffer(points);
            mst_graph->update_color_buffer(colors);
            mst_graph->set_coloring_method(State::COLOR_PROPERTY);
            mst_graph->set_visible(true);
        }
#endif

        return true;
    }

#else

    bool PointCloudNormals::reorient(PointCloud *cloud, unsigned int k)
    {
        LOG(ERROR) << "reorient point cloud normals requires boost";
        return false;
    }

#endif

}
