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

#ifndef EASY3D_ALGO_SURFACE_MESH_SIMPLIFICATION_H
#define EASY3D_ALGO_SURFACE_MESH_SIMPLIFICATION_H

#include <easy3d/core/surface_mesh.h>

#include <easy3d/algo/heap.h>
#include <easy3d/algo/normal_cone.h>
#include <easy3d/algo/quadric.h>

#include <set>
#include <vector>


namespace easy3d {

//=============================================================================
//! \addtogroup algorithms algorithms
//! @{
//=============================================================================

//! \brief Surface mesh simplification based on approximation error and fairness criteria.
//! \details Performs incremental greedy mesh simplification based on halfedge
//! collapses. See \cite kobbelt_1998_general and \cite garland_1997_surface for
//! details.
    class SurfaceSimplification {
    public:
        //! Construct with mesh to be simplified.
        SurfaceSimplification(SurfaceMesh *mesh);

        // destructor
        ~SurfaceSimplification();

        //! Initialize with given parameters.
        void initialize(float aspect_ratio = 0.0, float edge_length = 0.0,
                        unsigned int max_valence = 0, float normal_deviation = 0.0,
                        float hausdorff_error = 0.0);

        //! Simplify mesh to \p n vertices.
        void simplify(unsigned int n_vertices);

    private: //------------------------------------------------------ private types
        //! Store data for an halfedge collapse
        /*
                    vl
                    *
                   / \
                  /   \
                 / fl  \
             v0 *------>* v1
                 \ fr  /
                  \   /
                   \ /
                    *
                    vr
        */
        struct CollapseData {
            CollapseData(SurfaceMesh *sm, SurfaceMesh::Halfedge h);

            SurfaceMesh *mesh;

            SurfaceMesh::Halfedge v0v1; // Halfedge to be collapsed
            SurfaceMesh::Halfedge v1v0; // Reverse halfedge
            SurfaceMesh::Vertex v0;     // Vertex to be removed
            SurfaceMesh::Vertex v1;     // Remaining vertex
            SurfaceMesh::Face fl;       // Left face
            SurfaceMesh::Face fr;       // Right face
            SurfaceMesh::Vertex vl;     // Left vertex
            SurfaceMesh::Vertex vr;     // Right vertex
            SurfaceMesh::Halfedge v1vl, vlv0, v0vr, vrv1;
        };

        //! Heap interface
        class HeapInterface {
        public:
            HeapInterface(SurfaceMesh::VertexProperty<float> prio, SurfaceMesh::VertexProperty<int> pos)
                    : prio_(prio), pos_(pos) {
            }

            bool less(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) { return prio_[v0] < prio_[v1]; }

            bool greater(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) { return prio_[v0] > prio_[v1]; }

            int get_heap_position(SurfaceMesh::Vertex v) { return pos_[v]; }

            void set_heap_position(SurfaceMesh::Vertex v, int pos) { pos_[v] = pos; }

        private:
            SurfaceMesh::VertexProperty<float> prio_;
            SurfaceMesh::VertexProperty<int> pos_;
        };

        typedef Heap<SurfaceMesh::Vertex, HeapInterface> PriorityQueue;

        typedef std::vector<vec3> Points;

    private: //-------------------------------------------------- private functions
        // put the vertex v in the priority queue
        void enqueue_vertex(SurfaceMesh::Vertex v);

        // is collapsing the halfedge h allowed?
        bool is_collapse_legal(const CollapseData &cd);

        // what is the priority of collapsing the halfedge h
        float priority(const CollapseData &cd);

        // postprocess halfedge collapse
        void postprocess_collapse(const CollapseData &cd);

        // compute aspect ratio for face f
        float aspect_ratio(SurfaceMesh::Face f) const;

        // compute distance from p to triagle f
        float distance(SurfaceMesh::Face f, const vec3 &p) const;

    private: //------------------------------------------------------- private data
        SurfaceMesh *mesh_;

        bool initialized_;

        SurfaceMesh::VertexProperty<float> vpriority_;
        SurfaceMesh::VertexProperty<SurfaceMesh::Halfedge> vtarget_;
        SurfaceMesh::VertexProperty<int> heap_pos_;
        SurfaceMesh::VertexProperty<Quadric> vquadric_;
        SurfaceMesh::FaceProperty<NormalCone> normal_cone_;
        SurfaceMesh::FaceProperty<Points> face_points_;

        SurfaceMesh::VertexProperty<vec3> vpoint_;
        SurfaceMesh::FaceProperty<vec3> fnormal_;
        SurfaceMesh::VertexProperty<bool> vselected_;
        SurfaceMesh::VertexProperty<bool> vfeature_;
        SurfaceMesh::EdgeProperty<bool> efeature_;

        PriorityQueue *queue_;

        bool has_selection_;
        bool has_features_;
        float normal_deviation_;
        float hausdorff_error_;
        float aspect_ratio_;
        float edge_length_;
        unsigned int max_valence_;
    };

//=============================================================================
} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_SIMPLIFICATION_H