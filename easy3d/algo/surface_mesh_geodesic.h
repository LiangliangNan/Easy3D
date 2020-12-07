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

#ifndef EASY3D_ALGO_SURFACE_MESH_GEODESIC_H
#define EASY3D_ALGO_SURFACE_MESH_GEODESIC_H

#include <easy3d/core/surface_mesh.h>
#include <vector>
#include <set>
#include <float.h>
#include <limits.h>

namespace easy3d {

    /**
     * \brief This class computes geodesic distance from a set of seed vertices.
     * \class SurfaceMeshGeodesic easy3d/algo/surface_mesh_geodesic.h
     * \details The method works by a Dykstra-like breadth first traversal from the seed vertices, implemented by a
     * heap structure. See the following paper for more details:
     *  - Kimmel and Sethian. Computing geodesic paths on manifolds. Proceedings of the National Academy of Sciences,
     *    95(15):8431–8435, 1998.
     */
    class SurfaceMeshGeodesic {
    public:
        //! \brief Construct from mesh.
        //! \param mesh The mesh on which to compute the geodesic distances.
        //! \param use_virtual_edges A flag to control the use of virtual edges.
        //! Default: true.
        //! \sa compute() to actually compute the geodesic distances.
        SurfaceMeshGeodesic(SurfaceMesh *mesh, bool use_virtual_edges = true);

        // destructor
        ~SurfaceMeshGeodesic();

        //! \brief Compute geodesic distances from specified seed points.
        //! \param[in] seed The vector of seed vertices.
        //! \param[in] maxdist The maximum distance up to which to compute the
        //! geodesic distances.
        //! \param[in] maxnum The maximum number of neighbors up to which to
        //! compute the geodesic distances.
        //! \param[out] neighbors The vector of neighbor vertices.
        //! \return The number of neighbors that have been found.
        unsigned int compute(const std::vector<SurfaceMesh::Vertex> &seed,
                             float maxdist = FLT_MAX,
                             unsigned int maxnum = INT_MAX,
                             std::vector<SurfaceMesh::Vertex> *neighbors = nullptr);

        //! \brief Access the computed geodesic distance.
        //! \param[in] v The vertex for which to return the geodesic distance.
        //! \return The geodesic distance of vertex \p v.
        //! \pre The function compute() has been called before.
        //! \pre The vertex \p v needs to be a valid vertex handle of the mesh
        //! used during construction.
        float operator()(SurfaceMesh::Vertex v) const { return distance_[v]; }

        //! \brief Use the normalized distances as texture coordinates
        //! \details Stores the normalized distances in a vertex property of type
        //! TexCoord named "v:tex". Re-uses any existing vertex property of the
        //! same type and name.
        void distance_to_texture_coordinates();

    private: // private types
        // functor for comparing two vertices w.r.t. their geodesic distance
        class VertexCmp {
        public:
            VertexCmp(const SurfaceMesh::VertexProperty<float> &dist) : dist_(dist) {}

            bool operator()(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) const {
                return ((dist_[v0] == dist_[v1]) ? (v0 < v1)
                                                 : (dist_[v0] < dist_[v1]));
            }

        private:
            const SurfaceMesh::VertexProperty<float> &dist_;
        };

        // priority queue using geodesic distance as sorting criterion
        typedef std::set<SurfaceMesh::Vertex, VertexCmp> PriorityQueue;

        // virtual edges for walking through obtuse triangles
        struct VirtualEdge {
            VirtualEdge(SurfaceMesh::Vertex v, float l) : vertex(v), length(l) {}

            SurfaceMesh::Vertex vertex;
            float length;
        };

        // set for storing virtual edges
        typedef std::map<SurfaceMesh::Halfedge, VirtualEdge> VirtualEdges;

    private: // private methods
        void find_virtual_edges();

        unsigned int init_front(const std::vector<SurfaceMesh::Vertex> &seed,
                                std::vector<SurfaceMesh::Vertex> *neighbors);

        unsigned int propagate_front(float maxdist, unsigned int maxnum,
                                     std::vector<SurfaceMesh::Vertex> *neighbors);

        void heap_vertex(SurfaceMesh::Vertex v);

        float distance(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, float r0 = FLT_MAX,
                       float r1 = FLT_MAX);

    private: // private data
        SurfaceMesh *mesh_;

        bool use_virtual_edges_;
        VirtualEdges virtual_edges_;

        PriorityQueue *front_;

        SurfaceMesh::VertexProperty<float> distance_;
        SurfaceMesh::VertexProperty<bool> processed_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_GEODESIC_H