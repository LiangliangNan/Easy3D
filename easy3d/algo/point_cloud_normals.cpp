/** \cond LICENSE
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
 \endcond */

#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/graph_minimum_spanning_tree.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/principal_axes.h>
#include <easy3d/kdtree/kdtree_search_nanoflann.h>

#include <easy3d/util/stop_watch.h>

#include <cmath>
#include <limits>
#include <vector>


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


    namespace internal {

        /// The normal propagation graph of Hoppe et al. (SIGGRAPH 1992): each point is
        /// connected with its k nearest neighbors and an edge is weighted by the Riemannian
        /// distance 1 - |n1 . n2| between the two normal *directions* (they are still
        /// unoriented at this point).
        /// The edges are collected in `weights`, index aligned with the edges of `graph`.
        void build_graph(PointCloud *cloud, const KdTreeSearch *tree, unsigned int k, Graph &graph,
                         std::vector<float> &weights) {
            const std::vector<vec3> &points = cloud->points();
            const std::vector<vec3> &normals = cloud->vertex_property<vec3>("v:normal").vector();

            graph.reserve(static_cast<unsigned int>(points.size()),
                          static_cast<unsigned int>(points.size() * k));
            for (auto p : points)
                graph.add_vertex(p);

            weights.reserve(points.size() * k);
            std::vector<int> neighbors;
            for (std::size_t i = 0; i < points.size(); ++i) {
                // NOTE: the result includes the query point itself
                tree->find_closest_k_points(points[i], static_cast<int>(k), neighbors);
                if (neighbors.size() < k)
                    continue; // in extreme cases, a point cloud can have less than K points

                const Graph::Vertex v1(static_cast<int>(i));
                for (auto j : neighbors) {
                    if (j == static_cast<int>(i))
                        continue;

                    const Graph::Vertex v2(j);
                    if (graph.find_edge(v1, v2).is_valid())
                        continue; // the neighbor already linked the two points

                    // safety check: the dot product of two unit vectors can slightly exceed 1
                    float weight = 1.0f - std::abs(dot(normals[i], normals[static_cast<std::size_t>(j)]));
                    if (weight < 0)
                        weight = 0;

                    weights.push_back(weight);
                    graph.add_edge(v1, v2);
                }
            }
        }

        /// The connected components of the graph. A point cloud may be acquired in several
        /// clusters, so each of them needs its own spanning tree.
        /// The normal of the highest point of a component is oriented towards +Z and roots
        /// the tree of that component.
        std::vector<int> component_roots(PointCloud *cloud, const Graph &graph) {
            const std::vector<vec3> &points = cloud->points();
            std::vector<vec3> &normals = cloud->vertex_property<vec3>("v:normal").vector();
            const int num = static_cast<int>(graph.vertices_size());

            std::vector<int> roots;
            std::vector<char> visited(static_cast<std::size_t>(num), 0);
            std::vector<int> pending; // iterative traversal: a component can be a long chain
            for (int i = 0; i < num; ++i) {
                if (visited[static_cast<std::size_t>(i)])
                    continue;

                visited[static_cast<std::size_t>(i)] = 1;
                pending.push_back(i);
                int top = i;
                while (!pending.empty()) {
                    const Graph::Vertex v(pending.back());
                    pending.pop_back();
                    if (points[static_cast<std::size_t>(v.idx())].z > points[static_cast<std::size_t>(top)].z)
                        top = v.idx();
                    for (auto e : graph.edges(v)) {
                        Graph::Vertex w = graph.vertex(e, 0);
                        if (w == v)
                            w = graph.vertex(e, 1);
                        if (visited[static_cast<std::size_t>(w.idx())])
                            continue;
                        visited[static_cast<std::size_t>(w.idx())] = 1;
                        pending.push_back(w.idx());
                    }
                }

                // the orientation of the tree is known from its root only
                if (normals[static_cast<std::size_t>(top)].z < 0)
                    normals[static_cast<std::size_t>(top)] = -normals[static_cast<std::size_t>(top)];
                roots.push_back(top);
            }

            return roots;
        }

        /// Propagates the orientation of the normals along the spanning tree: a normal is
        /// flipped to agree with the one of its parent. The propagation stops being trusted
        /// across an edge whose two normals differ by more than `angle_max`, because such an
        /// edge is too uncertain to carry the orientation over.
        /// \pre Normals must be unit vectors
        void propagate(PointCloud *cloud, const std::vector<int> &roots, const std::vector<int> &predecessor) {
            // Note: `0 < angle_max <= PI/2`
            const static double angle_max = M_PI * 0.5;

            std::vector<vec3> &normals = cloud->vertex_property<vec3>("v:normal").vector();
            const int num = static_cast<int>(predecessor.size());

            // invert the parent relation to walk the tree from its roots to its leaves
            std::vector<std::vector<int>> children(static_cast<std::size_t>(num));
            std::vector<char> oriented(static_cast<std::size_t>(num), 0);
            for (auto r : roots)
                oriented[static_cast<std::size_t>(r)] = 1;

            std::vector<int> pending = roots;
            for (int i = 0; i < num; ++i) {
                const int parent = predecessor[static_cast<std::size_t>(i)];
                if (parent < 0)
                    continue; // a root or a vertex the tree did not reach
                children[static_cast<std::size_t>(parent)].push_back(i);
            }

            while (!pending.empty()) {
                const int p = pending.back();
                pending.pop_back();

                for (auto c : children[static_cast<std::size_t>(p)]) {
                    vec3 &target = normals[static_cast<std::size_t>(c)];
                    const vec3 &source = normals[static_cast<std::size_t>(p)];

                    float normal_dot = dot(source, target);
                    if (normal_dot < 0)
                        target = -target;

                    oriented[static_cast<std::size_t>(c)] = oriented[static_cast<std::size_t>(p)] &&
                        std::abs(normal_dot) >= std::cos(angle_max);
                    pending.push_back(c);
                }
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
        Graph graph;
        std::vector<float> weights;
        internal::build_graph(cloud, &kdtree, k, graph, weights);
        const auto roots = internal::component_roots(cloud, graph);
        LOG(INFO) << "done. #vertices: " << graph.n_vertices()
                  << ", #edges: " << graph.n_edges()
                  << ", #components: " << roots.size()
                  << ". " << w.time_string();

        w.restart();
        LOG(INFO) << "extract minimum spanning tree...";
        std::vector<int> predecessor;
        if (!GraphMinimumSpanningTree::compute(&graph, weights, roots, predecessor)) {
            LOG(ERROR) << "failed to extract the minimum spanning tree";
            return false;
        }
        LOG(INFO) << "done. " << w.time_string();

        w.restart();
        LOG(INFO) << "propagate...";
        internal::propagate(cloud, roots, predecessor);
        LOG(INFO) << "done. " << w.time_string();

        return true;
    }

}
