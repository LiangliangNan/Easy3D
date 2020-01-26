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

/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications and
 * significant enhancement.
 *		- Surface_mesh (version 1.1)
 * The original code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *
 *----------------------------------------------------------*/

// I would like to process/visualize huge scans (tens of millions of points, or even more),
// an earlier implementation based on double-connected list has the following limitations:
// 	1) no random access of the data;
//  2) OpenGL renderring overhead (needs packing to transfer data to GPU);
//  3) hard to employ OMP support;
//  4) file management (unable to read and write large blocks);
//  5) selection, etc.
// Thus I have this implementation inspired by (actually some code is taken from) 
// Surface_mesh: https://opensource.cit-ec.de/projects/surface_mesh
// The idea behind is that point coordinates and associated attribute values are stored as
// std::vector<T>.

#ifndef EASY3D_POINT_CLOUD_H
#define EASY3D_POINT_CLOUD_H

#include <easy3d/viewer/model.h>
#include <easy3d/core/types.h>
#include <easy3d/core/properties.h>

namespace easy3d {

    /// A for point clouds.
    class PointCloud : public Model
    {

    public: //------------------------------------------------------ topology types


        /// Base class for topology types (internally it is basically an index)
        /// \sa Vertex
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

        private:
            friend class VertexIterator;
            friend class PointCloud;
            int idx_;
        };


        /// this type represents a vertex (internally it is basically an index)
        struct Vertex : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };


    public: //------------------------------------------------------ property types

        /// Vertex property of type T
        template <class T> class VertexProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit VertexProperty() {}
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


        /// Cloud property of type T
        /// \sa VertexProperty
        template <class T> class ModelProperty : public Property<T>
        {
        public:

            /// default constructor
            explicit ModelProperty() {}
            explicit ModelProperty(Property<T> p) : Property<T>(p) {}

            /// access the data stored for the cloud
            typename Property<T>::reference operator[](size_t idx)
            {
                return Property<T>::operator[](idx);
            }

            /// access the data stored for the cloud
            typename Property<T>::const_reference operator[](size_t idx) const
            {
                return Property<T>::operator[](idx);
            }
        };



    public: //------------------------------------------------------ iterator types

        /// this class iterates linearly over all vertices
        /// \sa vertices_begin(), vertices_end()
        class VertexIterator
        {
        public:

            /// Default constructor
            VertexIterator(Vertex v=Vertex(), const PointCloud* m=nullptr) : hnd_(v), cloud_(m)
            {
                if (cloud_ && cloud_->garbage()) while (cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
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
                assert(cloud_);
                while (cloud_->garbage() && cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /// pre-decrement iterator
            VertexIterator& operator--()
            {
                --hnd_.idx_;
                assert(cloud_);
                while (cloud_->garbage() && cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) --hnd_.idx_;
                return *this;
            }

        private:
            Vertex  hnd_;
            const PointCloud* cloud_;
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


    public: //-------------------------------------------- constructor / destructor

        /// \name Construct, destruct, assignment
        //@{

        /// default constructor
        PointCloud();

        // destructor (is virtual, since we inherit from Geometry_representation)
        virtual ~PointCloud();

        /// copy constructor: copies \c rhs to \c *this. performs a deep copy of all properties.
        PointCloud(const PointCloud& rhs) { operator=(rhs); }

        /// assign \c rhs to \c *this. performs a deep copy of all properties.
        PointCloud& operator=(const PointCloud& rhs);

        /// assign \c rhs to \c *this. does not copy custom properties.
        PointCloud& assign(const PointCloud& rhs);

        //@}


    public: //----------------------------------------------- add new vertex

        /// \name Add new elements by hand
        //@{

        /// add a new vertex with position \c p
        Vertex add_vertex(const vec3& p);

        //@}


    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /// returns number of (deleted and valid) vertices in the cloud
        unsigned int vertices_size() const { return (unsigned int) vprops_.size(); }

        /// returns number of vertices in the cloud
        unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }

        /// returns true iff the cloud is empty, i.e., has no vertices
        unsigned int empty() const { return n_vertices() == 0; }

        /// clear cloud: remove all vertices
        void clear();

        /// remove unused memory from vectors
        void free_memory();

        /// resize space for vertices and their currently associated properties.
        void resize(unsigned int nv) { vprops_.resize(nv); }

        /// remove deleted vertices
        void garbage_collection();

        /// deletes the vertex \c v from the cloud
        void delete_vertex(Vertex v);

        /// returns whether vertex \c v is deleted
        /// \sa garbage_collection()
        bool is_deleted(Vertex v) const
        {
            return vdeleted_[v];
        }

        /// return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Vertex v) const
        {
            return (0 <= v.idx()) && (v.idx() < (int)vertices_size());
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
        /** add a model property of type \c T with name \c name and default value \c t.
        fails if a property named \c name exists already, since the name has to be unique.
        in this case it returns an invalid property */
        template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t = T())
        {
            return ModelProperty<T>(mprops_.add<T>(name, t));
        }

        /** get the vertex property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const
        {
            return VertexProperty<T>(vprops_.get<T>(name));
        }
        /** get the model property named \c name of type \c T. returns an invalid
        ModelProperty if the property does not exist or if the type does not match. */
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
        /** if a model property of type \c T with name \c name exists, it is returned.
        otherwise this property is added (with default value \c t) */
        template <class T> ModelProperty<T> model_property(const std::string& name, const T t = T())
        {
            return ModelProperty<T>(mprops_.get_or_add<T>(name, t));
        }

        /// remove the vertex property \c p
        template <class T> void remove_vertex_property(VertexProperty<T>& p)
        {
            vprops_.remove(p);
        }
        /// remove the model property \c p
        template <class T> void remove_model_property(ModelProperty<T>& p)
        {
            mprops_.remove(p);
        }


        /** get the type_info \c T of vertex property named \c. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_vertex_property_type(const std::string& name)
        {
            return vprops_.get_type(name);
        }
        /** get the type_info \c T of model property named \c. returns an typeid(void)
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
        /// returns the names of all model properties
        std::vector<std::string> model_properties() const
        {
            return mprops_.properties();
        }

        /// prints the names of all properties
        void property_stats() const;

        //@}


    public: //--------------------------------------------- iterators

        /// \name Iterators
        //@{

        /// returns start iterator for vertices
        VertexIterator vertices_begin() const
        {
            return VertexIterator(Vertex(0), this);
        }

        /// returns end iterator for vertices
        VertexIterator vertices_end() const
        {
            return VertexIterator(Vertex(vertices_size()), this);
        }

        /// returns vertex container for C++11 range-based for-loops
        VertexContainer vertices() const
        {
            return VertexContainer(vertices_begin(), vertices_end());
        }

        //@}

    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /// position of a vertex (read only)
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /// position of a vertex
        vec3& position(Vertex v) { return vpoint_[v]; }

        /// vector of vertex positions (read only)
        const std::vector<vec3>& points() const { return vpoint_.vector(); }

        /// vector of vertex positions
        std::vector<vec3>& points() { return vpoint_.vector(); }

        //@}

    private: //---------------------------------------------- allocate new elements

        /// allocate a new vertex, resize vertex properties accordingly.
        Vertex new_vertex()
        {
            vprops_.push_back();
            return Vertex(vertices_size()-1);
        }


    private: //--------------------------------------------------- helper functions

        /// are there deleted vertices
        bool garbage() const { return garbage_; }

    private: //------------------------------------------------------- private data

        PropertyContainer		vprops_;
        PropertyContainer		mprops_;

        VertexProperty<bool>	vdeleted_;
        VertexProperty<vec3>    vpoint_;

        unsigned int	deleted_vertices_;
        bool			garbage_;
    };


    //------------------------------------------------------------ output operators

    inline std::ostream& operator<<(std::ostream& os, PointCloud::Vertex v)
    {
        return (os << 'v' << v.idx());
    }

} // namespace easy3d

#endif // EASY3D_POINT_CLOUD_H
