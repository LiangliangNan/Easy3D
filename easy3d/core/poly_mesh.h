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
     * This implementation is inspired by Surface_mesh
     * https://opensource.cit-ec.de/projects/surface_mesh
     */

    class PolyMesh : public virtual Model
    {

    public: //------------------------------------------------------ topology types


        /// Base class for all topology types (internally it is basically an index)
        /// \sa Vertex, Edge, HalfFace, Face, Cell
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
            friend class PolyMesh;
            int idx_;
        };


        /// this type represents a vertex (internally it is basically an index)
        ///  \sa Edge, HalfFace, Face, Cell
        struct Vertex : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };

        /// this type represents an edge (internally it is basically an index)
        /// \sa Vertex, HalfFace, Face, Cell
        struct Edge : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Edge(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'e' << idx(); }
        };


        /// this type represents a halfface (internally it is basically an index)
        /// \sa Vertex, Edge, Face, Cell
        struct HalfFace : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit HalfFace(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'h' << idx(); }
        };

        /// this type represents a face (internally it is basically an index)
        /// \sa Vertex, Edge, HalfFace, Cell
        struct Face : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Face(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'f' << idx(); }
        };

        /// this type represents a polyhedral cell (internally it is basically an index)
        /// \sa Vertex, Edge, HalfFace, Face
        struct Cell : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Cell(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'c' << idx(); }
        };



    public: //-------------------------------------------------- connectivity types

        /// This type stores the vertex connectivity
        /// \sa EdgeConnectivity, HalfFaceConnectivity, CellConnectivity
        struct VertexConnectivity
        {
            std::set<Vertex>     vertices_;
            std::set<Edge>       edges_;
            std::set<HalfFace>   halffaces_;
            std::set<Cell>       cells_;

            void read(std::istream& in);
            void write(std::ostream& out) const;
        };


        /// This type stores the edge connectivity
        /// \sa VertexConnectivity, HalfFaceConnectivity, CellConnectivity
        struct EdgeConnectivity
        {
            std::vector<Vertex>  vertices_;
            std::set<HalfFace>   halffaces_;
            std::set<Cell>       cells_;

            void read(std::istream& in);
            void write(std::ostream& out) const;
        };

        /// This type stores the halfface connectivity
        /// \sa VertexConnectivity, EdgeConnectivity, CellConnectivity
        struct HalfFaceConnectivity
        {
            std::vector<Vertex> vertices_;
            std::set<Edge>   edges_;
            Cell             cell_;
            HalfFace         opposite_;

            void read(std::istream& in);
            void write(std::ostream& out) const;
        };

        /// This type stores the cell connectivity
        /// \sa VertexConnectivity, EdgeConnectivity, HalfFaceConnectivity
        struct CellConnectivity
        {
            std::set<Vertex>     vertices_;
            std::set<Edge>       edges_;
            std::vector<HalfFace>   halffaces_;

            void read(std::istream& in);
            void write(std::ostream& out) const;
        };



    public: //------------------------------------------------------ property types

        /// Vertex property of type T
        /// \sa EdgeProperty, HalfFaceProperty, FaceProperty, CellProperty
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


        /// Edge property of type T
        /// \sa VertexProperty, HalfFaceProperty, FaceProperty, CellProperty
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


        /// HalfFace property of type T
        /// \sa VertexProperty, EdgeProperty, FaceProperty, CellProperty
        template <class T> class HalfFaceProperty : public Property<T>
        {
        public:

            /// default constructor
            HalfFaceProperty() = default;
            explicit HalfFaceProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for halfface \c h
            typename Property<T>::reference operator[](HalfFace h)
            {
                return Property<T>::operator[](h.idx());
            }

            /// access the data stored for halfface \c h
            typename Property<T>::const_reference operator[](HalfFace h) const
            {
                return Property<T>::operator[](h.idx());
            }
        };


        /// Face property of type T
        /// \sa VertexProperty, EdgeProperty, HalfFaceProperty, CellProperty
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


        /// Cell property of type T
        /// \sa VertexProperty, EdgeProperty, HalfFaceProperty, FaceProperty
        template <class T> class CellProperty : public Property<T>
        {
        public:

            /// default constructor
            CellProperty() = default;
            explicit CellProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for Cell \c c
            typename Property<T>::reference operator[](Cell c)
            {
                return Property<T>::operator[](c.idx());
            }

            /// access the data stored for Cell \c c
            typename Property<T>::const_reference operator[](Cell c) const
            {
                return Property<T>::operator[](c.idx());
            }
        };


        /// Mesh property of type T
        /// \sa VertexProperty, EdgeProperty, HalfFaceProperty, FaceProperty, CellProperty
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

        /// this class iterates linearly over all vertices
        /// \sa vertices_begin(), vertices_end()
        /// \sa EdgeIterator, HalfFaceIterator, FaceIterator, CellIterator
        class VertexIterator
        {
        public:

            /// Default constructor
            explicit VertexIterator(Vertex v=Vertex(), const PolyMesh* m=nullptr) : hnd_(v), mesh_(m)
            {
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
                return *this;
            }

            /// pre-decrement iterator
            VertexIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Vertex  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all edges
        /// \sa edges_begin(), edges_end()
        /// \sa VertexIterator, HalfFaceIterator, FaceIterator, CellIterator
        class EdgeIterator
        {
        public:

            /// Default constructor
            explicit EdgeIterator(Edge e=Edge(), const PolyMesh* m=nullptr) : hnd_(e), mesh_(m)
            {
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
                return *this;
            }

            /// pre-decrement iterator
            EdgeIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Edge  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all halffaces
        /// \sa halffaces_begin(), halffaces_end()
        /// \sa VertexIterator, EdgeIterator, FaceIterator, CellIterator
        class HalfFaceIterator
        {
        public:

            /// Default constructor
            explicit HalfFaceIterator(HalfFace h=HalfFace(), const PolyMesh* m=nullptr) : hnd_(h), mesh_(m)
            {
            }

            /// get the halfface the iterator refers to
            HalfFace operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const HalfFaceIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const HalfFaceIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            HalfFaceIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            HalfFaceIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            HalfFace  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all faces
        /// \sa faces_begin(), faces_end()
        /// \sa VertexIterator, EdgeIterator, HalfFaceIterator, CellIterator
        class FaceIterator
        {
        public:

            /// Default constructor
            explicit FaceIterator(Face f=Face(), const PolyMesh* m=nullptr) : hnd_(f), mesh_(m)
            {
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
                return *this;
            }

            /// pre-decrement iterator
            FaceIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Face  hnd_;
            const PolyMesh* mesh_;
        };


        /// this class iterates linearly over all cells
        /// \sa cells_begin(), cells_end()
        /// \sa VertexIterator, EdgeIterator, HalfFaceIterator, FaceIterator
        class CellIterator
        {
        public:

            /// Default constructor
            explicit CellIterator(Cell c=Cell(), const PolyMesh* m=nullptr) : hnd_(c), mesh_(m)
            {
            }

            /// get the cell the iterator refers to
            Cell operator*()  const { return  hnd_; }

            /// are two iterators equal?
            bool operator==(const CellIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /// are two iterators different?
            bool operator!=(const CellIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /// pre-increment iterator
            CellIterator& operator++()
            {
                ++hnd_.idx_;
                assert(mesh_);
                return *this;
            }

            /// pre-decrement iterator
            CellIterator& operator--()
            {
                --hnd_.idx_;
                assert(mesh_);
                return *this;
            }

        private:
            Cell  hnd_;
            const PolyMesh* mesh_;
        };


    public: //-------------------------- containers for C++11 range-based for loops

        /// this helper class is a container for iterating through all
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


        /// this helper class is a container for iterating through all
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


        /// this helper class is a container for iterating through all
        /// halffaces using C++11 range-based for-loops.
        /// \sa halffaces()
        class HalffaceContainer
        {
        public:
            HalffaceContainer(HalfFaceIterator _begin, HalfFaceIterator _end) : begin_(_begin), end_(_end) {}
            HalfFaceIterator begin() const { return begin_; }
            HalfFaceIterator end()   const { return end_;   }
        private:
            HalfFaceIterator begin_, end_;
        };


        /// this helper class is a container for iterating through all
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

        
        /// this helper class is a container for iterating through all
        /// cells using C++11 range-based for-loops.
        /// \sa cells()
        class CellContainer
        {
        public:
            CellContainer(CellIterator _begin, CellIterator _end) : begin_(_begin), end_(_end) {}
            CellIterator begin() const { return begin_; }
            CellIterator end()   const { return end_;   }
        private:
            CellIterator begin_, end_;
        };


    public: //-------------------------------------------- constructor / destructor

        /// \name Construct, destruct, assignment
        //@{

        /// default constructor
        PolyMesh();

        // destructor
        ~PolyMesh() override = default;

        /// copy constructor: copies \c rhs to \c *this. performs a deep copy of all properties.
        PolyMesh(const PolyMesh& rhs) { operator=(rhs); }

        /// assign \c rhs to \c *this. performs a deep copy of all properties.
        PolyMesh& operator=(const PolyMesh& rhs);

        /// assign \c rhs to \c *this. does not copy custom properties.
        PolyMesh& assign(const PolyMesh& rhs);
        //@}

        //! \name File IO
        //!@{

        //! \brief Read mesh from a PM file \p filename.
        //! Mainly for quick debug purposes. Client code should use PolyMeshIO.
        //! \sa PolyMeshIO.
        bool read(const std::string& filename);

        //! \brief Write mesh to a PM file \p filename.
        //! Mainly for quick debug purposes. Client code should use PolyMeshIO.
        //! \sa PolyMeshIO.
        bool write(const std::string& filename) const;
        //@}

    public: //----------------------------------------------- add new vertex / face / cell

        /// \name Add new elements by hand
        //@{

        /// add a new vertex with position \c p
        Vertex add_vertex(const vec3& p) { Vertex v = new_vertex(); vpoint_[v] = p; return v; }

        /// add a new cell defined by \c faces.
        /// \param faces The input faces created by add_face(), add_triangle(), or add_quad().
        /// \sa add_face(), add_triangle(), add_quad(), add_tetra(), add_hexa()
        Cell add_cell(const std::vector<HalfFace>& faces);

        /// add a new tetrahedron defined by its faces.
        /// \param {f0, f1, f2, f3} The input faces created by add_face() or add_triangle().
        /// \sa add_cell(), add_tetra(), add_face(), add_triangle().
        Cell add_tetra(HalfFace f0, HalfFace f1, HalfFace f2, HalfFace f3) { return add_cell({f0, f1, f2, f3}); }

        /// add a new tetrahedron connecting vertices \c v0, \c v1, \c v2, \c v3.
        /// \param {v0, v1, v2, v3} The input vertices created by add_vertex().
        /// \details It creates all the faces and the cell, and adds them to the mesh.
        /// \sa add_vertex()
        Cell add_tetra(Vertex v0, Vertex v1, Vertex v2, Vertex v3);

        /// add a new hexahedron connecting vertices \c v0, \c v1, \c v2, \c v3, \c v4, \c v5, \c v6, \c v7.
        /// \param {v0, v1, v2, v3, v4, v5, v6, v7} The input vertices created by add_vertex().
        ///     The vertices must be ordered as bellow:
        ///             3--------------------2
        ///             |\                   |\
        ///             | \                  | \
        ///             |  \                 |  \
        ///             |   7----------------+---6
        ///             |   |                |   |
        ///             0---+----------------1   |
        ///              \  |                 \  |
        ///               \ |                  \ |
        ///                \|                   \|
        ///                 4--------------------5
        /// \details It creates all the faces and the cell, and adds them to the mesh.
        /// \sa add_vertex()
        Cell add_hexa(Vertex v0, Vertex v1, Vertex v2, Vertex v3, Vertex v4, Vertex v5, Vertex v6, Vertex v7);

        /// add a new face connecting \c vertices
        /// \param vertices The input vertices created by add_vertex().
        /// \sa add_triangle(), add_quad()
        HalfFace add_face(const std::vector<Vertex>& vertices);

        /// add a new triangle face connecting vertices \c v0, \c v1, \c v2.
        /// \param {v0, v1, v2} The input vertices created by add_vertex().
        /// \sa add_face(), add_quad()
        HalfFace add_triangle(Vertex v0, Vertex v1, Vertex v2) { return add_face({ v0, v1, v2 }); }

        /// add a new quad face connecting vertices \c v0, \c v1, \c v2, \c v3.
        /// \param {v0, v1, v2, v3} The input vertices created by add_vertex().
        /// \sa add_face(), add_triangle()
        HalfFace add_quad(Vertex v0, Vertex v1, Vertex v2, Vertex v3) { return add_face({ v0, v1, v2, v3}); }

        //@}

    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /// returns number of vertices in the mesh
        unsigned int n_vertices() const { return (unsigned int) vprops_.size(); }
        /// returns number of edges in the mesh
        unsigned int n_edges() const { return (unsigned int) eprops_.size(); }
        /// returns number of halffaces in the mesh
        unsigned int n_halffaces() const { return (unsigned int) hprops_.size(); }
        /// returns number of faces in the mesh
        unsigned int n_faces() const { return (unsigned int) fprops_.size(); }
        /// returns number of cells in the mesh
        unsigned int n_cells() const { return (unsigned int) cprops_.size(); }

        /// \brief Removes all vertices, edges, halffaces, faces, cells and properties.
        /// \details After calling this method, the mesh is the same as newly constructed. The additional properties
        /// (such as normal vectors) are also removed and must thus be re-added if needed.
        void clear();

        /// resizes space for vertices, edges, halffaces, and their currently
        /// associated properties.
        /// Note: nf is the number of faces. for halffaces, nh = 2 * nf. */
        void resize(unsigned int nv, unsigned int ne, unsigned int nf, unsigned int nc) {
            vprops_.resize(nv);
            eprops_.resize(ne);
            hprops_.resize(2 * nf);
            fprops_.resize(nf);
            cprops_.resize(nc);
        }

        /// return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Vertex v) const
        {
            return (0 <= v.idx()) && (v.idx() < (int)n_vertices());
        }
        /// return whether edge \c e is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Edge e) const
        {
            return (0 <= e.idx()) && (e.idx() < (int)n_edges());
        }
        /// return whether halfface \c h is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(HalfFace h) const
        {
            return (0 <= h.idx()) && (h.idx() < (int)n_halffaces());
        }
        /// return whether face \c f is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Face f) const
        {
            return (0 <= f.idx()) && (f.idx() < (int)n_faces());
        }
        /// return whether cell \c h is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Cell c) const
        {
            return (0 <= c.idx()) && (c.idx() < (int)n_cells());
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
        /** add a edge property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> EdgeProperty<T> add_edge_property(const std::string& name, const T t=T())
        {
            return EdgeProperty<T>(eprops_.add<T>(name, t));
        }
        /** add a halfface property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> HalfFaceProperty<T> add_halfface_property(const std::string& name, const T t=T())
        {
            return HalfFaceProperty<T>(hprops_.add<T>(name, t));
        }
        /** add a face property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> FaceProperty<T> add_face_property(const std::string& name, const T t=T())
        {
            return FaceProperty<T>(fprops_.add<T>(name, t));
        }
        /** add a cell property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> CellProperty<T> add_cell_property(const std::string& name, const T t=T())
        {
            return CellProperty<T>(cprops_.add<T>(name, t));
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
        /** get the edge property named \c name of type \c T. returns an invalid
         EdgeProperty if the property does not exist or if the type does not match. */
        template <class T> EdgeProperty<T> get_edge_property(const std::string& name) const
        {
            return EdgeProperty<T>(eprops_.get<T>(name));
        }
        /** get the halfface property named \c name of type \c T. returns an invalid
         HalfFaceProperty if the property does not exist or if the type does not match. */
        template <class T> HalfFaceProperty<T> get_halfface_property(const std::string& name) const
        {
            return HalfFaceProperty<T>(hprops_.get<T>(name));
        }
        /** get the face property named \c name of type \c T. returns an invalid
         FaceProperty if the property does not exist or if the type does not match. */
        template <class T> FaceProperty<T> get_face_property(const std::string& name) const
        {
            return FaceProperty<T>(fprops_.get<T>(name));
        }
        /** get the cell property named \c name of type \c T. returns an invalid
         CellProperty if the property does not exist or if the type does not match. */
        template <class T> CellProperty<T> get_cell_property(const std::string& name) const
        {
            return CellProperty<T>(cprops_.get<T>(name));
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
        /** if an edge property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> EdgeProperty<T> edge_property(const std::string& name, const T t=T())
        {
            return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
        }
        /** if a halfface property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> HalfFaceProperty<T> halfface_property(const std::string& name, const T t=T())
        {
            return HalfFaceProperty<T>(hprops_.get_or_add<T>(name, t));
        }
        /** if a face property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> FaceProperty<T> face_property(const std::string& name, const T t=T())
        {
            return FaceProperty<T>(fprops_.get_or_add<T>(name, t));
        }
        /** if a cell property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> CellProperty<T> cell_property(const std::string& name, const T t=T())
        {
            return CellProperty<T>(cprops_.get_or_add<T>(name, t));
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

        /// remove the edge property \c p
        template<class T>
        bool remove_edge_property(EdgeProperty<T> &p) { return eprops_.remove(p); }

        /// remove the edge property named \c n
        bool remove_edge_property(const std::string &n) { return eprops_.remove(n); }

        /// remove the halfface property \c p
        template<class T>
        bool remove_halfface_property(HalfFaceProperty<T> &p) { return hprops_.remove(p); }

        /// remove the halfface property named \c n
        bool remove_halfface_property(const std::string &n) { return hprops_.remove(n); }

        /// remove the face property \c p
        template<class T>
        bool remove_face_property(FaceProperty<T> &p) { return fprops_.remove(p); }

        /// remove the face property named \c n
        bool remove_face_property(const std::string &n) { return fprops_.remove(n); }

        /// remove the cell property \c p
        template<class T>
        bool remove_cell_property(CellProperty<T> &p) { return cprops_.remove(p); }

        /// remove the cell property named \c n
        bool remove_cell_property(const std::string &n) { return cprops_.remove(n); }

        /// remove the model property \c p
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }

        /// remove the model property named \c n
        bool remove_model_property(const std::string &n) { return mprops_.remove(n); }

        /// rename a vertex property given its name
        bool rename_vertex_property(const std::string &old_name, const std::string &new_name) {
            return vprops_.rename(old_name, new_name);
        }

        /// rename a halfface property given its name
        bool rename_halfface_property(const std::string &old_name, const std::string &new_name) {
            return hprops_.rename(old_name, new_name);
        }

        /// rename a face property given its name
        bool rename_face_property(const std::string &old_name, const std::string &new_name) {
            return fprops_.rename(old_name, new_name);
        }

        /// rename an edge property given its name
        bool rename_edge_property(const std::string &old_name, const std::string &new_name) {
            return eprops_.rename(old_name, new_name);
        }

        /// rename a cell property given its name
        bool rename_cell_property(const std::string &old_name, const std::string &new_name) {
            return cprops_.rename(old_name, new_name);
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
        /** get the type_info \c T of edge property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_edge_property_type(const std::string& name) const
        {
            return eprops_.get_type(name);
        }
        /** get the type_info \c T of halfface property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_halfface_property_type(const std::string& name) const
        {
            return hprops_.get_type(name);
        }
        /** get the type_info \c T of face property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_face_property_type(const std::string& name) const
        {
            return fprops_.get_type(name);
        }
        /** get the type_info \c T of cell property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_cell_property_type(const std::string& name) const
        {
            return cprops_.get_type(name);
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
        /// returns the names of all edge properties
        std::vector<std::string> edge_properties() const
        {
            return eprops_.properties();
        }
        /// returns the names of all halfface properties
        std::vector<std::string> halfface_properties() const
        {
            return hprops_.properties();
        }
        /// returns the names of all face properties
        std::vector<std::string> face_properties() const
        {
            return fprops_.properties();
        }
        /// returns the names of all cell properties
        std::vector<std::string> cell_properties() const
        {
            return cprops_.properties();
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
            return VertexIterator(Vertex(static_cast<int>(n_vertices())), this);
        }

        /// returns vertex container for C++11 range-based for-loops
        VertexContainer vertices() const
        {
            return VertexContainer(vertices_begin(), vertices_end());
        }

        /// returns start iterator for edges
        EdgeIterator edges_begin() const
        {
            return EdgeIterator(Edge(0), this);
        }

        /// returns end iterator for edges
        EdgeIterator edges_end() const
        {
            return EdgeIterator(Edge(static_cast<int>(n_edges())), this);
        }

        /// returns edge container for C++11 range-based for-loops
        EdgeContainer edges() const
        {
            return EdgeContainer(edges_begin(), edges_end());
        }

        /// returns start iterator for halffaces
        HalfFaceIterator halffaces_begin() const
        {
            return HalfFaceIterator(HalfFace(0), this);
        }

        /// returns end iterator for halffaces
        HalfFaceIterator halffaces_end() const
        {
            return HalfFaceIterator(HalfFace(static_cast<int>(n_halffaces())), this);
        }

        /// returns halfface container for C++11 range-based for-loops
        HalffaceContainer halffaces() const
        {
            return HalffaceContainer(halffaces_begin(), halffaces_end());
        }

        /// returns start iterator for faces
        FaceIterator faces_begin() const
        {
            return FaceIterator(Face(0), this);
        }

        /// returns end iterator for faces
        FaceIterator faces_end() const
        {
            return FaceIterator(Face(static_cast<int>(n_faces())), this);
        }

        /// returns face container for C++11 range-based for-loops
        FaceContainer faces() const
        {
            return FaceContainer(faces_begin(), faces_end());
        }

        /// returns start iterator for cells
        CellIterator cells_begin() const
        {
            return CellIterator(Cell(0), this);
        }

        /// returns end iterator for cells
        CellIterator cells_end() const
        {
            return CellIterator(Cell(static_cast<int>(n_cells())), this);
        }

        /// returns cell container for C++11 range-based for-loops
        CellContainer cells() const
        {
            return CellContainer(cells_begin(), cells_end());
        }
        //@}

    public: //--------------------------------------------- adjacency access

        /// \name Adjacency access
        //@{

        /// returns the vertices around vertex \c v
        const std::set<Vertex>& vertices(Vertex v) const
        {
            return vconn_[v].vertices_;
        }

        /// returns the \c i'th halfface of face \c f. \c i has to be 0 or 1.
        HalfFace halfface(Face f, unsigned int i) const
        {
            assert(i<=1);
            return HalfFace(static_cast<int>((f.idx() << 1) + i));
        }

        /// returns the face of HalfFace \c h.
        Face face(HalfFace h) const {
            return Face((h.idx() >> 1));
        }

        /// returns the twin halfface of halfface \c h.
        HalfFace opposite(HalfFace h) const
        {
            return hconn_[h].opposite_;
        }

        /// returns the \c i'th vertex of edge \c e. \c i has to be 0 or 1.
        Vertex vertex(Edge e, unsigned int i) const
        {
            assert(i<=1);
            return econn_[e].vertices_[i];
        }

        /// returns the set of vertices around halfface \c h.
        /// The vertices are ordered in a way such that its normal points outside of the cell associated with \c h.
        const std::vector<Vertex>& vertices(HalfFace h) const
        {
            return hconn_[h].vertices_;
        }

        /// returns the set of vertices around face \c f
        const std::vector<Vertex>& vertices(Face f) const
        {
            return vertices(halfface(f, 0));
        }

        /// returns the set of vertices around cell \c c
        const std::set<Vertex>& vertices(Cell c) const
        {
            return cconn_[c].vertices_;
        }

        /// returns the set of edges around vertex \c v
        const std::set<Edge>& edges(Vertex v) const
        {
            return vconn_[v].edges_;
        }

        /// returns the set of edges around halfface \c h
        const std::set<Edge>& edges(HalfFace h) const
        {
            return hconn_[h].edges_;
        }

        /// returns the set of edges around cell \c c
        const std::set<Edge>& edges(Cell c) const
        {
            return cconn_[c].edges_;
        }
        
        /// returns the set of halffaces around vertex \c v
        const std::set<HalfFace>& halffaces(Vertex v) const
        {
            return vconn_[v].halffaces_;
        }

        /// returns the set of halffaces around edge \c e
        const std::set<HalfFace>& halffaces(Edge e) const
        {
            return econn_[e].halffaces_;
        }

        /// returns the set of halffaces around cell \c c
        const std::vector<HalfFace>& halffaces(Cell c) const
        {
            return cconn_[c].halffaces_;
        }

        /// returns the set of cells around vertex \c v
        const std::set<Cell>& cells(Vertex v) const
        {
            return vconn_[v].cells_;
        }

        /// returns the set of cells around edge \c e
        const std::set<Cell>& cells(Edge e) const
        {
            return econn_[e].cells_;
        }

        /// returns the cell associated with halfface \c h
        Cell cell(HalfFace h) const
        {
            return hconn_[h].cell_;
        }
        //@}

    public: //--------------------------------------------- higher-level operations

        /// \name Higher-level Topological Operations
        //@{

        /// returns whether the mesh a tetrahedral mesh, i.e., every cell is a tetrahedron.
        bool is_tetraheral_mesh() const;

        /// returns whether \c v is a boundary vertex, i.e., at least one of its incident halfface
        /// is not associated with a cell.
        bool is_border(Vertex v) const {
            for (auto h : halffaces(v)) { if (is_border(h)) return true; }
            return false;
        }

        /// returns whether \c e is a boundary edge, i.e., at least one of its incident halfface
        /// is not associated with a cell.
        bool is_border(Edge e) const {
            for (auto h : halffaces(e)) { if (is_border(h)) return true; }
            return false;
        }

        /// returns whether \c f is a boundary face, i.e., it is not associated with a cell.
        bool is_border(HalfFace h) const {
            return (!cell(h).is_valid());
        }

        /// returns whether \c f is a boundary face, i.e., it is incident to only one cell.
        bool is_border(Face f) const {
            return is_border(halfface(f, 0)) || is_border(halfface(f, 1));
        }

        /// find the edge (a,b)
        Edge find_edge(Vertex a, Vertex b) const;

        /// find the halfface defined by a sequence of \c vertices (orientation sensitive)
        HalfFace find_half_face(const std::vector<Vertex>& vertices) const;

        /// returns whether face \c f is degenerate
        bool is_degenerate(Face f) const;

        /// extracts the boundary surface and return its faces
        void extract_boundary(std::vector< std::vector<Vertex> >& faces) const;
        //@}

    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /// position of a vertex (read only)
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /// vector of vertex positions (read only)
        const std::vector<vec3>& points() const override { return vpoint_.vector(); }

        /// @brief vector of vertex positions
        std::vector<vec3>& points() override { return vpoint_.vector(); }

        /// compute face normals by calling compute_face_normal(HalfFace) for each face.
        void update_face_normals();

        /// compute normal vector of face \c f.
        vec3 compute_face_normal(Face f) const;

        /**
         * Computes vertex normals for each vertex.
         * \attention For vertices on the border of the polyhedral mesh, the normals point outside.
         *      For interior vertices, vertex normals are not defined.
         *      This method is not stable for concave vertices or vertices with spanning angles close to 0 or 180
         *      degrees (but these are very rare cases for polyhedral meshes).
         */
        void update_vertex_normals();

        /// compute the length of edge \c e.
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