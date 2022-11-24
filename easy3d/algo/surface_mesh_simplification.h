/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_SIMPLIFICATION_H
#define EASY3D_ALGO_SURFACE_MESH_SIMPLIFICATION_H

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/heap.h>

#include <set>
#include <vector>


namespace easy3d {

    //! A quadric as a symmetric 4x4 matrix. Used by the error quadric mesh decimation algorithms.
    class Quadric {
    public:
        //! construct quadric from upper triangle of symmetric 4x4 matrix
        Quadric(double a, double b, double c, double d,
                double e, double f, double g,
                double h, double i,
                double j)
                : a_(a), b_(b), c_(c), d_(d),
                  e_(e), f_(f), g_(g),
                  h_(h), i_(i),
                  j_(j) {}

        //! constructor quadric from given plane equation: ax+by+cz+d=0
        explicit Quadric(double a = 0.0, double b = 0.0, double c = 0.0, double d = 0.0)
                : a_(a * a), b_(a * b), c_(a * c), d_(a * d),
                  e_(b * b), f_(b * c), g_(b * d),
                  h_(c * c), i_(c * d),
                  j_(d * d) {}

        //! construct from point and normal specifying a plane
        Quadric(const vec3 &n, const vec3 &p) {
            *this = Quadric(n[0], n[1], n[2], -dot(n, p));
        }

        //! set all matrix entries to zero
        void clear() { a_ = b_ = c_ = d_ = e_ = f_ = g_ = h_ = i_ = j_ = 0.0; }

        //! add given quadric to this quadric
        Quadric &operator+=(const Quadric &q) {
            a_ += q.a_;
            b_ += q.b_;
            c_ += q.c_;
            d_ += q.d_;
            e_ += q.e_;
            f_ += q.f_;
            g_ += q.g_;
            h_ += q.h_;
            i_ += q.i_;
            j_ += q.j_;
            return *this;
        }

        //! multiply quadric by a scalar
        Quadric &operator*=(double s) {
            a_ *= s;
            b_ *= s;
            c_ *= s;
            d_ *= s;
            e_ *= s;
            f_ *= s;
            g_ *= s;
            h_ *= s;
            i_ *= s;
            j_ *= s;
            return *this;
        }

        //! evaluate quadric Q at position p by computing (p^T * Q * p)
        double operator()(const vec3 &p) const {
            const double x(p[0]), y(p[1]), z(p[2]);
            return a_ * x * x + 2.0 * b_ * x * y + 2.0 * c_ * x * z + 2.0 * d_ * x
                   + e_ * y * y + 2.0 * f_ * y * z + 2.0 * g_ * y
                   + h_ * z * z + 2.0 * i_ * z
                   + j_;
        }

    private:

        double a_, b_, c_, d_,
                e_, f_, g_,
                h_, i_,
                j_;
    };

    //=============================================================================

    /// A class implementing a normal cone.
    class NormalCone {
    public:
        //! default constructor (not initialized)
        NormalCone() = default;

        //! Initialize cone with center (unit vector) and angle (radius in radians)
        explicit NormalCone(const vec3 &normal, float angle = 0.0)
                : center_normal_(normal), angle_(angle) {
        }

        //! returns center normal
        const vec3 &center_normal() const { return center_normal_; }

        //! returns size of cone (radius in radians)
        float angle() const { return angle_; }

        //! merge *this with n.
        NormalCone &merge(const vec3 &n) { return merge(NormalCone(n)); }

        //! merge *this with nc. *this will then enclose both cones.
        NormalCone &merge(const NormalCone &nc) {
            const float dp = dot(center_normal_, nc.center_normal_);

            // axes point in same direction
            if (dp > 0.99999) {
                angle_ = std::max(angle_, nc.angle_);
            }

                // axes point in opposite directions
            else if (dp < -0.99999) {
                angle_ = 2 * M_PI;
            } else {
                // new angle
                float center_angle = std::acos(dp);
                float min_angle = std::min(-angle_, center_angle - nc.angle_);
                float max_angle = std::max(angle_, center_angle + nc.angle_);
                angle_ = 0.5f * (max_angle - min_angle);

                // axis by SLERP
                float axis_angle = 0.5f * (min_angle + max_angle);
                center_normal_ = ((center_normal_ * std::sin(center_angle - axis_angle) +
                                   nc.center_normal_ * std::sin(axis_angle)) /
                                  std::sin(center_angle));
            }

            return *this;
        }

    private:
        vec3 center_normal_;
        float angle_;
    };


    //=============================================================================

    /**
     * \brief Surface mesh simplification based on approximation error and fairness criteria.
     * \class SurfaceMeshSimplification easy3d/algo/surface_mesh_simplification.h
     * \details It performs incremental greedy mesh simplification based on halfedge collapses. See the following paper
     * for more details:
     *  - Michael Garland and Paul Seagrave Heckbert. Surface simplification using quadric error metrics. SIGGRAPH 1997.
     *  - Leif Kobbelt et al. A general framework for mesh decimation. In Proceedings of Graphics Interface, 1998.
     */
    class SurfaceMeshSimplification {
    public:
        //! Construct with mesh to be simplified.
        explicit SurfaceMeshSimplification(SurfaceMesh *mesh);

        // destructor
        ~SurfaceMeshSimplification();

        //! Initialize with given parameters.
        void initialize(float aspect_ratio = 0.0, float edge_length = 0.0,
                        unsigned int max_valence = 0, float normal_deviation = 0.0,
                        float hausdorff_error = 0.0);

        //! Simplify mesh to \p n vertices.
        void simplify(unsigned int n_vertices);

    private:
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

    private:
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

        // compute distance from p to triangle f
        float distance(SurfaceMesh::Face f, const vec3 &p) const;

    private:
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

} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_SIMPLIFICATION_H