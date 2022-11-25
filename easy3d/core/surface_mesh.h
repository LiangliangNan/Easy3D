/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2013 by Graphics & Geometry Group, Bielefeld University
 * Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
 *
 * The code in this file is partly from Surface_mesh (v1.1) with
 * modifications and enhancement:
 *      https://opensource.cit-ec.de/projects/surface_mesh
 * The original code was distributed under the GNU GPL License.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation, version 2.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ********************************************************************/

#ifndef EASY3D_CORE_SURFACE_MESH_H
#define EASY3D_CORE_SURFACE_MESH_H

#include <easy3d/core/model.h>
#include <easy3d/core/types.h>
#include <easy3d/core/property.h>

namespace easy3d {

    /**
     * @brief A halfedge data structure for polygonal meshes of 2-manifold.
     *
     * @details A surface mesh is a consistent and orientable polygonal mesh that may have one or more boundaries.
     *      The faces are simple polygons and the edges are line segments. Each edge connects two vertices, and is
     *      shared by two faces (including the null face for boundary edges). A surface mesh can have any number
     *      of connected components and also some self-intersections.
     *
     * @note The construction of a manifold surface mesh can be done by iteratively calling add_vertex() and
     *      add_face(). These two methods can ONLY be used when you're sure that the mesh is manifold. Otherwise,
     *      SurfaceMeshBuilder should be used for the construction, which guarantees you end up with a polygonal
     *      mesh of a 2-manifold topology. In any case, client code is highly recommended to use SurfaceMeshBuilder.
     *
     * \class SurfaceMesh easy3d/core/surface_mesh.h
     * \sa SurfaceMeshBuilder.
     */

    class SurfaceMesh : public virtual Model
    {

    public: //------------------------------------------------------ topology types


        /// Base class for all topology types (internally it is basically an index)
        /// \sa Vertex, Halfedge, Edge, Face
        class BaseHandle
        {
        public:

            /// constructor
            explicit BaseHandle(int _idx=-1) : idx_(_idx) {}

            /// Get the underlying index of this handle
            int idx() const { return idx_; }

            /// reset handle to be invalid (index=-1)
            void reset() { idx_=-1; }

            /// return whether the handle is valid, i.e., the index is not equal to -1.
            bool is_valid() const { return idx_ != -1; }

            /// are two handles equal?
            bool operator==(const BaseHandle& _rhs) const {
                return idx_ == _rhs.idx_;
            }

            /// are two handles different?
            bool operator!=(const BaseHandle& _rhs) const {
                return idx_ != _rhs.idx_;
            }

            /// compare operator useful for sorting handles
            bool operator<(const BaseHandle& _rhs) const {
                return idx_ < _rhs.idx_;
            }

            /// helper structure to be able to use std::unordered_map
            struct Hash {
                std::size_t operator()(const BaseHandle& h) const { return h.idx(); }
            };

        private:
            friend class SurfaceMesh;
            int idx_;
        };


        /// this type represents a vertex (internally it is basically an index)
        ///  \sa Halfedge, Edge, Face
        struct Vertex : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };


        /// this type represents a halfedge (internally it is basically an index)
        /// \sa Vertex, Edge, Face
        struct Halfedge : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Halfedge(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'h' << idx(); }
        };


        /// this type represents an edge (internally it is basically an index)
        /// \sa Vertex, Halfedge, Face
        struct Edge : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Edge(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'e' << idx(); }
        };


        /// this type represents a face (internally it is basically an index)
        /// \sa Vertex, Halfedge, Edge
        struct Face : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Face(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'f' << idx(); }
        };




    public: //-------------------------------------------------- connectivity types

        /// This type stores the vertex connectivity
        /// \sa HalfedgeConnectivity, FaceConnectivity
        struct VertexConnectivity
        {
            /// an outgoing halfedge per vertex (it will be a boundary halfedge for boundary vertices)
            Halfedge  halfedge_;
        };


        /// This type stores the halfedge connectivity
        /// \sa VertexConnectivity, FaceConnectivity
        struct HalfedgeConnectivity
        {
            /// face incident to halfedge
            Face      face_;
            /// vertex the halfedge points to
            Vertex    vertex_;
            /// next halfedge within a face (or along a boundary)
            Halfedge  next_;
            /// previous halfedge within a face (or along a boundary)
            Halfedge  prev_;
        };


        /// This type stores the face connectivity
        /// \sa VertexConnectivity, HalfedgeConnectivity
        struct FaceConnectivity
        {
            /// a halfedge that is part of the face
            Halfedge  halfedge_;
        };




    public: //------------------------------------------------------ property types

        /// Vertex property of type T
        /// \sa HalfedgeProperty, EdgeProperty, FaceProperty
        template <class T> class VertexProperty : public Property<T>
        {
        public:

            /// default constructor
            VertexProperty() = default;
            explicit VertexProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for vertex \c v
            typename Property<T>::reference operator[](Vertex v)
            {
                return Property<T>::operator[](v.idx());
            }

            /// access the data stored for vertex \c v
            typename Property<T>::const_reference operator[](Vertex v) const
            {
                return Property<T>::operator[](v.idx());
            }
        };


        /// Halfedge property of type T
        /// \sa VertexProperty, EdgeProperty, FaceProperty
        template <class T> class HalfedgeProperty : public Property<T>
        {
        public:

            /// default constructor
            HalfedgeProperty() = default;
            explicit HalfedgeProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for halfedge \c h
            typename Property<T>::reference operator[](Halfedge h)
            {
                return Property<T>::operator[](h.idx());
            }

            /// access the data stored for halfedge \c h
            typename Property<T>::const_reference operator[](Halfedge h) const
            {
                return Property<T>::operator[](h.idx());
            }
        };


        /// Edge property of type T
        /// \sa VertexProperty, HalfedgeProperty, FaceProperty
        template <class T> class EdgeProperty : public Property<T>
        {
        public:

            /// default constructor
            EdgeProperty() = default;
            explicit EdgeProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for edge \c e
            typename Property<T>::reference operator[](Edge e)
            {
                return Property<T>::operator[](e.idx());
            }

            /// access the data stored for edge \c e
            typename Property<T>::const_reference operator[](Edge e) const
            {
                return Property<T>::operator[](e.idx());
            }
        };


        /// Face property of type T
        /// \sa VertexProperty, HalfedgeProperty, EdgeProperty
        template <class T> class FaceProperty : public Property<T>
        {
        public:

            /// default constructor
            FaceProperty() = default;
            explicit FaceProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for face \c f
            typename Property<T>::reference operator[](Face f)
            {
                return Property<T>::operator[](f.idx());
            }

            /// access the data stored for face \c f
            typename Property<T>::const_reference operator[](Face f) const
            {
                return Property<T>::operator[](f.idx());
            }
        };


        /// Mesh property of type T
        /// \sa VertexProperty, HalfedgeProperty, EdgeProperty
        template <class T> class ModelProperty : public Property<T>
        {
        public:

            /// default constructor
            ModelProperty() = default;
            explicit ModelProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for the mesh
            typename Property<T>::reference operator[](size_t idx)
            {
                return Property<T>::operator[](idx);
            }

            /// access the data stored for the mesh
            typename Property<T>::const_reference operator[](size_t idx) const
            {
                return Property<T>::operator[](idx);
            }
        };



    public: //------------------------------------------------------ iterator types

        /// This class iterates linearly over all vertices
        /// \sa vertices_begin(), vertices_end()
        /// \sa HalfedgeIterator, EdgeIterator, FaceIterator
        class VertexIterator
        {
        public:

            /// Default constructor
            explicit VertexIterator(Vertex v=Vertex(), const SurfaceMesh* m=nullptr) : hnd_(v), mesh_(m)
            {
                if (mesh_ && mesh_->has_garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            }

            /// get the vertex the iterator refers to
            Vertex operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const VertexIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const VertexIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            VertexIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /// pre-decrement iterator
            VertexIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
                return *this;
            }

        private:
            Vertex  hnd_;
            const SurfaceMesh* mesh_;
        };


        /// This class iterates linearly over all halfedges
        /// \sa halfedges_begin(), halfedges_end()
        /// \sa VertexIterator, EdgeIterator, FaceIterator
        class HalfedgeIterator
        {
        public:

            /// Default constructor
            explicit HalfedgeIterator(Halfedge h=Halfedge(), const SurfaceMesh* m=nullptr) : hnd_(h), mesh_(m)
            {
                if (mesh_ && mesh_->has_garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            }

            /// get the halfedge the iterator refers to
            Halfedge operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const HalfedgeIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const HalfedgeIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            HalfedgeIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /// pre-decrement iterator
            HalfedgeIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
                return *this;
            }

        private:
            Halfedge  hnd_;
            const SurfaceMesh* mesh_;
        };


        /// This class iterates linearly over all edges
        /// \sa edges_begin(), edges_end()
        /// \sa VertexIterator, HalfedgeIterator, FaceIterator
        class EdgeIterator
        {
        public:

            /// Default constructor
            explicit EdgeIterator(Edge e=Edge(), const SurfaceMesh* m=nullptr) : hnd_(e), mesh_(m)
            {
                if (mesh_ && mesh_->has_garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            }

            /// get the edge the iterator refers to
            Edge operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const EdgeIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const EdgeIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            EdgeIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /// pre-decrement iterator
            EdgeIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
                return *this;
            }

        private:
            Edge  hnd_;
            const SurfaceMesh* mesh_;
        };


        /// This class iterates linearly over all faces
        /// \sa faces_begin(), faces_end()
        /// \sa VertexIterator, HalfedgeIterator, EdgeIterator
        class FaceIterator
        {
        public:

            /// Default constructor
            explicit FaceIterator(Face f=Face(), const SurfaceMesh* m=nullptr) : hnd_(f), mesh_(m)
            {
                if (mesh_ && mesh_->has_garbage()) while (mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
            }

            /// get the face the iterator refers to
            Face operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const FaceIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const FaceIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            FaceIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /// pre-decrement iterator
            FaceIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                while (mesh_->has_garbage() && mesh_->is_valid(hnd_) && mesh_->is_deleted(hnd_)) --hnd_.idx_;
                return *this;
            }

        private:
            Face  hnd_;
            const SurfaceMesh* mesh_;
        };



    public: //-------------------------- containers for C++11 range-based for loops

        /// This helper class is a container for iterating through all
        /// vertices using C++11 range-based for-loops.
        /// \sa vertices()
        class VertexContainer
        {
        public:
            VertexContainer(VertexIterator _begin, VertexIterator _end) : begin_(_begin), end_(_end) {}
            VertexIterator begin() const { return begin_; }
            VertexIterator end()   const { return end_;   }
        private:
            VertexIterator begin_, end_;
        };



        /// This helper class is a container for iterating through all
        /// halfedge using C++11 range-based for-loops.
        /// \sa halfedges()
        class HalfedgeContainer
        {
        public:
            HalfedgeContainer(HalfedgeIterator _begin, HalfedgeIterator _end) : begin_(_begin), end_(_end) {}
            HalfedgeIterator begin() const { return begin_; }
            HalfedgeIterator end()   const { return end_;   }
        private:
            HalfedgeIterator begin_, end_;
        };



        /// This helper class is a container for iterating through all
        /// edges using C++11 range-based for-loops.
        /// \sa edges()
        class EdgeContainer
        {
        public:
            EdgeContainer(EdgeIterator _begin, EdgeIterator _end) : begin_(_begin), end_(_end) {}
            EdgeIterator begin() const { return begin_; }
            EdgeIterator end()   const { return end_;   }
        private:
            EdgeIterator begin_, end_;
        };



        /// This helper class is a container for iterating through all
        /// faces using C++11 range-based for-loops.
        /// \sa faces()
        class FaceContainer
        {
        public:
            FaceContainer(FaceIterator _begin, FaceIterator _end) : begin_(_begin), end_(_end) {}
            FaceIterator begin() const { return begin_; }
            FaceIterator end()   const { return end_;   }
        private:
            FaceIterator begin_, end_;
        };



    public: //---------------------------------------------------- circulator types

        /**
         * This class circulates through all one-ring neighbors of a vertex.
         * It also acts as a container-concept for C++11 range-based for loops.
         *
         * The follow code shows how to use VertexAroundVertexCirculator:
         * \code
         *      SurfaceMesh::VertexAroundVertexCirculator cir(mesh, v);
         *      SurfaceMesh::VertexAroundVertexCirculator end = cir;
         *      do {
         *          SurfaceMesh::Vertex v = *cir;
         *          // do something with v
         *          ++cir;
         *      } while (cir != end);
         * \endcode
         * \sa HalfedgeAroundVertexCirculator, FaceAroundVertexCirculator, vertices()
         */
        class VertexAroundVertexCirculator
        {
        public:

            /// default constructor
            explicit VertexAroundVertexCirculator(const SurfaceMesh* m=nullptr, Vertex v=Vertex())
            : mesh_(m), active_(true)
            {
                if (mesh_) halfedge_ = mesh_->out_halfedge(v);
            }

            /// are two circulators equal?
            bool operator==(const VertexAroundVertexCirculator& rhs) const
            {
                assert(mesh_);
                return (active_ && (mesh_==rhs.mesh_) && (halfedge_==rhs.halfedge_));
            }

            /// are two circulators different?
            bool operator!=(const VertexAroundVertexCirculator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment (rotate counter-clockwise)
            VertexAroundVertexCirculator& operator++()
            {
                assert(mesh_);
                halfedge_ = mesh_->prev_around_source(halfedge_);
                active_ = true;
                return *this;
            }

            /// pre-decrement (rotate clockwise)
            VertexAroundVertexCirculator& operator--()
            {
                assert(mesh_);
                halfedge_ = mesh_->next_around_source(halfedge_);
                return *this;
            }

            /// get the vertex the circulator refers to
            Vertex operator*()  const
            {
                assert(mesh_);
                return mesh_->target(halfedge_);
            }

            /// cast to bool: true if vertex is not isolated
            operator bool() const { return halfedge_.is_valid(); }

            /// return current halfedge
            Halfedge halfedge() const { return halfedge_; }

            // helper for C++11 range-based for-loops
            VertexAroundVertexCirculator& begin() { active_=!halfedge_.is_valid(); return *this; }
            // helper for C++11 range-based for-loops
            VertexAroundVertexCirculator& end()   { active_=true;  return *this; }

        private:
            const SurfaceMesh*  mesh_;
            Halfedge         halfedge_;
            // helper for C++11 range-based for-loops
            bool active_;
        };


        /**
         * This class circulates through all outgoing halfedges of a vertex.
         * It also acts as a container-concept for C++11 range-based for loops.
         *
         * The follow code shows how to use HalfedgeAroundVertexCirculator:
         * \code
         *      SurfaceMesh::HalfedgeAroundVertexCirculator cir(mesh, v);
         *      SurfaceMesh::HalfedgeAroundVertexCirculator end = cir;
         *      do {
         *          SurfaceMesh::Halfedge h = *cir;
         *          // do something with h
         *          ++cir;
         *      } while (cir != end);
         * \endcode
         * \sa VertexAroundVertexCirculator, FaceAroundVertexCirculator, halfedges()
         */
        class HalfedgeAroundVertexCirculator
        {
        public:

            /// default constructor
            explicit HalfedgeAroundVertexCirculator(const SurfaceMesh* m=nullptr, Vertex v=Vertex())
            : mesh_(m), active_(true)
            {
                if (mesh_) halfedge_ = mesh_->out_halfedge(v);
            }

            /// are two circulators equal?
            bool operator==(const HalfedgeAroundVertexCirculator& rhs) const
            {
                assert(mesh_);
                return (active_ && (mesh_==rhs.mesh_) && (halfedge_==rhs.halfedge_));
            }

            /// are two circulators different?
            bool operator!=(const HalfedgeAroundVertexCirculator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment (rotate counter-clockwise)
            HalfedgeAroundVertexCirculator& operator++()
            {
                assert(mesh_);
                halfedge_ = mesh_->prev_around_source(halfedge_);
                active_ = true;
                return *this;
            }

            /// pre-decrement (rotate clockwise)
            HalfedgeAroundVertexCirculator& operator--()
            {
                assert(mesh_);
                halfedge_ = mesh_->next_around_source(halfedge_);
                return *this;
            }

            /// get the halfedge the circulator refers to
            Halfedge operator*() const { return halfedge_; }

            /// cast to bool: true if vertex is not isolated
            operator bool() const { return halfedge_.is_valid(); }

            // helper for C++11 range-based for-loops
            HalfedgeAroundVertexCirculator& begin() { active_=!halfedge_.is_valid(); return *this; }
            // helper for C++11 range-based for-loops
            HalfedgeAroundVertexCirculator& end()   { active_=true;  return *this; }

        private:
            const SurfaceMesh*  mesh_;
            Halfedge         halfedge_;
            // helper for C++11 range-based for-loops
            bool active_;
        };


        /**
         * This class circulates through all incident faces of a vertex.
         * It also acts as a container-concept for C++11 range-based for loops.
         *
         * The follow code shows how to use FaceAroundVertexCirculator:
         * \code
         *      SurfaceMesh::FaceAroundVertexCirculator cir(mesh, v);
         *      SurfaceMesh::FaceAroundVertexCirculator end = cir;
         *      do {
         *          SurfaceMesh::Face f = *cir;
         *          // do something with f
         *          ++cir;
         *      } while (cir != end);
         * \endcode
         * \sa VertexAroundVertexCirculator, HalfedgeAroundVertexCirculator, faces()
         */
        class FaceAroundVertexCirculator
        {
        public:

            /// construct with mesh and vertex (vertex should not be isolated!)
            explicit FaceAroundVertexCirculator(const SurfaceMesh* m=nullptr, Vertex v=Vertex())
            : mesh_(m), active_(true)
            {
                if (mesh_)
                {
                    halfedge_ = mesh_->out_halfedge(v);
                    if (halfedge_.is_valid() && mesh_->is_border(halfedge_))
                        operator++();
                }
            }

            /// are two circulators equal?
            bool operator==(const FaceAroundVertexCirculator& rhs) const
            {
                assert(mesh_);
                return (active_ && (mesh_==rhs.mesh_) && (halfedge_==rhs.halfedge_));
            }

            /// are two circulators different?
            bool operator!=(const FaceAroundVertexCirculator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment (rotates counter-clockwise)
            FaceAroundVertexCirculator& operator++()
            {
                assert(mesh_ && halfedge_.is_valid());
                do {
                    halfedge_ = mesh_->prev_around_source(halfedge_);
                } while (mesh_->is_border(halfedge_));
                active_ = true;
                return *this;
            }

            /// pre-decrement (rotate clockwise)
            FaceAroundVertexCirculator& operator--()
            {
                assert(mesh_ && halfedge_.is_valid());
                do
                    halfedge_ = mesh_->next_around_source(halfedge_);
                while (mesh_->is_border(halfedge_));
                return *this;
            }

            /// get the face the circulator refers to
            Face operator*() const
            {
                assert(mesh_ && halfedge_.is_valid());
                return mesh_->face(halfedge_);
            }

            /// cast to bool: true if vertex is not isolated
            operator bool() const { return halfedge_.is_valid(); }

            // helper for C++11 range-based for-loops
            FaceAroundVertexCirculator& begin() { active_=!halfedge_.is_valid(); return *this; }
            // helper for C++11 range-based for-loops
            FaceAroundVertexCirculator& end()   { active_=true;  return *this; }

        private:
            const SurfaceMesh*  mesh_;
            Halfedge         halfedge_;
            // helper for C++11 range-based for-loops
            bool active_;
        };


        /**
         * This class circulates through the vertices of a face.
         * It also acts as a container-concept for C++11 range-based for loops.
         *
         * The follow code shows how to use VertexAroundFaceCirculator:
         * \code
         *      SurfaceMesh::VertexAroundFaceCirculator cir = mesh->vertices(f);
         *      SurfaceMesh::VertexAroundFaceCirculator end = cir;
         *      do {
         *          SurfaceMesh::Vertex v = *cir;
         *          // do something with v
         *          ++cir;
         *      } while (cir != end);
         * \endcode
         * \sa HalfedgeAroundFaceCirculator, vertices()
         */
        class VertexAroundFaceCirculator
        {
        public:

            /// default constructor
            explicit VertexAroundFaceCirculator(const SurfaceMesh* m=nullptr, Face f=Face())
            : mesh_(m), active_(true)
            {
                if (mesh_) halfedge_ = mesh_->halfedge(f);
            }

            /// are two circulators equal?
            bool operator==(const VertexAroundFaceCirculator& rhs) const
            {
                assert(mesh_);
                return (active_ && (mesh_==rhs.mesh_) && (halfedge_==rhs.halfedge_));
            }

            /// are two circulators different?
            bool operator!=(const VertexAroundFaceCirculator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment (rotates counter-clockwise)
            VertexAroundFaceCirculator& operator++()
            {
                assert(mesh_ && halfedge_.is_valid());
                halfedge_ = mesh_->next(halfedge_);
                active_ = true;
                return *this;
            }

            /// pre-decrement (rotates clockwise)
            VertexAroundFaceCirculator& operator--()
            {
                assert(mesh_ && halfedge_.is_valid());
                halfedge_ = mesh_->prev(halfedge_);
                return *this;
            }

            /// get the vertex the circulator refers to
            Vertex operator*() const
            {
                assert(mesh_ && halfedge_.is_valid());
                return mesh_->target(halfedge_);
            }

            // helper for C++11 range-based for-loops
            VertexAroundFaceCirculator& begin() { active_=false; return *this; }
            // helper for C++11 range-based for-loops
            VertexAroundFaceCirculator& end()   { active_=true;  return *this; }

        private:
            const SurfaceMesh*  mesh_;
            Halfedge         halfedge_;
            // helper for C++11 range-based for-loops
            bool active_;
        };


        /**
         * This class circulates through all halfedges of a face.
         * It also acts as a container-concept for C++11 range-based for loops.
         *
         * The following code shows how to use HalfedgeAroundFaceCirculator:
         * \code
         *      SurfaceMesh::HalfedgeAroundFaceCirculator cir(mesh, f);
         *      SurfaceMesh::HalfedgeAroundFaceCirculator end = cir;
         *      do {
         *          SurfaceMesh::Halfedge h = *cir;
         *          // do something with h
         *          ++cir;
         *      } while (cir != end);
         * \endcode
         * \sa VertexAroundFaceCirculator, halfedges()
         */
        class HalfedgeAroundFaceCirculator
        {
        public:

            /// default constructor
            explicit HalfedgeAroundFaceCirculator(const SurfaceMesh* m=nullptr, Face f=Face())
            : mesh_(m), active_(true)
            {
                if (mesh_) halfedge_ = mesh_->halfedge(f);
            }

            /// are two circulators equal?
            bool operator==(const HalfedgeAroundFaceCirculator& rhs) const
            {
                assert(mesh_);
                return (active_ && (mesh_==rhs.mesh_) && (halfedge_==rhs.halfedge_));
            }

            /// are two circulators different?
            bool operator!=(const HalfedgeAroundFaceCirculator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment (rotates counter-clockwise)
            HalfedgeAroundFaceCirculator& operator++()
            {
                assert(mesh_ && halfedge_.is_valid());
                halfedge_ = mesh_->next(halfedge_);
                active_ = true;
                return *this;
            }

            /// pre-decrement (rotates clockwise)
            HalfedgeAroundFaceCirculator& operator--()
            {
                assert(mesh_ && halfedge_.is_valid());
                halfedge_ = mesh_->prev(halfedge_);
                return *this;
            }

            /// get the halfedge the circulator refers to
            Halfedge operator*() const { return halfedge_; }

            // helper for C++11 range-based for-loops
            HalfedgeAroundFaceCirculator& begin() { active_=false; return *this; }
            // helper for C++11 range-based for-loops
            HalfedgeAroundFaceCirculator& end()   { active_=true;  return *this; }

        private:
            const SurfaceMesh*  mesh_;
            Halfedge         halfedge_;
            // helper for C++11 range-based for-loops
            bool active_;
        };



    public: //-------------------------------------------- constructor / destructor

        /// \name Construct, destruct, assignment
        //@{

        /// default constructor
        SurfaceMesh();

        /// destructor (is virtual, since we inherit from Geometry_representation)
        ~SurfaceMesh() override = default;

        /// copy constructor: copies \c rhs to \c *this. performs a deep copy of all properties.
        SurfaceMesh(const SurfaceMesh& rhs) { operator=(rhs); }

        /// assign \c rhs to \c *this. performs a deep copy of all properties.
        SurfaceMesh& operator=(const SurfaceMesh& rhs);

        /// \brief Merges another surface mesh into the current one.
        /// Shifts the indices of vertices of the other mesh by `number_of_vertices() + number_of_removed_vertices()`
        /// and analogously for halfedges, edges, and faces.
        /// Copies entries of all property maps which have the same name in both meshes. That is, properties maps which
        /// are only in `other` are ignored.
        /// Also copies elements which are marked as removed, and concatenates the freelists of both meshes.
        SurfaceMesh& operator+=(const SurfaceMesh& other) { join(other); return *this; }

        /// \brief Merges another surface mesh into the current one.
        /// Shifts the indices of vertices of the other mesh by `number_of_vertices() + number_of_removed_vertices()`
        /// and analogously for halfedges, edges, and faces.
        /// Copies entries of all property maps which have the same name in both meshes. That is, properties maps which
        /// are only in `other` are ignored.
        /// Also copies elements which are marked as removed, and concatenates the freelists of both meshes.
        SurfaceMesh& join(const SurfaceMesh& other);

        /// assign \c rhs to \c *this. does not copy custom properties.
        SurfaceMesh& assign(const SurfaceMesh& rhs);
        //@}

        //! \name File IO
        //!@{

        //! \brief Read mesh from a SM file \p filename.
        //! Mainly for quick debug purposes. Client code should use SurfaceMeshIO.
        //! \sa SurfaceMeshIO.
        bool read(const std::string& filename);

        //! \brief Write mesh to a SM file \p filename.
        //! Mainly for quick debug purposes. Client code should use SurfaceMeshIO.
        //! \sa SurfaceMeshIO.
        bool write(const std::string& filename) const;
        //@}

    public: //----------------------------------------------- add new vertex / face

        /// \name Add new elements by hand
        //@{

        /// add a new vertex with position \c p
        Vertex add_vertex(const vec3& p) { Vertex v = new_vertex(); vpoint_[v] = p; return v; }

        /// add a new face with vertex list \c vertices
        /// \param vertices The input vertices created by add_vertex().
        /// \sa add_triangle, add_quad
        Face add_face(const std::vector<Vertex>& vertices);

        /// add a new triangle connecting vertices \c v1, \c v2, \c v3
        /// \param {v1, v2, v3} The input vertices created by add_vertex().
        /// \sa add_face, add_quad
        Face add_triangle(Vertex v1, Vertex v2, Vertex v3);

        /// add a new quad connecting vertices \c v1, \c v2, \c v3, \c v4
        /// \param {v1, v2, v3, v4} The input vertices created by add_vertex().
        /// \sa add_triangle, add_face
        Face add_quad(Vertex v1, Vertex v2, Vertex v3, Vertex v4);

        //@}


    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /// returns number of (deleted and valid) vertices in the mesh
        unsigned int vertices_size() const { return (unsigned int) vprops_.size(); }
        /// returns number of (deleted and valid)halfedge in the mesh
        unsigned int halfedges_size() const { return (unsigned int) hprops_.size(); }
        /// returns number of (deleted and valid)edges in the mesh
        unsigned int edges_size() const { return (unsigned int) eprops_.size(); }
        /// returns number of (deleted and valid)faces in the mesh
        unsigned int faces_size() const { return (unsigned int) fprops_.size(); }


        /// returns number of vertices in the mesh
        unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }
        /// returns number of halfedge in the mesh
        unsigned int n_halfedges() const { return halfedges_size() - 2*deleted_edges_; }
        /// returns number of edges in the mesh
        unsigned int n_edges() const { return edges_size() - deleted_edges_; }
        /// returns number of faces in the mesh
        unsigned int n_faces() const { return faces_size() - deleted_faces_; }

        /// \brief Removes all vertices, edges, faces, and properties (and resets garbage state).
        /// \details After calling this method, the mesh is the same as newly constructed. The additional properties
        /// (such as normal vectors) are also removed and must thus be re-added if needed.
        void clear();

        /// reserves memory (mainly used in file readers)
        void reserve(unsigned int nvertices,
                     unsigned int nedges,
                     unsigned int nfaces );

        /// resizes space for vertices, halfedges, edges, faces, and their currently
        /// associated properties.
        /// Note: ne is the number of edges. for halfedges, nh = 2 * ne. */
        void resize(unsigned int nv, unsigned int ne, unsigned int nf) {
            vprops_.resize(nv);
            hprops_.resize(2 * ne);
            eprops_.resize(ne);
            fprops_.resize(nf);
        }

        /// are there deleted vertices, edges or faces?
        bool has_garbage() const { return garbage_; }

        /// remove deleted vertices/edges/faces
        void collect_garbage();


        /// returns whether vertex \c v is deleted
        /// \sa collect_garbage()
        bool is_deleted(Vertex v) const
        {
            return vdeleted_[v];
        }
        /// returns whether halfedge \c h is deleted
        /// \sa collect_garbage()
        bool is_deleted(Halfedge h) const
        {
            return edeleted_[edge(h)];
        }
        /// returns whether edge \c e is deleted
        /// \sa collect_garbage()
        bool is_deleted(Edge e) const
        {
            return edeleted_[e];
        }
        /// returns whether face \c f is deleted
        /// \sa collect_garbage()
        bool is_deleted(Face f) const
        {
            return fdeleted_[f];
        }


        /// return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Vertex v) const
        {
            return (0 <= v.idx()) && (v.idx() < (int)vertices_size());
        }
        /// return whether halfedge \c h is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Halfedge h) const
        {
            return (0 <= h.idx()) && (h.idx() < (int)halfedges_size());
        }
        /// return whether edge \c e is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Edge e) const
        {
            return (0 <= e.idx()) && (e.idx() < (int)edges_size());
        }
        /// return whether face \c f is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Face f) const
        {
            return (0 <= f.idx()) && (f.idx() < (int)faces_size());
        }

        //@}




    public: //---------------------------------------------- low-level connectivity

        /// \name Low-level connectivity
        //@{

        /// returns an outgoing halfedge of vertex \c v.
        /// if \c v is a boundary vertex this will be a boundary halfedge.
        Halfedge out_halfedge(Vertex v) const
        {
            return vconn_[v].halfedge_;
        }

        /// set the outgoing halfedge of vertex \c v to \c h
        void set_out_halfedge(Vertex v, Halfedge h)
        {
            vconn_[v].halfedge_ = h;
        }

        /// returns whether \c v is a boundary vertex
        bool is_border(Vertex v) const
        {
            Halfedge h(out_halfedge(v));
            return (!(h.is_valid() && face(h).is_valid()));
        }

        /// returns whether \c v is isolated, i.e., not incident to any face
        bool is_isolated(Vertex v) const
        {
            return !out_halfedge(v).is_valid();
        }

        /// returns whether \c v is a manifold vertex (not incident to several patches)
        bool is_manifold(Vertex v) const
        {
            // [Liangliang: I doubt. It should also check if more than 1 cones meet at the same vertex.
            //              See the "resolve_non_manifold_vertices()" in manifold_builder.cpp

            // The vertex is non-manifold if more than one gap exists, i.e.
            // more than one outgoing boundary halfedge.
            int n(0);
            HalfedgeAroundVertexCirculator hit = halfedges(v), hend=hit;
            if (hit) do
            {
                if (is_border(*hit))
                    ++n;
            }
            while (++hit!=hend);
            return n<2;
        }

        /// returns whether \c f is degenerate
        bool is_degenerate(Face f) const;

        /// returns the vertex the halfedge \c h points to
        Vertex target(Halfedge h) const
        {
            return hconn_[h].vertex_;
        }

        /// returns the vertex the halfedge \c h emanates from
        Vertex source(Halfedge h) const
        {
            return target(opposite(h));
        }

        /// sets the vertex the halfedge \c h points to to \c v
        void set_target(Halfedge h, Vertex v)
        {
            hconn_[h].vertex_ = v;
        }

        /// returns the face incident to halfedge \c h
        Face face(Halfedge h) const
        {
            return hconn_[h].face_;
        }

        /// sets the incident face to halfedge \c h to \c f
        void set_face(Halfedge h, Face f)
        {
            hconn_[h].face_ = f;
        }

        /// returns the next halfedge within the incident face
        Halfedge next(Halfedge h) const
        {
            return hconn_[h].next_;
        }

        /// sets the next halfedge of \c h within the face to \c nh
        void set_next(Halfedge h, Halfedge nh)
        {
            hconn_[h].next_ = nh;
            hconn_[nh].prev_ = h;
        }

        /// returns the previous halfedge within the incident face
        Halfedge prev(Halfedge h) const
        {
            return hconn_[h].prev_;
        }

        /// returns the opposite halfedge of \c h
        Halfedge opposite(Halfedge h) const
        {
            return Halfedge((h.idx() & 1) ? h.idx()-1 : h.idx()+1);
        }

        /// returns the halfedge that is rotated \b counter-clockwise around the
        /// start vertex of \c h. it is the opposite halfedge of the previous halfedge of \c h.
        Halfedge prev_around_source(Halfedge h) const
        {
            return opposite(prev(h));
        }

        /// returns the halfedge that is rotated \b clockwise around the
        /// start vertex of \c h. it is the next halfedge of the opposite halfedge of \c h.
        Halfedge next_around_source(Halfedge h) const
        {
            return next(opposite(h));
        }

        /// returns the halfedge that is rotated \b counter-clockwise around the
        /// end vertex of \c h. it is the prev halfedge of the opposite halfedge of \c h.
        Halfedge prev_around_target(Halfedge h) const
        {
            return prev(opposite(h));
        }

        /// returns the halfedge that is rotated \b clockwise around the
        /// end vertex of \c h. it is the opposite halfedge of the next halfedge of \c h.
        Halfedge next_around_target(Halfedge h) const
        {
            return opposite(next(h));
        }

        /// return the edge that contains halfedge \c h as one of its two halfedges.
        Edge edge(Halfedge h) const
        {
            return Edge(h.idx() >> 1);
        }

        /// returns whether h is a boundary halfedge, i.e., if its face does not exist.
        bool is_border(Halfedge h) const
        {
            return !face(h).is_valid();
        }


        /// returns the \c i'th halfedge of edge \c e. \c i has to be 0 or 1.
        Halfedge halfedge(Edge e, unsigned int i) const
        {
            assert(i<=1);
            return Halfedge(static_cast<int>((e.idx() << 1) + i));
        }

        /// returns the \c i'th vertex of edge \c e. \c i has to be 0 or 1.
        Vertex vertex(Edge e, unsigned int i) const
        {
            assert(i<=1);
            return target(halfedge(e, i));
        }

        /// returns the face incident to the \c i'th halfedge of edge \c e. \c i has to be 0 or 1.
        Face face(Edge e, unsigned int i) const
        {
            assert(i<=1);
            return face(halfedge(e, i));
        }

        /// returns whether \c e is a boundary edge, i.e., if one of its
        /// halfedges is a boundary halfedge.
        bool is_border(Edge e) const
        {
            return (is_border(halfedge(e, 0)) || is_border(halfedge(e, 1)));
        }

        /// returns a halfedge of face \c f
        Halfedge halfedge(Face f) const
        {
            return fconn_[f].halfedge_;
        }

        /// sets the halfedge of face \c f to \c h
        void set_halfedge(Face f, Halfedge h)
        {
            fconn_[f].halfedge_ = h;
        }

        /// returns whether \c f is a boundary face, i.e., it one of its edges is a boundary edge.
        bool is_border(Face f) const
        {
            Halfedge h  = halfedge(f);
            Halfedge hh = h;
            do
            {
                if (is_border(opposite(h)))
                    return true;
                h = next(h);
            }
            while (h != hh);
            return false;
        }

        //@}




    public: //--------------------------------------------------- property handling

        /// \name Property handling
        //@{

        /** add a vertex property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t=T())
        {
            return VertexProperty<T>(vprops_.add<T>(name, t));
        }
        /** add a halfedge property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> HalfedgeProperty<T> add_halfedge_property(const std::string& name, const T t=T())
        {
            return HalfedgeProperty<T>(hprops_.add<T>(name, t));
        }
        /** add a edge property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> EdgeProperty<T> add_edge_property(const std::string& name, const T t=T())
        {
            return EdgeProperty<T>(eprops_.add<T>(name, t));
        }
        /** add a face property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> FaceProperty<T> add_face_property(const std::string& name, const T t=T())
        {
            return FaceProperty<T>(fprops_.add<T>(name, t));
        }
        /**
         * \brief Adds a model property of type \c T with name \c name and default value \c t.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *      In this case it returns an invalid property.
         * Example:
         *      \code
         *          auto trans = cloud->add_model_property<mat4>("transformation", mat4::identity());
         *          trans[0] = mat4::translation(-x0, -y0, -z0);
         *      \endcode
         */
        template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t=T())
        {
            return ModelProperty<T>(mprops_.add<T>(name, t));
        }


        /** get the vertex property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const
        {
            return VertexProperty<T>(vprops_.get<T>(name));
        }
        /** get the halfedge property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> HalfedgeProperty<T> get_halfedge_property(const std::string& name) const
        {
            return HalfedgeProperty<T>(hprops_.get<T>(name));
        }
        /** get the edge property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> EdgeProperty<T> get_edge_property(const std::string& name) const
        {
            return EdgeProperty<T>(eprops_.get<T>(name));
        }
        /** get the face property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> FaceProperty<T> get_face_property(const std::string& name) const
        {
            return FaceProperty<T>(fprops_.get<T>(name));
        }
        /**
         * \brief Gets the model property named \c name of type \c T.
         * \return The model property. An invalid ModelProperty will be returned if the
         *      property does not exist or if the type does not match.
         * Example:
         *      \code
         *          auto T = cloud->get_model_property<mat4>("transformation");
         *          T[0] = mat4::translation(-x0, -y0, -z0);
         *      \endcode
         */
        template <class T> ModelProperty<T> get_model_property(const std::string& name) const
        {
            return ModelProperty<T>(mprops_.get<T>(name));
        }


        /** if a vertex property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t=T())
        {
            return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
        }
        /** if a halfedge property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> HalfedgeProperty<T> halfedge_property(const std::string& name, const T t=T())
        {
            return HalfedgeProperty<T>(hprops_.get_or_add<T>(name, t));
        }
        /** if an edge property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> EdgeProperty<T> edge_property(const std::string& name, const T t=T())
        {
            return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
        }
        /** if a face property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> FaceProperty<T> face_property(const std::string& name, const T t=T())
        {
            return FaceProperty<T>(fprops_.get_or_add<T>(name, t));
        }

         /** if a model property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> ModelProperty<T> model_property(const std::string& name, const T t=T())
        {
            return ModelProperty<T>(mprops_.get_or_add<T>(name, t));
        }


        /// remove the vertex property \c p
        template<class T>
        bool remove_vertex_property(VertexProperty<T> &p) { return vprops_.remove(p); }

        /// remove the vertex property named \c n
        bool remove_vertex_property(const std::string &n) { return vprops_.remove(n); }

        /// remove the halfedge property \c p
        template<class T>
        bool remove_halfedge_property(HalfedgeProperty<T> &p) { return hprops_.remove(p); }

        /// remove the halfedge property named \c n
        bool remove_halfedge_property(const std::string &n) { return hprops_.remove(n); }

        /// remove the edge property \c p
        template<class T>
        bool remove_edge_property(EdgeProperty<T> &p) { return eprops_.remove(p); }

        /// remove the edge property named \c n
        bool remove_edge_property(const std::string &n) { return eprops_.remove(n); }

        /// remove the face property \c p
        template<class T>
        bool remove_face_property(FaceProperty<T> &p) { return fprops_.remove(p); }

        /// remove the face property named \c n
        bool remove_face_property(const std::string &n) { return fprops_.remove(n); }

        /// remove the model property \c p
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }

        /// remove the model property named \c n
        bool remove_model_property(const std::string &n) { return mprops_.remove(n); }

        /// rename a vertex property given its name
        bool rename_vertex_property(const std::string &old_name, const std::string &new_name) {
            return vprops_.rename(old_name, new_name);
        }

        /// rename a face property given its name
        bool rename_face_property(const std::string &old_name, const std::string &new_name) {
            return fprops_.rename(old_name, new_name);
        }

        /// rename an edge property given its name
        bool rename_edge_property(const std::string &old_name, const std::string &new_name) {
            return eprops_.rename(old_name, new_name);
        }

        /// rename a halfedge property given its name
        bool rename_halfedge_property(const std::string &old_name, const std::string &new_name) {
            return hprops_.rename(old_name, new_name);
        }

        /// rename a model property given its name
        bool rename_model_property(const std::string &old_name, const std::string &new_name) {
            return mprops_.rename(old_name, new_name);
        }


        /** get the type_info \c T of vertex property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_vertex_property_type(const std::string& name) const
        {
            return vprops_.get_type(name);
        }
        /** get the type_info \c T of halfedge property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_halfedge_property_type(const std::string& name) const
        {
            return hprops_.get_type(name);
        }
        /** get the type_info \c T of edge property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_edge_property_type(const std::string& name) const
        {
            return eprops_.get_type(name);
        }
        /** get the type_info \c T of face property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_face_property_type(const std::string& name) const
        {
            return fprops_.get_type(name);
        }
        /** get the type_info \c T of model property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_model_property_type(const std::string& name) const
        {
            return mprops_.get_type(name);
        }


        /// returns the names of all vertex properties
        std::vector<std::string> vertex_properties() const
        {
            return vprops_.properties();
        }
        /// returns the names of all halfedge properties
        std::vector<std::string> halfedge_properties() const
        {
            return hprops_.properties();
        }
        /// returns the names of all edge properties
        std::vector<std::string> edge_properties() const
        {
            return eprops_.properties();
        }
        /// returns the names of all face properties
        std::vector<std::string> face_properties() const
        {
            return fprops_.properties();
        }
        /// returns the names of all model properties
        std::vector<std::string> model_properties() const
        {
            return mprops_.properties();
        }

        /// prints the names of all properties to an output stream (e.g., std::cout).
        void property_stats(std::ostream &output) const override;

        //@}


    public: //--------------------------------------------- iterators & circulators

        /// \name Iterators & Circulators
        //@{

        /// returns start iterator for vertices
        VertexIterator vertices_begin() const
        {
            return VertexIterator(Vertex(0), this);
        }

        /// returns end iterator for vertices
        VertexIterator vertices_end() const
        {
            return VertexIterator(Vertex(static_cast<int>(vertices_size())), this);
        }

        /// returns vertex container for C++11 range-based for-loops
        VertexContainer vertices() const
        {
            return VertexContainer(vertices_begin(), vertices_end());
        }

        /// returns start iterator for halfedges
        HalfedgeIterator halfedges_begin() const
        {
            return HalfedgeIterator(Halfedge(0), this);
        }

        /// returns end iterator for halfedges
        HalfedgeIterator halfedges_end() const
        {
            return HalfedgeIterator(Halfedge(static_cast<int>(halfedges_size())), this);
        }

        /// returns halfedge container for C++11 range-based for-loops
        HalfedgeContainer halfedges() const
        {
            return HalfedgeContainer(halfedges_begin(), halfedges_end());
        }

        /// returns start iterator for edges
        EdgeIterator edges_begin() const
        {
            return EdgeIterator(Edge(0), this);
        }

        /// returns end iterator for edges
        EdgeIterator edges_end() const
        {
            return EdgeIterator(Edge(static_cast<int>(edges_size())), this);
        }

        /// returns edge container for C++11 range-based for-loops
        EdgeContainer edges() const
        {
            return EdgeContainer(edges_begin(), edges_end());
        }

        /// returns start iterator for faces
        FaceIterator faces_begin() const
        {
            return FaceIterator(Face(0), this);
        }

        /// returns end iterator for faces
        FaceIterator faces_end() const
        {
            return FaceIterator(Face(static_cast<int>(faces_size())), this);
        }

        /// returns face container for C++11 range-based for-loops
        FaceContainer faces() const
        {
            return FaceContainer(faces_begin(), faces_end());
        }

        /// returns circulator for vertices around vertex \c v
        VertexAroundVertexCirculator vertices(Vertex v) const
        {
            return VertexAroundVertexCirculator(this, v);
        }

        /// returns circulator for outgoing halfedges around vertex \c v
        HalfedgeAroundVertexCirculator halfedges(Vertex v) const
        {
            return HalfedgeAroundVertexCirculator(this, v);
        }

        /// returns circulator for faces around vertex \c v
        FaceAroundVertexCirculator faces(Vertex v) const
        {
            return FaceAroundVertexCirculator(this, v);
        }

        /// returns circulator for vertices of face \c f
        VertexAroundFaceCirculator vertices(Face f) const
        {
            return VertexAroundFaceCirculator(this, f);
        }

        /// returns circulator for halfedges of face \c f
        HalfedgeAroundFaceCirculator halfedges(Face f) const
        {
            return HalfedgeAroundFaceCirculator(this, f);
        }

        //@}


    public: //--------------------------------------------- higher-level operations

        /// \name Higher-level Topological Operations
        //@{

        /// returns whether the mesh closed (i.e., no boundary edges)
        bool is_closed() const;

        /// returns whether the mesh a triangle mesh. this function simply tests
        /// each face, and therefore is not very efficient.
        /// \sa triangulate(), triangulate(Face)
        bool is_triangle_mesh() const;

        /// returns whether the mesh a quad mesh. this function simply tests
        /// each face, and therefore is not very efficient.
        bool is_quad_mesh() const;

        /// triangulate the entire mesh, by calling triangulate(Face) for each face.
        /// \sa triangulate(Face)
        void triangulate();

        /// triangulate the face \c f
        /// \sa triangulate()
        void triangulate(Face f);

        /// \brief Reverses the orientation of the entire mesh.
        /// \details This function reverses for each face the order of the vertices along the face boundary.
        /// As a consequence, the normal computed for each face using compute_face_normal() is also reversed.
        void reverse_orientation();

        /// returns whether collapsing the halfedge \c h is topologically legal.
        /// \attention This function is only valid for triangle meshes.
        bool is_collapse_ok(Halfedge h) const;

        /** Collapse the halfedge \c h by moving its start vertex into its target
         vertex. For non-boundary halfedges this function removes one vertex, three
         edges, and two faces. For boundary halfedges it removes one vertex, two
         edges and one face.
         \attention This function is only valid for triangle meshes.
         \attention Halfedge collapses might lead to invalid faces. Call
         is_collapse_ok(Halfedge) to be sure the collapse is legal.
         \attention The removed items are only marked as deleted. You have
         to call collect_garbage() to finally remove them.
         */
        void collapse(Halfedge h);


        /** Split the face \c f by first adding point \c p to the mesh and then
         inserting edges between \c p and the vertices of \c f. For a triangle
         this is a standard one-to-three split.
         \sa split(Face, Vertex)
         */
        Vertex split(Face f, const vec3& p) { Vertex v=add_vertex(p); split(f,v); return v; }

        /** Split the face \c f by inserting edges between \c v and the vertices
         of \c f. For a triangle this is a standard one-to-three split.
         \sa split(Face, const vec3&)
         */
        void split(Face f, Vertex v);


        /** Split the edge \c e by first adding point \c p to the mesh and then
         connecting it to the two vertices of the adjacent triangles that are
         opposite to edge \c e. Returns the halfedge pointing to \c p that is
         created by splitting the existing edge \c e.

         \attention This function is only valid for triangle meshes.
         \sa split(Edge, Vertex)
         */
        Halfedge split(Edge e, const vec3& p) { return split(e, add_vertex(p)); }


        /** Split the edge \c e by connecting vertex \c v it to the two
         vertices of the adjacent triangles that are opposite to edge \c
         e. Returns the halfedge pointing to \c p that is created by splitting
         the existing edge \c e.

         \attention This function is only valid for triangle meshes.
         \sa split(Edge, vec3)
         */
        Halfedge split(Edge e, Vertex v);


        /** Subdivide the edge \c e = (v0,v1) by splitting it into the two edge
         (v0,p) and (p,v1). Note that this function does not introduce any
         other edge or faces. It simply splits the edge. Returns halfedge that
         points to \c p.
         \sa insert_vertex(Edge, Vertex)
         \sa insert_vertex(Halfedge, Vertex)
         \related join_edges(Vertex)
         */
        Halfedge insert_vertex(Edge e, const vec3& p)
        {
            return insert_vertex(halfedge(e,0), add_vertex(p));
        }

        /** Subdivide the edge \c e = (v0,v1) by splitting it into the two edge
         (v0,v) and (v,v1). Note that this function does not introduce any other
         edge or faces. It simply splits the edge. Returns halfedge that points to \c v.
         \sa insert_vertex(Edge, vec3)
         \sa insert_vertex(Halfedge, Vertex)
         \related join_edges(Vertex)
         */
        Halfedge insert_vertex(Edge e, Vertex v)
        {
            return insert_vertex(halfedge(e,0), v);
        }

        /** Subdivide the edge \c e = (v0,v1) by splitting it into the two edge
         (v0,v) and (v,v1). Note that this function does not introduce any other
         edge or faces. It simply splits the edge. Returns halfedge that points to \c v.
         \sa insert_vertex(Edge, vec3)
         \sa insert_vertex(Edge, Vertex)
         \related join_edges(Vertex)
         */
        Halfedge insert_vertex(Halfedge h, Vertex v);

        /**
         * Merges the two incident edges of a 2-degree vertex. This is the reverse operation of insert_vertex().
         * \pre valence(v) == 2.
         * \sa insert_vertex(Edge, vec3)
         * \sa insert_vertex(Edge, Vertex)
         * \sa insert_vertex(Halfedge, Vertex)
         */
        bool join_edges(Vertex v);
        /**
         * Check whether the two incident edges of a vertex can be joined. It only allows for vertices of valence two.
         * \sa join_edges(Vertex)
         */
        bool can_join_edges(Vertex v) const;


        /// insert edge between the to-vertices v0 of h0 and v1 of h1.
        /// returns the new halfedge from v0 to v1.
        /// \attention h0 and h1 have to belong to the same face
        Halfedge insert_edge(Halfedge h0, Halfedge h1);


        /**
         * Check whether flipping edge \c e is topologically allowed.
         \attention This function is only valid for triangle meshes.
         \sa flip(Edge)
         */
        bool is_flip_ok(Edge e) const;

        /**
         * Flip edge \c e: Remove edge \c e and add an edge between the two vertices opposite to edge \c e of the two
         * incident triangles.
         * \attention This function is only valid for triangle meshes. \sa is_flip_ok(Edge)
         */
        void flip(Edge e);


        /**
         * Check whether stitching two halfedges \c h0 an \c h1 is topologically allowed. Two halfedges can be stitched
         * if they are both on on the border and point in reverse directions.
         */
        bool is_stitch_ok(Halfedge h0, Halfedge h1);

        /**
         * @brief Stitch two halfedges h0 and h1. Precondition: h0 and h1 are both on the border and point in reversed
         *      directions.
         * @attention Stitching two halfedges changes the topology and geometry significantly, thus it may result in a
         *      non-manifold mesh, client code must check if this operation can be executed. @see is_stitch_ok().
         */
        void stitch(Halfedge h0, Halfedge h1);


        /** returns the valence (number of incident edges or neighboring vertices)
         of vertex \c v. */
        unsigned int valence(Vertex v) const;

        /// returns the valence of face \c f (its number of vertices)
        unsigned int valence(Face f) const;

        /// find the halfedge from start to end
        Halfedge find_halfedge(Vertex start, Vertex end) const;

        /// find the edge (a, b)
        Edge find_edge(Vertex a, Vertex b) const;

        /// deletes the vertex \c v from the mesh. Its incident edges and faces will also be deleted.
        /// \attention This function only marks the vertex and its incident edges and faces as deleted, and you have
        ///     to call collect_garbage() to finally remove them.
        void delete_vertex(Vertex v);

        /// deletes the edge \c e from the mesh. Its incident faces will also be deleted.
        /// \attention This function only marks the edge and its incident faces as deleted, and you have to call
        ///     collect_garbage() to finally remove them.
        void delete_edge(Edge e);

        /// deletes the face \c f from the mesh. Its incident edges (if on boundary) will also be deleted.
        /// \attention This function only marks the face and its incident edges (if on boundary) as deleted, and
        ///     you have to call collect_garbage() to finally remove them.
        void delete_face(Face f);

        //@}


    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /// position of a vertex (read only)
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /// position of a vertex
        vec3& position(Vertex v) { return vpoint_[v]; }

        /// vector of vertex positions (read only)
        const std::vector<vec3>& points() const override { return vpoint_.vector(); }

        /// vector of vertex positions
        std::vector<vec3>& points() override { return vpoint_.vector(); }

        /// compute face normals by calling compute_face_normal(Face) for each face.
        void update_face_normals();

        /// compute normal vector of face \c f.
        vec3 compute_face_normal(Face f) const;

        /// compute vertex normals by calling compute_vertex_normal(Vertex) for each vertex.
        void update_vertex_normals();

        /// compute normal vector of vertex \c v. This is the angle-weighted average of incident face normals.
        /// TODO: not stable for concave vertices or vertices with spanning angles close to 0 or 180 degrees.
        vec3 compute_vertex_normal(Vertex v) const;

        /// compute the length of edge \c e.
        float edge_length(Edge e) const;
        /// compute the length of an edge denoted by one of its halfedge \c h.
        float edge_length(Halfedge h) const;

        //@}


    private: //---------------------------------------------- allocate new elements

        /// allocate a new vertex, resize vertex properties accordingly.
        Vertex new_vertex()
        {
            vprops_.push_back();
            return Vertex(static_cast<int>(vertices_size()-1));
        }

        /// allocate a new edge, resize edge and halfedge properties accordingly.
        Halfedge new_edge(Vertex start, Vertex end)
        {
            assert(start != end);

            eprops_.push_back();
            hprops_.push_back();
            hprops_.push_back();

            Halfedge h0(static_cast<int>(halfedges_size()-2));
            Halfedge h1(static_cast<int>(halfedges_size()-1));

            set_target(h0, end);
            set_target(h1, start);

            return h0;
        }

        /// allocate a new face, resize face properties accordingly.
        Face new_face()
        {
            fprops_.push_back();
            return Face(static_cast<int>(faces_size()-1));
        }


    private: //--------------------------------------------------- helper functions

        /**
         * [Liangliang]:
         * The outgoing halfedges of the vertices may not be valid after a sequence calls to add_face() operations or
         * after deleting faces, because manifoldness is not maintained). This function assigns the correct outgoing
         * halfedge to each vertex.
         */
        void adjust_outgoing_halfedges();

        /** make sure that the outgoing halfedge of vertex v is a boundary halfedge
         if v is a boundary vertex. */
        void adjust_outgoing_halfedge(Vertex v);

        /// Helper for halfedge collapse
        void remove_edge(Halfedge h);

        /// Helper for halfedge collapse
        void remove_loop(Halfedge h);

        /// Helper for stitching edges. It checks whether the vertices pointed by h0 and h1 can be merged. It is called
        /// twice by is_stitch_ok(), once per orientation of the edges.
        bool can_merge_vertices(Halfedge h0, Halfedge h1) const;

    private: //------------------------------------------------------- private data

        PropertyContainer vprops_;
        PropertyContainer hprops_;
        PropertyContainer eprops_;
        PropertyContainer fprops_;
        PropertyContainer mprops_;

        VertexProperty<VertexConnectivity>      vconn_;
        HalfedgeProperty<HalfedgeConnectivity>  hconn_;
        FaceProperty<FaceConnectivity>          fconn_;

        VertexProperty<bool>  vdeleted_;
        EdgeProperty<bool>    edeleted_;
        FaceProperty<bool>    fdeleted_;

        VertexProperty<vec3>  vpoint_;
        VertexProperty<vec3>  vnormal_;
        FaceProperty<vec3>    fnormal_;

        unsigned int deleted_vertices_;
        unsigned int deleted_edges_;
        unsigned int deleted_faces_;
        bool garbage_;

        // helper data for add_face()
        typedef std::pair<Halfedge, Halfedge>  NextCacheEntry;
        typedef std::vector<NextCacheEntry>    NextCache;
        std::vector<Vertex>      add_face_vertices_;
        std::vector<Halfedge>    add_face_halfedges_;
        std::vector<bool>        add_face_is_new_;
        std::vector<bool>        add_face_needs_adjust_;
        NextCache                add_face_next_cache_;

		friend class SurfaceMeshBuilder;
    };


    //------------------------------------------------------------ output operators

    /// Output stream support for SurfaceMesh::Vertex.
    inline std::ostream& operator<<(std::ostream& os, SurfaceMesh::Vertex v)
    {
        return (os << 'v' << v.idx());
    }

    /// Output stream support for SurfaceMesh::Halfedge.
    inline std::ostream& operator<<(std::ostream& os, SurfaceMesh::Halfedge h)
    {
        return (os << 'h' << h.idx());
    }

    /// Output stream support for SurfaceMesh::Edge.
    inline std::ostream& operator<<(std::ostream& os, SurfaceMesh::Edge e)
    {
        return (os << 'e' << e.idx());
    }

    /// Output stream support for SurfaceMesh::Face.
    inline std::ostream& operator<<(std::ostream& os, SurfaceMesh::Face f)
    {
        return (os << 'f' << f.idx());
    }

} // namespace easy3d

#endif // EASY3D_CORE_SURFACE_MESH_H