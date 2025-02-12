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

#ifndef EASY3D_CORE_POLYHEDRAL_MESH_H
#define EASY3D_CORE_POLYHEDRAL_MESH_H

#include <easy3d/core/model.h>

#include <set>

#include <easy3d/core/types.h>
#include <easy3d/core/property.h>


namespace easy3d {

    /**
     * \brief Data structure representing a polyhedral mesh.
     * \class PolyMesh easy3d/core/poly_mesh.h
     * \note PolyMesh assumes the half-face normals pointing outside the cells.
     *
     * This implementation is inspired by Surface\_mesh
     * https://opensource.cit-ec.de/projects/surface\_mesh
     */
    class PolyMesh : public Model
    {

    public: //------------------------------------------------------ topology types

        /**
         * \brief Base class for all topology types (internally it is basically an index).
         * \sa Vertex, Edge, HalfFace, Face, Cell
         */
        class BaseHandle
        {
        public:
            /**
             * \brief Constructor
             * \param _idx The index of the handle.
             */
            explicit BaseHandle(int _idx=-1) : idx_(_idx) {}

            /**
             * \brief Return the index of the handle.
             * \return The index of the handle.
             */
            int idx() const { return idx_; }

            /**
             * \brief Reset handle to be invalid (index = -1).
             */
            void reset() { idx_=-1; }

            /**
             * \brief Return whether the handle is valid, i.e., the index is not equal to -1.
             * \return True if the handle is valid, false otherwise.
             */
            bool is_valid() const { return idx_ != -1; }

            /**
             * \brief Are two handles equal?
             * \param _rhs The other handle to compare.
             * \return True if the handles are equal, false otherwise.
             */
            bool operator==(const BaseHandle& _rhs) const {
                return idx_ == _rhs.idx_;
            }

            /**
             * \brief Are two handles different?
             * \param _rhs The other handle to compare.
             * \return True if the handles are different, false otherwise.
             */
            bool operator!=(const BaseHandle& _rhs) const {
                return idx_ != _rhs.idx_;
            }

            /**
             * \brief Compare operator useful for sorting handles.
             * \param _rhs The other handle to compare.
             * \return True if this handle is less than the other handle, false otherwise.
             */
            bool operator<(const BaseHandle& _rhs) const {
                return idx_ < _rhs.idx_;
            }

            /**
             * \brief Helper structure to be able to use std::unordered_map.
             */
            struct Hash {
                /**
                 * \brief Hash function for BaseHandle.
                 * \param h The handle to hash.
                 * \return The hash value, which is the index.
                 */
                std::size_t operator()(const BaseHandle& h) const { return h.idx(); }
            };

        private:
            friend class PolyMesh;
            int idx_;
        };

        /**
         * This type represents a vertex (internally it is basically an index).
         * \sa Edge, HalfFace, Face, Cell
         */
        struct Vertex : BaseHandle {
            /**
             * \brief Default constructor (with invalid index).
             * \param _idx The index of the vertex.
             */
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            /**
             * \brief Output stream support for Vertex.
             * \param os The output stream.
             * \return The output stream with the vertex index.
             */
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };

        /**
         * This type represents an edge (internally it is basically an index).
         * \sa Vertex, HalfFace, Face, Cell
         */
        struct Edge : BaseHandle {
            /**
             * \brief Default constructor (with invalid index).
             * \param _idx The index of the edge.
             */
            explicit Edge(int _idx=-1) : BaseHandle(_idx) {}
            /**
             * \brief Output stream support for Edge.
             * \param os The output stream.
             * \return The output stream with the edge index.
             */
            std::ostream& operator<<(std::ostream& os) const { return os << 'e' << idx(); }
        };

        /**
         * This type represents a halfface (internally it is basically an index).
         * \sa Vertex, Edge, Face, Cell
         */
        struct HalfFace : BaseHandle {
            /**
             * \brief Default constructor (with invalid index).
             * \param _idx The index of the halfface.
             */
            explicit HalfFace(int _idx=-1) : BaseHandle(_idx) {}
            /**
             * \brief Output stream support for HalfFace.
             * \param os The output stream.
             * \return The output stream with the halfface index.
             */
            std::ostream& operator<<(std::ostream& os) const { return os << 'h' << idx(); }
        };

        /**
         * This type represents a face (internally it is basically an index).
         * \sa Vertex, Edge, HalfFace, Cell
         */
        struct Face : BaseHandle {
            /**
             * \brief Default constructor (with invalid index).
             * \param _idx The index of the face.
             */
            explicit Face(int _idx=-1) : BaseHandle(_idx) {}
            /**
             * \brief Output stream support for Face.
             * \param os The output stream.
             * \return The output stream with the face index.
             */
            std::ostream& operator<<(std::ostream& os) const { return os << 'f' << idx(); }
        };

        /**
         * This type represents a polyhedral cell (internally it is basically an index).
         * \sa Vertex, Edge, HalfFace, Face
         */
        struct Cell : BaseHandle {
            /**
             * \brief Default constructor (with invalid index).
             * \param _idx The index of the cell.
             */
            explicit Cell(int _idx=-1) : BaseHandle(_idx) {}
            /**
             * \brief Output stream support for Cell.
             * \param os The output stream.
             * \return The output stream with the cell index.
             */
            std::ostream& operator<<(std::ostream& os) const { return os << 'c' << idx(); }
        };


    public: //-------------------------------------------------- connectivity types

        /**
         * This type stores the vertex connectivity
         * \sa EdgeConnectivity, HalfFaceConnectivity, CellConnectivity
         */
        struct VertexConnectivity {
            std::set<Vertex>     vertices_;  ///< The vertices connected to this vertex.
            std::set<Edge>       edges_;     ///< The edges incident to this vertex.
            std::set<HalfFace>   halffaces_; ///< The halffaces incident to this vertex.
            std::set<Cell>       cells_;     ///< The cells incident to this vertex.
            /**
             * This type stores the vertex connectivity
             * \sa EdgeConnectivity, HalfFaceConnectivity, CellConnectivity
             */
            void read(std::istream& in);
            /**
             * \brief Writes the edge connectivity to an output stream.
             * \param out The output stream.
             */
            void write(std::ostream& out) const;
        };

        /**
         * This type stores the edge connectivity
         * \sa VertexConnectivity, HalfFaceConnectivity, CellConnectivity
         */
        struct EdgeConnectivity {
            std::vector<Vertex>  vertices_;  ///< The vertices connected to this edge.
            std::set<HalfFace>   halffaces_; ///< The halffaces incident to this edge.
            std::set<Cell>       cells_;     ///< The cells incident to this edge.
            /**
             * \brief Reads the edge connectivity from an input stream.
             * \param in The input stream.
             */
            void read(std::istream& in);
            /**
             * \brief Writes the edge connectivity to an output stream.
             * \param out The output stream.
             */
            void write(std::ostream& out) const;
        };

        /**
         * This type stores the halfface connectivity
         * \sa VertexConnectivity, EdgeConnectivity, CellConnectivity
         */
        struct HalfFaceConnectivity {
            std::vector<Vertex> vertices_; ///< The vertices connected to this halfface.
            std::set<Edge>      edges_;    ///< The edges incident to this halfface.
            Cell                cell_;     ///< The cell incident to this halfface.
            HalfFace            opposite_; ///< The opposite halfface.
            /**
             * \brief Reads the halfface connectivity from an input stream.
             * \param in The input stream.
             */
            void read(std::istream& in);
            /**
             * \brief Writes the halfface connectivity to an output stream.
             * \param out The output stream.
             */
            void write(std::ostream& out) const;
        };

        /**
         * This type stores the cell connectivity
         * \sa VertexConnectivity, EdgeConnectivity, HalfFaceConnectivity
         */
        struct CellConnectivity {
            std::set<Vertex>        vertices_;    ///< The vertices connected to this cell.
            std::set<Edge>          edges_;       ///< The edges incident to this cell.
            std::vector<HalfFace>   halffaces_;   ///< The halffaces incident to this cell.
            /**
             * \brief Reads the cell connectivity from an input stream.
             * \param in The input stream.
             */
            void read(std::istream& in);
            /**
             * \brief Writes the cell connectivity to an output stream.
             * \param out The output stream.
             */
            void write(std::ostream& out) const;
        };


    public: //------------------------------------------------------ property types

        /**
         * \brief Vertex property of type T
         * \sa EdgeProperty, HalfFaceProperty, FaceProperty, CellProperty
         */
        template <class T> class VertexProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor
             */
            VertexProperty() = default;
            /**
             * \brief Constructor with property
             * \param p The property to initialize with
             */
            explicit VertexProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for vertex \c v
             * \param v The vertex to access
             * \return Reference to the data
             */
            typename Property<T>::reference operator[](Vertex v) {
                return Property<T>::operator[](v.idx());
            }

            /**
             * \brief Access the data stored for vertex \c v
             * \param v The vertex to access
             * \return Const reference to the data
             */
            typename Property<T>::const_reference operator[](Vertex v) const {
                return Property<T>::operator[](v.idx());
            }
        };

        /**
         * \brief Edge property of type T
         * \sa VertexProperty, HalfFaceProperty, FaceProperty, CellProperty
         */
        template <class T> class EdgeProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor
             */
            EdgeProperty() = default;
            /**
             * \brief Constructor with property
             * \param p The property to initialize with
             */
            explicit EdgeProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for edge \c e
             * \param e The edge to access
             * \return Reference to the data
             */
            typename Property<T>::reference operator[](Edge e) {
                return Property<T>::operator[](e.idx());
            }

            /**
             * \brief Access the data stored for edge \c e
             * \param e The edge to access
             * \return Const reference to the data
             */
            typename Property<T>::const_reference operator[](Edge e) const {
                return Property<T>::operator[](e.idx());
            }
        };

        /**
         * \brief HalfFace property of type T
         * \sa VertexProperty, EdgeProperty, FaceProperty, CellProperty
         */
        template <class T> class HalfFaceProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor
             */
            HalfFaceProperty() = default;
            /**
             * \brief Constructor with property
             * \param p The property to initialize with
             */
            explicit HalfFaceProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for halfface \c h
             * \param h The halfface to access
             * \return Reference to the data
             */
            typename Property<T>::reference operator[](HalfFace h) {
                return Property<T>::operator[](h.idx());
            }

            /**
             * \brief Access the data stored for halfface \c h
             * \param h The halfface to access
             * \return Const reference to the data
             */
            typename Property<T>::const_reference operator[](HalfFace h) const {
                return Property<T>::operator[](h.idx());
            }
        };

        /**
         * \brief Face property of type T
         * \sa VertexProperty, EdgeProperty, HalfFaceProperty, CellProperty
         */
        template <class T> class FaceProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor
             */
            FaceProperty() = default;
            /**
             * \brief Constructor with property
             * \param p The property to initialize with
             */
            explicit FaceProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for face \c f
             * \param f The face to access
             * \return Reference to the data
             */
            typename Property<T>::reference operator[](Face f) {
                return Property<T>::operator[](f.idx());
            }

            /**
             * \brief Access the data stored for face \c f
             * \param f The face to access
             * \return Const reference to the data
             */
            typename Property<T>::const_reference operator[](Face f) const {
                return Property<T>::operator[](f.idx());
            }
        };

        /**
         * \brief Cell property of type T
         * \sa VertexProperty, EdgeProperty, HalfFaceProperty, FaceProperty
         */
        template <class T> class CellProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor
             */
            CellProperty() = default;
            /**
             * \brief Constructor with property
             * \param p The property to initialize with
             */
            explicit CellProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for cell \c c
             * \param c The cell to access
             * \return Reference to the data
             */
            typename Property<T>::reference operator[](Cell c) {
                return Property<T>::operator[](c.idx());
            }

            /**
             * \brief Access the data stored for cell \c c
             * \param c The cell to access
             * \return Const reference to the data
             */
            typename Property<T>::const_reference operator[](Cell c) const {
                return Property<T>::operator[](c.idx());
            }
        };


        /**
         * \brief Model property of type T
         * \sa VertexProperty, EdgeProperty, HalfFaceProperty, FaceProperty, CellProperty
         */
        template <class T> class ModelProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor
             */
            ModelProperty() = default;
            /**
             * \brief Constructor with property
             * \param p The property to initialize with
             */
            explicit ModelProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for the mesh
             * \param idx The index to access
             * \return Reference to the data
             */
            typename Property<T>::reference operator[](size_t idx) {
                return Property<T>::operator[](idx);
            }

            /**
             * \brief Access the data stored for the mesh
             * \param idx The index to access
             * \return Const reference to the data
             */
            typename Property<T>::const_reference operator[](size_t idx) const {
                return Property<T>::operator[](idx);
            }
        };


    public: //------------------------------------------------------ iterator types
        /**
         * \brief Vertex iterator for PolyMesh. This class iterates linearly over all vertices.
         * \sa vertices_begin(), vertices_end()
         * \sa EdgeIterator, HalfFaceIterator, FaceIterator, CellIterator
         */
        class VertexIterator
        {
        public:
            /**
             * \brief Default constructor
             * \param v The vertex to start the iterator
             * \param m The PolyMesh to iterate over
             */
            explicit VertexIterator(Vertex v=Vertex(), const PolyMesh* m=nullptr) : hnd_(v), mesh_(m) { }

            /**
             * \brief Get the vertex the iterator refers to
             * \return The current vertex
             */
            Vertex operator*()  const { return  hnd_; }

            /**
             * \brief Are two iterators equal?
             * \param rhs The other iterator to compare
             * \return True if the iterators are equal, false otherwise
             */
            bool operator==(const VertexIterator& rhs) const {
                return (hnd_==rhs.hnd_);
            }

            /**
             * \brief Are two iterators different?
             * \param rhs The other iterator to compare
             * \return True if the iterators are different, false otherwise
             */
            bool operator!=(const VertexIterator& rhs) const {
                return !operator==(rhs);
            }

            /**
             * \brief Pre-increment iterator
             * \return Reference to the incremented iterator
             */
            VertexIterator& operator++() {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /**
             * \brief Pre-decrement iterator
             * \return Reference to the decremented iterator
             */
            VertexIterator& operator--() {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Vertex  hnd_;
            const PolyMesh* mesh_;
        };

        /**
         * \brief Edge iterator for PolyMesh. This class iterates linearly over all edges.
         * \sa edges_begin(), edges_end()
         * \sa VertexIterator, HalfFaceIterator, FaceIterator, CellIterator
         */
        class EdgeIterator
        {
        public:
            /**
             * \brief Default constructor
             * \param e The edge to start the iterator
             * \param m The PolyMesh to iterate over
             */
            explicit EdgeIterator(Edge e=Edge(), const PolyMesh* m=nullptr) : hnd_(e), mesh_(m) { }

            /**
             * \brief Get the edge the iterator refers to
             * \return The current edge
             */
            Edge operator*()  const { return  hnd_; }

            /**
             * \brief Are two iterators equal?
             * \param rhs The other iterator to compare
             * \return True if the iterators are equal, false otherwise
             */
            bool operator==(const EdgeIterator& rhs) const {
                return (hnd_==rhs.hnd_);
            }

            /**
             * \brief Are two iterators different?
             * \param rhs The other iterator to compare
             * \return True if the iterators are different, false otherwise
             */
            bool operator!=(const EdgeIterator& rhs) const {
                return !operator==(rhs);
            }

            /**
             * \brief Pre-increment iterator
             * \return Reference to the incremented iterator
             */
            EdgeIterator& operator++() {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /**
             * \brief Pre-decrement iterator
             * \return Reference to the decremented iterator
             */
            EdgeIterator& operator--() {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Edge  hnd_;
            const PolyMesh* mesh_;
        };


        /**
         * \brief HalfFace iterator for PolyMesh. This class iterates linearly over all halffaces.
         * \sa halffaces_begin(), halffaces_end()
         * \sa VertexIterator, EdgeIterator, FaceIterator, CellIterator
         */
        class HalfFaceIterator
        {
        public:
            /**
             * \brief Default constructor
             * \param h The halfface to start the iterator
             * \param m The PolyMesh to iterate over
             */
            explicit HalfFaceIterator(HalfFace h=HalfFace(), const PolyMesh* m=nullptr) : hnd_(h), mesh_(m) { }

            /**
             * \brief Get the halfface the iterator refers to
             * \return The current halfface
             */
            HalfFace operator*()  const { return  hnd_; }

            /**
             * \brief Are two iterators equal?
             * \param rhs The other iterator to compare
             * \return True if the iterators are equal, false otherwise
             */
            bool operator==(const HalfFaceIterator& rhs) const {
                return (hnd_==rhs.hnd_);
            }

            /**
             * \brief Are two iterators different?
             * \param rhs The other iterator to compare
             * \return True if the iterators are different, false otherwise
             */
            bool operator!=(const HalfFaceIterator& rhs) const {
                return !operator==(rhs);
            }

            /**
             * \brief Pre-increment iterator
             * \return Reference to the incremented iterator
             */
            HalfFaceIterator& operator++() {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /**
             * \brief Pre-decrement iterator
             * \return Reference to the decremented iterator
             */
            HalfFaceIterator& operator--() {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            HalfFace  hnd_;
            const PolyMesh* mesh_;
        };


        /**
         * \brief Face iterator for PolyMesh. This class iterates linearly over all faces.
         * \sa faces_begin(), faces_end()
         * \sa VertexIterator, EdgeIterator, HalfFaceIterator, CellIterator
         */
        class FaceIterator
        {
        public:
            /**
             * \brief Default constructor
             * \param f The face to start the iterator
             * \param m The PolyMesh to iterate over
             */
            explicit FaceIterator(Face f=Face(), const PolyMesh* m=nullptr) : hnd_(f), mesh_(m) { }

            /**
             * \brief Get the face the iterator refers to
             * \return The current face
             */
            Face operator*()  const { return  hnd_; }

            /**
             * \brief Are two iterators equal?
             * \param rhs The other iterator to compare
             * \return True if the iterators are equal, false otherwise
             */
            bool operator==(const FaceIterator& rhs) const {
                return (hnd_==rhs.hnd_);
            }

            /**
             * \brief Are two iterators different?
             * \param rhs The other iterator to compare
             * \return True if the iterators are different, false otherwise
             */
            bool operator!=(const FaceIterator& rhs) const {
                return !operator==(rhs);
            }

            /**
             * \brief Pre-increment iterator
             * \return Reference to the incremented iterator
             */
            FaceIterator& operator++() {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /**
             * \brief Pre-decrement iterator
             * \return Reference to the decremented iterator
             */
            FaceIterator& operator--() {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Face  hnd_;
            const PolyMesh* mesh_;
        };


        /**
         * \brief Cell iterator for PolyMesh. This class iterates linearly over all cells.
         * \sa cells_begin(), cells_end()
         * \sa VertexIterator, EdgeIterator, HalfFaceIterator, FaceIterator
         */
        class CellIterator
        {
        public:
            /**
             * \brief Default constructor
             * \param c The cell to start the iterator
             * \param m The PolyMesh to iterate over
             */
            explicit CellIterator(Cell c=Cell(), const PolyMesh* m=nullptr) : hnd_(c), mesh_(m) { }

            /**
             * \brief Get the cell the iterator refers to
             * \return The current cell
             */
            Cell operator*() const { return  hnd_; }

            /**
             * \brief Are two iterators equal?
             * \param rhs The other iterator to compare
             * \return True if the iterators are equal, false otherwise
             */
            bool operator==(const CellIterator& rhs) const {
                return (hnd_==rhs.hnd_);
            }

            /**
             * \brief Are two iterators different?
             * \param rhs The other iterator to compare
             * \return True if the iterators are different, false otherwise
             */
            bool operator!=(const CellIterator& rhs) const {
                return !operator==(rhs);
            }

            /**
             * \brief Pre-increment iterator
             * \return Reference to the incremented iterator
             */
            CellIterator& operator++() {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /**
             * \brief Pre-decrement iterator
             * \return Reference to the decremented iterator
             */
            CellIterator& operator--() {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Cell  hnd_;
            const PolyMesh* mesh_;
        };


    public: //-------------------------- containers for C++11 range-based for loops

        /**
         * \brief This helper class is a container for iterating through all vertices using C++11 range-based for-loops.
         * \sa vertices()
         */
        class VertexContainer
        {
        public:
            /**
             * \brief Constructor
             * \param _begin The beginning iterator
             * \param _end The ending iterator
             */
            VertexContainer(VertexIterator _begin, VertexIterator _end) : begin_(_begin), end_(_end) {}
            /**
             * \brief Get the beginning iterator
             * \return The beginning iterator
             */
            VertexIterator begin() const { return begin_; }
            /**
             * \brief Get the ending iterator
             * \return The ending iterator
             */
            VertexIterator end()   const { return end_;   }
        private:
            VertexIterator begin_, end_;
        };


        /**
         * \brief This helper class is a container for iterating through all edges using C++11 range-based for-loops.
         * \sa edges()
         */
        class EdgeContainer
        {
        public:
            /**
             * \brief Constructor
             * \param _begin The beginning iterator
             * \param _end The ending iterator
             */
            EdgeContainer(EdgeIterator _begin, EdgeIterator _end) : begin_(_begin), end_(_end) {}
            /**
             * \brief Get the beginning iterator
             * \return The beginning iterator
             */
            EdgeIterator begin() const { return begin_; }
            /**
             * \brief Get the ending iterator
             * \return The ending iterator
             */
            EdgeIterator end()   const { return end_;   }
        private:
            EdgeIterator begin_, end_;
        };


        /**
         * \brief This helper class is a container for iterating through all halffaces using C++11 range-based for-loops.
         * \sa halffaces()
         */
        class HalffaceContainer
        {
        public:
            /**
             * \brief Constructor
             * \param _begin The beginning iterator
             * \param _end The ending iterator
             */
            HalffaceContainer(HalfFaceIterator _begin, HalfFaceIterator _end) : begin_(_begin), end_(_end) {}
            /**
             * \brief Get the beginning iterator
             * \return The beginning iterator
             */
            HalfFaceIterator begin() const { return begin_; }
            /**
             * \brief Get the ending iterator
             * \return The ending iterator
             */
            HalfFaceIterator end()   const { return end_;   }
        private:
            HalfFaceIterator begin_, end_;
        };


        /**
         * \brief This helper class is a container for iterating through all faces using C++11 range-based for-loops.
         * \sa faces()
         */
        class FaceContainer
        {
        public:
            /**
             * \brief Constructor
             * \param _begin The beginning iterator
             * \param _end The ending iterator
             */
            FaceContainer(FaceIterator _begin, FaceIterator _end) : begin_(_begin), end_(_end) {}
            /**
             * \brief Get the beginning iterator
             * \return The beginning iterator
             */
            FaceIterator begin() const { return begin_; }
            /**
             * \brief Get the ending iterator
             * \return The ending iterator
             */
            FaceIterator end()   const { return end_;   }
        private:
            FaceIterator begin_, end_;
        };

        
        /**
         * \brief This helper class is a container for iterating through all cells using C++11 range-based for-loops.
         * \sa cells()
         */
        class CellContainer
        {
        public:
            /**
             * \brief Constructor
             * \param _begin The beginning iterator
             * \param _end The ending iterator
             */
            CellContainer(CellIterator _begin, CellIterator _end) : begin_(_begin), end_(_end) {}
            /**
             * \brief Get the beginning iterator
             * \return The beginning iterator
             */
            CellIterator begin() const { return begin_; }
            /**
             * \brief Get the ending iterator
             * \return The ending iterator
             */
            CellIterator end()   const { return end_;   }
        private:
            CellIterator begin_, end_;
        };


    public: //-------------------------------------------- constructor / destructor

        /// \name Construct, destruct, assignment
        //@{

        /**
         * \brief Default constructor
         */
        PolyMesh();

        /**
         * \brief Destructor
         */
        ~PolyMesh() override = default;

        /**
         * \brief Copy constructor: copies \c rhs to \c *this. Performs a deep copy of all properties.
         * \param rhs The PolyMesh to copy from.
         */
        PolyMesh(const PolyMesh& rhs) { operator=(rhs); }

        /**
         * \brief Assign \c rhs to \c *this. Performs a deep copy of all properties.
         * \param rhs The PolyMesh to assign from.
         * \return Reference to the assigned PolyMesh.
         */
        PolyMesh& operator=(const PolyMesh& rhs);

        /**
         * \brief Assign \c rhs to \c *this. Does not copy custom properties.
         * \param rhs The PolyMesh to assign from.
         * \return Reference to the assigned PolyMesh.
         */
        PolyMesh& assign(const PolyMesh& rhs);
        //@}

        //! \name File IO
        //!@{

        /**
         * \brief Read mesh from a PM file \p filename.
         * \details Mainly for quick debug purposes. Client code should use PolyMeshIO.
         * \param filename The name of the file to read from.
         * \return True if the read was successful, false otherwise.
         * \sa PolyMeshIO.
         */
        bool read(const std::string& filename);
        /**
         * \brief Write mesh to a PM file \p filename.
         * \details Mainly for quick debug purposes. Client code should use PolyMeshIO.
         * \param filename The name of the file to write to.
         * \return True if the write was successful, false otherwise.
         * \sa PolyMeshIO.
         */
        bool write(const std::string& filename) const;
        //@}

    public: //----------------------------------------------- add new vertex / face / cell

        /// \name Add new elements by hand
        //@{

        /**
         * \brief Add a new vertex
         * \param p The position of the new vertex
         * \return The new vertex
         */
        Vertex add_vertex(const vec3& p) { Vertex v = new_vertex(); vpoint_[v] = p; return v; }
        /**
         * \brief Add a new cell defined by \c faces.
         * \param faces The input faces created by add_face(), add_triangle(), or add_quad().
         * \return The new cell.
         * \sa add_face(), add_triangle(), add_quad(), add_tetra(), add_hexa()
         */
        Cell add_cell(const std::vector<HalfFace>& faces);
        /**
         * \brief Add a new tetrahedron defined by its faces.
         * \param f0 The first face created by add_face() or add_triangle().
         * \param f1 The second face created by add_face() or add_triangle().
         * \param f2 The third face created by add_face() or add_triangle().
         * \param f3 The fourth face created by add_face() or add_triangle().
         * \return The new cell.
         * \sa add_cell(), add_tetra(), add_face(), add_triangle().
         */
        Cell add_tetra(HalfFace f0, HalfFace f1, HalfFace f2, HalfFace f3) { return add_cell({f0, f1, f2, f3}); }
        /**
         * \brief Add a new tetrahedron connecting vertices \c v0, \c v1, \c v2, \c v3.
         * \param v0 The first vertex created by add_vertex().
         * \param v1 The second vertex created by add_vertex().
         * \param v2 The third vertex created by add_vertex().
         * \param v3 The fourth vertex created by add_vertex().
         * \return The new cell.
         * \details It creates all the faces and the cell, and adds them to the mesh.
         * \sa add_vertex()
         */
        Cell add_tetra(Vertex v0, Vertex v1, Vertex v2, Vertex v3);
        /**
         * \brief Add a new hexahedron connecting vertices \c v0, \c v1, \c v2, \c v3, \c v4, \c v5, \c v6, \c v7.
         * \param v0 The first vertex created by add_vertex().
         * \param v1 The second vertex created by add_vertex().
         * \param v2 The third vertex created by add_vertex().
         * \param v3 The fourth vertex created by add_vertex().
         * \param v4 The fifth vertex created by add_vertex().
         * \param v5 The sixth vertex created by add_vertex().
         * \param v6 The seventh vertex created by add_vertex().
         * \param v7 The eighth vertex created by add_vertex().
         * \return The new cell.
         * \details This function creates all the faces and the cell, and adds them to the mesh. The vertices must be
         *         ordered as below:
         *             3--------------------2
         *             |\                   |\
         *             | \                  | \
         *             |  \                 |  \
         *             |   7----------------+---6
         *             |   |                |   |
         *             0---+----------------1   |
         *              \  |                 \  |
         *               \ |                  \ |
         *                \|                   \|
         *                 4--------------------5
         * \sa add_vertex()
         */
        Cell add_hexa(Vertex v0, Vertex v1, Vertex v2, Vertex v3, Vertex v4, Vertex v5, Vertex v6, Vertex v7);
        /**
         * \brief Add a new face connecting \c vertices.
         * \param vertices The input vertices created by add_vertex().
         * \return The new halfface.
         * \sa add_triangle(), add_quad()
         */
        HalfFace add_face(const std::vector<Vertex>& vertices);
        /**
         * \brief Add a new triangle face connecting vertices \c v0, \c v1, \c v2.
         * \param v0 The first vertex created by add_vertex().
         * \param v1 The second vertex created by add_vertex().
         * \param v2 The third vertex created by add_vertex().
         * \return The new halfface.
         * \sa add_face(), add_quad()
         */
        HalfFace add_triangle(Vertex v0, Vertex v1, Vertex v2) { return add_face({ v0, v1, v2 }); }
        /**
         * \brief Add a new quad face connecting vertices \c v0, \c v1, \c v2, \c v3.
         * \param v0 The first vertex created by add_vertex().
         * \param v1 The second vertex created by add_vertex().
         * \param v2 The third vertex created by add_vertex().
         * \param v3 The fourth vertex created by add_vertex().
         * \return The new halfface.
         * \sa add_face(), add_triangle()
         */
        HalfFace add_quad(Vertex v0, Vertex v1, Vertex v2, Vertex v3) { return add_face({ v0, v1, v2, v3}); }

        //@}

    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /**
         * \brief Returns number of vertices in the mesh.
         * \return The number of vertices.
         */
        unsigned int n_vertices() const { return (unsigned int) vprops_.size(); }
        /**
         * \brief Returns number of edges in the mesh.
         * \return The number of edges.
         */
        unsigned int n_edges() const { return (unsigned int) eprops_.size(); }
        /**
         * \brief Returns number of halffaces in the mesh.
         * \return The number of halffaces.
         */
        unsigned int n_halffaces() const { return (unsigned int) hprops_.size(); }
        /**
         * \brief Returns number of faces in the mesh.
         * \return The number of faces.
         */
        unsigned int n_faces() const { return (unsigned int) fprops_.size(); }
        /**
         * \brief Returns number of cells in the mesh.
         * \return The number of cells.
         */
        unsigned int n_cells() const { return (unsigned int) cprops_.size(); }

        /**
         * \brief Removes all vertices, edges, halffaces, faces, cells and properties.
         * \details After calling this method, the mesh is the same as newly constructed. The additional properties
         *         (such as normal vectors) are also removed and must thus be re-added if needed.
         */
        void clear();

        /**
         * \brief Resizes space for vertices, edges, halffaces, and their currently associated properties.
         * \param nv Number of vertices.
         * \param ne Number of edges.
         * \param nf Number of faces.
         * \param nc Number of cells.
         * \details nf is the number of faces. For halffaces, nh = 2 * nf.
         */
        void resize(unsigned int nv, unsigned int ne, unsigned int nf, unsigned int nc) {
            vprops_.resize(nv);
            eprops_.resize(ne);
            hprops_.resize(2 * nf);
            fprops_.resize(nf);
            cprops_.resize(nc);
        }

        /**
         * \brief Return whether vertex \c v is valid, i.e. the index is stored within the array bounds.
         * \param v The vertex to check.
         * \return True if the vertex is valid, false otherwise.
         */
        bool is_valid(Vertex v) const {
            return (0 <= v.idx()) && (v.idx() < (int)n_vertices());
        }
        /**
         * \brief Return whether edge \c e is valid, i.e. the index is stored within the array bounds.
         * \param e The edge to check.
         * \return True if the edge is valid, false otherwise.
         */
        bool is_valid(Edge e) const {
            return (0 <= e.idx()) && (e.idx() < (int)n_edges());
        }
        /**
         * \brief Return whether halfface \c h is valid, i.e. the index is stored within the array bounds.
         * \param h The halfface to check.
         * \return True if the halfface is valid, false otherwise.
         */
        bool is_valid(HalfFace h) const {
            return (0 <= h.idx()) && (h.idx() < (int)n_halffaces());
        }
        /**
         * \brief Return whether face \c f is valid, i.e. the index is stored within the array bounds.
         * \param f The face to check.
         * \return True if the face is valid, false otherwise.
         */
        bool is_valid(Face f) const {
            return (0 <= f.idx()) && (f.idx() < (int)n_faces());
        }
        /**
         * \brief Return whether cell \c c is valid, i.e. the index is stored within the array bounds.
         * \param c The cell to check.
         * \return True if the cell is valid, false otherwise.
         */
        bool is_valid(Cell c) const {
            return (0 <= c.idx()) && (c.idx() < (int)n_cells());
        }

        //@}
        

    public: //--------------------------------------------------- property handling

        /// \name Property handling
        //@{

        /**
         * \brief Add a vertex property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The new vertex property.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *         In this case, it returns an invalid property.
         */
        template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t=T()) {
            return VertexProperty<T>(vprops_.add<T>(name, t));
        }
        /**
         * \brief Add an edge property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The new edge property.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *         In this case, it returns an invalid property.
         */
        template <class T> EdgeProperty<T> add_edge_property(const std::string& name, const T t=T()) {
            return EdgeProperty<T>(eprops_.add<T>(name, t));
        }
        /**
         * \brief Add a halfface property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The new halfface property.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *         In this case, it returns an invalid property.
         */
        template <class T> HalfFaceProperty<T> add_halfface_property(const std::string& name, const T t=T()) {
            return HalfFaceProperty<T>(hprops_.add<T>(name, t));
        }
        /**
         * \brief Add a face property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The new face property.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *         In this case, it returns an invalid property.
         */
        template <class T> FaceProperty<T> add_face_property(const std::string& name, const T t=T()) {
            return FaceProperty<T>(fprops_.add<T>(name, t));
        }
        /**
         * \brief Add a cell property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The new cell property.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *         In this case, it returns an invalid property.
         */
        template <class T> CellProperty<T> add_cell_property(const std::string& name, const T t=T()) {
            return CellProperty<T>(cprops_.add<T>(name, t));
        }
        /**
         * \brief Adds a model property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \details Fails if a property named \c name exists already, since the name has to be unique.
         *      In this case it returns an invalid property.
         * Example:
         *      \code
         *          auto trans = cloud->add_model_property<mat4>("transformation", mat4::identity());
         *          trans[0] = mat4::translation(-x0, -y0, -z0);
         *      \endcode
         */
        template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t=T()) {
            return ModelProperty<T>(mprops_.add<T>(name, t));
        }

        /**
         * \brief Get the vertex property with name \c name of type \c T.
         * \param name The name of the property to get.
         * \return The vertex property. An invalid VertexProperty will be returned if the property does not exist or
         *         if the type does not match.
         */
        template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const {
            return VertexProperty<T>(vprops_.get<T>(name));
        }
        /**
         * \brief Get the edge property with name \c name of type \c T.
         * \param name The name of the property to get.
         * \return The edge property. An invalid EdgeProperty will be returned if the property does not exist or
         *         if the type does not match.
         */
        template <class T> EdgeProperty<T> get_edge_property(const std::string& name) const {
            return EdgeProperty<T>(eprops_.get<T>(name));
        }
        /**
         * \brief Get the halfface property with name \c name of type \c T.
         * \param name The name of the property to get.
         * \return The halfface property. An invalid HalfFaceProperty will be returned if the property does not exist or
         *         if the type does not match.
         */
        template <class T> HalfFaceProperty<T> get_halfface_property(const std::string& name) const {
            return HalfFaceProperty<T>(hprops_.get<T>(name));
        }
        /**
         * \brief Get the face property with name \c name of type \c T.
         * \param name The name of the property to get.
         * \return The face property. An invalid FaceProperty will be returned if the property does not exist or if the
         *        type does not match.
         */
        template <class T> FaceProperty<T> get_face_property(const std::string& name) const {
            return FaceProperty<T>(fprops_.get<T>(name));
        }
        /**
         * \brief Get the cell property with name \c name of type \c T.
         * \param name The name of the property to get.
         * \return The cell property. An invalid CellProperty will be returned if the property does not exist or if the
         *        type does not match.
         */
        template <class T> CellProperty<T> get_cell_property(const std::string& name) const {
            return CellProperty<T>(cprops_.get<T>(name));
        }
        /**
         * \brief Gets the model property named \c name of type \c T.
         * \param name The name of the property to get.
         * \return The model property. An invalid ModelProperty will be returned if the property does not exist or if
         *         the type does not match.
         * Example:
         *      \code
         *          auto T = cloud->get_model_property<mat4>("transformation");
         *          T[0] = mat4::translation(-x0, -y0, -z0);
         *      \endcode
         */
        template <class T> ModelProperty<T> get_model_property(const std::string& name) const {
            return ModelProperty<T>(mprops_.get<T>(name));
        }

        /**
         * \brief If a vertex property of type \c T with name \c name exists, it is returned.
         *        Otherwise, this property is added with default value \c t.
         * \param name The name of the property to get.
         * \param t The default value of the property.
         * \return The vertex property. If the property does not exist, it is added with the default value \c t and
         *         returned.
         */
        template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t=T()) {
            return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
        }
        /**
         * \brief If an edge property of type \c T with name \c name exists, it is returned.
         *        Otherwise, this property is added with default value \c t.
         * \param name The name of the property to get.
         * \param t The default value of the property.
         * \return The edge property. If the property does not exist, it is added with the default value \c t and
         *         returned.
         */
        template <class T> EdgeProperty<T> edge_property(const std::string& name, const T t=T()) {
            return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
        }
        /**
         * \brief If a halfface property of type \c T with name \c name exists, it is returned.
         *        Otherwise, this property is added with default value \c t.
         * \param name The name of the property to get.
         * \param t The default value of the property.
         * \return The halfface property. If the property does not exist, it is added with the default value \c t and
         *         returned.
         */
        template <class T> HalfFaceProperty<T> halfface_property(const std::string& name, const T t=T()) {
            return HalfFaceProperty<T>(hprops_.get_or_add<T>(name, t));
        }
        /**
         * \brief If a face property of type \c T with name \c name exists, it is returned.
         *        Otherwise, this property is added with default value \c t.
         * \param name The name of the property to get.
         * \param t The default value of the property.
         * \return The face property. If the property does not exist, it is added with the default value \c t and
         *         returned.
         */
        template <class T> FaceProperty<T> face_property(const std::string& name, const T t=T()) {
            return FaceProperty<T>(fprops_.get_or_add<T>(name, t));
        }
        /**
         * \brief If a cell property of type \c T with name \c name exists, it is returned.
         *        Otherwise, this property is added with default value \c t.
         * \param name The name of the property to get.
         * \param t The default value of the property.
         * \return The cell property. If the property does not exist, it is added with the default value \c t and
         *         returned.
         */
        template <class T> CellProperty<T> cell_property(const std::string& name, const T t=T()) {
            return CellProperty<T>(cprops_.get_or_add<T>(name, t));
        }
        /**
         * \brief If a model property of type \c T with name \c name exists, it is returned.
         *        Otherwise, this property is added with default value \c t.
         * \param name The name of the property to get.
         * \param t The default value of the property.
         * \return The model property. If the property does not exist, it is added with the default value \c t and
         *         returned.
         */
        template <class T> ModelProperty<T> model_property(const std::string& name, const T t=T()) {
            return ModelProperty<T>(mprops_.get_or_add<T>(name, t));
        }


        /**
         * \brief Remove the vertex property \c p.
         * \param p The property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_vertex_property(VertexProperty<T> &p) { return vprops_.remove(p); }
        /**
         * \brief Remove the vertex property named \c n.
         * \param n The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_vertex_property(const std::string &n) { return vprops_.remove(n); }
        /**
         * \brief Remove the edge property \c p.
         * \param p The property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_edge_property(EdgeProperty<T> &p) { return eprops_.remove(p); }
        /**
         * \brief Remove the edge property named \c n.
         * \param n The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_edge_property(const std::string &n) { return eprops_.remove(n); }
        /**
         * \brief Remove the halfface property \c p.
         * \param p The property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_halfface_property(HalfFaceProperty<T> &p) { return hprops_.remove(p); }
        /**
         * \brief Remove the halfface property named \c n.
         * \param n The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_halfface_property(const std::string &n) { return hprops_.remove(n); }
        /**
         * \brief Remove the face property \c p.
         * \param p The property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_face_property(FaceProperty<T> &p) { return fprops_.remove(p); }
        /**
         * \brief Remove the face property named \c n.
         * \param n The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_face_property(const std::string &n) { return fprops_.remove(n); }
        /**
         * \brief Remove the cell property \c p.
         * \param p The property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_cell_property(CellProperty<T> &p) { return cprops_.remove(p); }
        /**
         * \brief Remove the cell property named \c n.
         * \param n The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_cell_property(const std::string &n) { return cprops_.remove(n); }
        /**
         * \brief Remove the model property \c p.
         * \param p The property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }
        /**
         * \brief Remove the model property named \c n.
         * \param n The name of the property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_model_property(const std::string &n) { return mprops_.remove(n); }

        /**
         * \brief Rename a vertex property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_vertex_property(const std::string &old_name, const std::string &new_name) {
            return vprops_.rename(old_name, new_name);
        }
        /**
         * \brief Rename a halfface property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_halfface_property(const std::string &old_name, const std::string &new_name) {
            return hprops_.rename(old_name, new_name);
        }
        /**
         * \brief Rename a face property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_face_property(const std::string &old_name, const std::string &new_name) {
            return fprops_.rename(old_name, new_name);
        }
        /**
         * \brief Rename an edge property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_edge_property(const std::string &old_name, const std::string &new_name) {
            return eprops_.rename(old_name, new_name);
        }
        /**
         * \brief Rename a cell property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_cell_property(const std::string &old_name, const std::string &new_name) {
            return cprops_.rename(old_name, new_name);
        }
        /**
         * \brief Rename a model property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_model_property(const std::string &old_name, const std::string &new_name) {
            return mprops_.rename(old_name, new_name);
        }

        /**
         * \brief Get the type information of the vertex property \c name.
         * \param name The name of the property.
         * \return The type information of the property. Returns \c typeid(void) if the property does not exist.
         */
        const std::type_info& get_vertex_property_type(const std::string& name) const {
            return vprops_.get_type(name);
        }
        /**
         * \brief Get the type information of the edge property \c name.
         * \param name The name of the property.
         * \return The type information of the property. Returns \c typeid(void) if the property does not exist.
         */
        const std::type_info& get_edge_property_type(const std::string& name) const {
            return eprops_.get_type(name);
        }
        /**
         * \brief Get the type information of the halfface property \c name.
         * \param name The name of the property.
         * \return The type information of the property. Returns \c typeid(void) if the property does not exist.
         */
        const std::type_info& get_halfface_property_type(const std::string& name) const {
            return hprops_.get_type(name);
        }
        /**
         * \brief Get the type information of the face property \c name.
         * \param name The name of the property.
         * \return The type information of the property. Returns \c typeid(void) if the property does not exist.
         */
        const std::type_info& get_face_property_type(const std::string& name) const {
            return fprops_.get_type(name);
        }
        /**
         * \brief Get the type information of the cell property \c name.
         * \param name The name of the property.
         * \return The type information of the property. Returns \c typeid(void) if the property does not exist.
         */
        const std::type_info& get_cell_property_type(const std::string& name) const {
            return cprops_.get_type(name);
        }
        /**
         * \brief Get the type information of the model property \c name.
         * \param name The name of the property.
         * \return The type information of the property. Returns \c typeid(void) if the property does not exist.
         */
        const std::type_info& get_model_property_type(const std::string& name) const {
            return mprops_.get_type(name);
        }


        /**
         * \brief Get the names of all vertex properties.
         * \return A vector containing the names of all vertex properties.
         */
        std::vector<std::string> vertex_properties() const {
            return vprops_.properties();
        }
        /**
         * \brief Get the names of all edge properties.
         * \return A vector containing the names of all edge properties.
         */
        std::vector<std::string> edge_properties() const {
            return eprops_.properties();
        }
        /**
         * \brief Get the names of all halfface properties.
         * \return A vector containing the names of all halfface properties.
         */
        std::vector<std::string> halfface_properties() const {
            return hprops_.properties();
        }
        /**
         * \brief Get the names of all face properties.
         * \return A vector containing the names of all face properties.
         */
        std::vector<std::string> face_properties() const {
            return fprops_.properties();
        }
        /**
         * \brief Get the names of all cell properties.
         * \return A vector containing the names of all cell properties.
         */
        std::vector<std::string> cell_properties() const {
            return cprops_.properties();
        }
        /**
         * \brief Get the names of all model properties.
         * \return A vector containing the names of all model properties.
         */
        std::vector<std::string> model_properties() const {
            return mprops_.properties();
        }

        /**
         * \brief Prints the names of all properties to an output stream (e.g., std::cout).
         * \param output The output stream.
         */
        void property_stats(std::ostream &output) const override;

        //@}


    public: //--------------------------------------------- iterators & circulators

        /// \name Iterators & Circulators
        //@{

        /**
         * \brief Returns an iterator to the beginning of the vertices.
         * \return VertexIterator pointing to the first vertex.
         */
        VertexIterator vertices_begin() const {
            return VertexIterator(Vertex(0), this);
        }

        /**
         * \brief Returns an iterator to the end of the vertices.
         * \return VertexIterator pointing past the last vertex.
         */
        VertexIterator vertices_end() const {
            return VertexIterator(Vertex(static_cast<int>(n_vertices())), this);
        }
        /**
         * \brief Returns a container for range-based iteration over vertices.
         * \return VertexContainer containing all vertices.
         */
        VertexContainer vertices() const {
            return VertexContainer(vertices_begin(), vertices_end());
        }

        /**
         * \brief Returns an iterator to the beginning of the edges.
         * \return EdgeIterator pointing to the first edge.
         */
        EdgeIterator edges_begin() const {
            return EdgeIterator(Edge(0), this);
        }

        /**
         * \brief Returns an iterator to the end of the edges.
         * \return EdgeIterator pointing past the last edge.
         */
        EdgeIterator edges_end() const {
            return EdgeIterator(Edge(static_cast<int>(n_edges())), this);
        }

        /**
         * \brief Returns a container for range-based iteration over edges.
         * \return EdgeContainer containing all edges.
         */
        EdgeContainer edges() const {
            return EdgeContainer(edges_begin(), edges_end());
        }

        /**
         * \brief Returns an iterator to the beginning of the halffaces.
         * \return HalfFaceIterator pointing to the first halfface.
         */
        HalfFaceIterator halffaces_begin() const {
            return HalfFaceIterator(HalfFace(0), this);
        }

        /**
         * \brief Returns an iterator to the end of the halffaces.
         * \return HalfFaceIterator pointing past the last halfface.
         */
        HalfFaceIterator halffaces_end() const {
            return HalfFaceIterator(HalfFace(static_cast<int>(n_halffaces())), this);
        }

        /**
         * \brief Returns a container for range-based iteration over halffaces.
         * \return HalffaceContainer containing all halffaces.
         */
        HalffaceContainer halffaces() const {
            return HalffaceContainer(halffaces_begin(), halffaces_end());
        }

        /**
         * \brief Returns an iterator to the beginning of the faces.
         * \return FaceIterator pointing to the first face.
         */
        FaceIterator faces_begin() const {
            return FaceIterator(Face(0), this);
        }

        /**
         * \brief Returns an iterator to the end of the faces.
         * \return FaceIterator pointing past the last face.
         */
        FaceIterator faces_end() const {
            return FaceIterator(Face(static_cast<int>(n_faces())), this);
        }

        /**
         * \brief Returns a container for range-based iteration over faces.
         * \return FaceContainer containing all faces.
         */
        FaceContainer faces() const {
            return FaceContainer(faces_begin(), faces_end());
        }

        /**
         * \brief Returns an iterator to the beginning of the cells.
         * \return CellIterator pointing to the first cell.
         */
        CellIterator cells_begin() const {
            return CellIterator(Cell(0), this);
        }

        /**
         * \brief Returns an iterator to the end of the cells.
         * \return CellIterator pointing past the last cell.
         */
        CellIterator cells_end() const {
            return CellIterator(Cell(static_cast<int>(n_cells())), this);
        }

        /**
         * \brief Returns a container for range-based iteration over cells.
         * \return CellContainer containing all cells.
         */
        CellContainer cells() const {
            return CellContainer(cells_begin(), cells_end());
        }
        //@}

    public: //--------------------------------------------- adjacency access

        /// \name Adjacency access
        //@{

        /**
         * \brief Returns the vertices around vertex \c v.
         * \param v The vertex.
         * \return A set of adjacent vertices.
         */
        const std::set<Vertex>& vertices(Vertex v) const {
            return vconn_[v].vertices_;
        }

        /**
         * \brief Returns the \c i'th halfface of face \c f. \c i has to be 0 or 1.
         * \param f The face.
         * \param i The index (0 or 1).
         * \return The corresponding HalfFace.
         */
        HalfFace halfface(Face f, unsigned int i) const {
            assert(i<=1);
            return HalfFace(static_cast<int>((f.idx() << 1) + i));
        }

        /**
         * \brief Returns the face of HalfFace \c h.
         * \param h The HalfFace.
         * \return The corresponding Face.
         */
        Face face(HalfFace h) const {
            return Face((h.idx() >> 1));
        }

        /**
         * \brief Returns the twin halfface of halfface \c h.
         * \param h The HalfFace.
         * \return The opposite HalfFace.
         */
        HalfFace opposite(HalfFace h) const {
            return hconn_[h].opposite_;
        }

        /**
         * \brief Returns the \c i'th vertex of edge \c e. \c i has to be 0 or 1.
         * \param e The Edge.
         * \param i The index (0 or 1).
         * \return The corresponding Vertex.
         */
        Vertex vertex(Edge e, unsigned int i) const {
            assert(i<=1);
            return econn_[e].vertices_[i];
        }

        /**
         * \brief Returns the set of vertices around halfface \c h.
         * The vertices are ordered in a way such that its normal points outside of the cell associated with \c h.
         * \param h The HalfFace.
         * \return A vector of ordered vertices.
         */
        const std::vector<Vertex>& vertices(HalfFace h) const {
            return hconn_[h].vertices_;
        }

        /**
         * \brief Returns the set of vertices around face \c f.
         * \param f The Face.
         * \return A vector of ordered vertices.
         */
        const std::vector<Vertex>& vertices(Face f) const {
            return vertices(halfface(f, 0));
        }

        /**
         * \brief Returns the set of vertices around cell \c c.
         * \param c The Cell.
         * \return A set of vertices.
         */
        const std::set<Vertex>& vertices(Cell c) const {
            return cconn_[c].vertices_;
        }

        /**
         * \brief Returns the set of edges around vertex \c v.
         * \param v The Vertex.
         * \return A set of edges.
         */
        const std::set<Edge>& edges(Vertex v) const {
            return vconn_[v].edges_;
        }

        /**
         * \brief Returns the set of edges around halfface \c h.
         * \param h The HalfFace.
         * \return A set of edges.
         */
        const std::set<Edge>& edges(HalfFace h) const {
            return hconn_[h].edges_;
        }

        /**
         * \brief Returns the set of edges around cell \c c.
         * \param c The Cell.
         * \return A set of edges.
         */
        const std::set<Edge>& edges(Cell c) const {
            return cconn_[c].edges_;
        }
        
        /**
         * \brief Returns the set of halffaces around vertex \c v.
         * \param v The Vertex.
         * \return A set of halffaces.
         */
        const std::set<HalfFace>& halffaces(Vertex v) const {
            return vconn_[v].halffaces_;
        }

        /**
         * \brief Returns the set of halffaces around edge \c e.
         * \param e The Edge.
         * \return A set of halffaces.
         */
        const std::set<HalfFace>& halffaces(Edge e) const {
            return econn_[e].halffaces_;
        }

        /**
         * \brief Returns the set of halffaces around cell \c c.
         * \param c The Cell.
         * \return A vector of halffaces.
         */
        const std::vector<HalfFace>& halffaces(Cell c) const {
            return cconn_[c].halffaces_;
        }

        /**
         * \brief Returns the set of cells around vertex \c v.
         * \param v The Vertex.
         * \return A set of cells.
         */
        const std::set<Cell>& cells(Vertex v) const {
            return vconn_[v].cells_;
        }

        /**
         * \brief Returns the set of cells around edge \c e.
         * \param e The Edge.
         * \return A set of cells.
         */
        const std::set<Cell>& cells(Edge e) const {
            return econn_[e].cells_;
        }

        /**
         * \brief Returns the cell associated with halfface \c h.
         * \param h The HalfFace.
         * \return The associated Cell.
         */
        Cell cell(HalfFace h) const {
            return hconn_[h].cell_;
        }
        //@}

    public: //--------------------------------------------- higher-level operations

        /// \name Higher-level Topological Operations
        //@{

        /**
         * \brief Returns whether the mesh is a tetrahedral mesh, i.e., every cell is a tetrahedron.
         * \return True if the mesh consists only of tetrahedral cells, false otherwise.
         */
        bool is_tetraheral_mesh() const;

        /**
         * \brief Returns whether \c v is a boundary vertex, i.e., at least one of its incident halffaces
         * is not associated with a cell.
         * \param v The vertex.
         * \return True if \c v is a boundary vertex, false otherwise.
         */
        bool is_border(Vertex v) const {
            for (auto h : halffaces(v)) { if (is_border(h)) return true; }
            return false;
        }

        /**
         * \brief Returns whether \c e is a boundary edge, i.e., at least one of its incident halffaces
         * is not associated with a cell.
         * \param e The edge.
         * \return True if \c e is a boundary edge, false otherwise.
         */
        bool is_border(Edge e) const {
            for (auto h : halffaces(e)) { if (is_border(h)) return true; }
            return false;
        }

        /**
         * \brief Returns whether \c h is a boundary halfface, i.e., it is not associated with a cell.
         * \param h The HalfFace.
         * \return True if \c h is a boundary halfface, false otherwise.
         */
        bool is_border(HalfFace h) const {
            return (!cell(h).is_valid());
        }

        /**
         * \brief Returns whether \c f is a boundary face, i.e., it is incident to only one cell.
         * \param f The Face.
         * \return True if \c f is a boundary face, false otherwise.
         */
        bool is_border(Face f) const {
            return is_border(halfface(f, 0)) || is_border(halfface(f, 1));
        }

        /**
         * Finds the edge connecting vertices \c a and \c b.
         * \param a The first vertex.
         * \param b The second vertex.
         * \return The edge (a, b), if it exists.
         */
        Edge find_edge(Vertex a, Vertex b) const;

        /**
         * Finds the halfface defined by a sequence of \c vertices (orientation sensitive).
         * \param vertices The ordered sequence of vertices defining the halfface.
         * \return The corresponding HalfFace, if it exists.
         */
        HalfFace find_half_face(const std::vector<Vertex>& vertices) const;

        /**
         * \brief Returns whether face \c f is degenerate, i.e., if it has collinear or coincident vertices.
         * \param f The Face.
         * \return True if the face is degenerate, false otherwise.
         */
        bool is_degenerate(Face f) const;

        /**
         * Extracts the boundary surface and returns its faces.
         * \param faces A reference to a vector of vertex sequences, each defining a boundary face.
         */
        void extract_boundary(std::vector< std::vector<Vertex> >& faces) const;
        //@}

    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /**
         * \brief Returns the position of a vertex.
         * \param v The vertex.
         * \return A reference to the 3D position vector.
         */
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /**
         * \brief Returns a read-only vector of all vertex positions.
         * \return A constant reference to the vector of vertex positions.
         */
        const std::vector<vec3>& points() const override { return vpoint_.vector(); }

        /**
         * \brief Returns a modifiable vector of all vertex positions.
         * \return A reference to the vector of vertex positions.
         */
        std::vector<vec3>& points() override { return vpoint_.vector(); }

        /**
         * \brief Computes face normals by calling compute_face_normal(HalfFace) for each face.
         */
        void update_face_normals();

        /**
         * \brief Computes the normal vector of face \c f.
         * \param f The Face.
         * \return The normal vector of the face.
         */
        vec3 compute_face_normal(Face f) const;

        /**
         * \brief Computes vertex normals for each vertex.
         * \attention For vertices on the border of the polyhedral mesh, the normals point outward. Interior vertices
         *      do not have well-defined normals. This method may be unstable for concave vertices or vertices with
         *      spanning angles close to 0 or 180 degrees.
         */
        void update_vertex_normals();

        /**
         * \brief Computes the length of edge \c e.
         * \param e The Edge.
         * \return The length of the edge.
         */
        float edge_length(Edge e) const;

        //@}


    private: //---------------------------------------------- allocate new elements

        /// allocate a new vertex, resize vertex properties accordingly.
        Vertex new_vertex()
        {
            vprops_.push_back();
            return Vertex(static_cast<int>(n_vertices()-1));
        }

        /// allocate a new edge, resize edge and edge properties accordingly.
        Edge new_edge(Vertex s, Vertex t)
        {
            assert(s != t);
            eprops_.push_back();
            Edge e = Edge(static_cast<int>(n_edges() - 1));
            econn_[e].vertices_ = {s, t};
            vconn_[s].edges_.insert(e);
            vconn_[t].edges_.insert(e);
            vconn_[s].vertices_.insert(t);
            vconn_[t].vertices_.insert(s);
            return e;
        }

        /// allocate a new face (i.e., creates two halffaces), resize face/halfface properties accordingly.
        HalfFace new_face()
        {
            fprops_.push_back();
            hprops_.push_back();
            hprops_.push_back();
            HalfFace h0(static_cast<int>(n_halffaces()-2));
            HalfFace h1(static_cast<int>(n_halffaces()-1));

            hconn_[h0].opposite_ = h1;
            hconn_[h1].opposite_ = h0;

            return h0;
        }

        /// allocate a new cell, resize cell properties accordingly.
        Cell new_cell()
        {
            cprops_.push_back();
            return Cell(static_cast<int>(n_cells()-1));
        }

    private: //------------------------------------------------------- private data

        PropertyContainer vprops_;
        PropertyContainer eprops_;
        PropertyContainer hprops_;
        PropertyContainer fprops_;
        PropertyContainer cprops_;
        PropertyContainer mprops_;

        VertexProperty<VertexConnectivity>      vconn_;
        EdgeProperty<EdgeConnectivity>          econn_;
        HalfFaceProperty<HalfFaceConnectivity>  hconn_;
        CellProperty<CellConnectivity>          cconn_;

        VertexProperty<vec3>    vpoint_;
    };


    //------------------------------------------------------------ output operators

    /// Output stream support for PolyMesh::Vertex.
    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Vertex v)
    {
        return (os << 'v' << v.idx());
    }

    /// Output stream support for PolyMesh::Edge.
    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Edge e)
    {
        return (os << 'e' << e.idx());
    }

    /// Output stream support for PolyMesh::HalfFace.
    inline std::ostream& operator<<(std::ostream& os, PolyMesh::HalfFace h)
    {
        return (os << 'h' << h.idx());
    }

    /// Output stream support for PolyMesh::Face.
    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Face f)
    {
        return (os << 'f' << f.idx());
    }

    /// Output stream support for PolyMesh::Cell.
    inline std::ostream& operator<<(std::ostream& os, PolyMesh::Cell c)
    {
        return (os << 'c' << c.idx());
    }

} // namespace easy3d

#endif // EASY3D_CORE_POLYHEDRAL_MESH_H