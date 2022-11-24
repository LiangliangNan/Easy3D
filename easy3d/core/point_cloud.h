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


#ifndef EASY3D_CORE_POINT_CLOUD_H
#define EASY3D_CORE_POINT_CLOUD_H

#include <easy3d/core/model.h>
#include <easy3d/core/types.h>
#include <easy3d/core/property.h>

namespace easy3d {

    /**
     * @brief A data structure for point clouds.
     * \class PointCloud easy3d/core/point_cloud.h
     *
     * This implementation is inspired by Surface_mesh
     * https://opensource.cit-ec.de/projects/surface_mesh
     */
    class PointCloud : public virtual Model
    {

    public: //------------------------------------------------------ topology types


        /// @brief Base class for topology types (internally it is basically an index)
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

            /// helper structure to be able to use std::unordered_map
            struct Hash {
                std::size_t operator()(const BaseHandle& h) const { return h.idx(); }
            };

        private:
            friend class PointCloud;
            int idx_;
        };


        /// @brief this type represents a vertex (internally it is basically an index)
        struct Vertex : public BaseHandle
        {
            /// default constructor (with invalid index)
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };


    public: //------------------------------------------------------ property types

        /// @brief Vertex property of type T
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


        /// @brief Cloud property of type T
        /// \sa VertexProperty
        template <class T> class ModelProperty : public Property<T>
        {
        public:

            /// default constructor
            ModelProperty() = default;
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

        /// @brief this class iterates linearly over all vertices
        /// \sa vertices_begin(), vertices_end()
        class VertexIterator
        {
        public:

            /// Default constructor
            explicit VertexIterator(Vertex v=Vertex(), const PointCloud* m=nullptr) : hnd_(v), cloud_(m)
            {
                if (cloud_ && cloud_->has_garbage()) while (cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
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
                while (cloud_->has_garbage() && cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /// pre-decrement iterator
            VertexIterator& operator--()
            {
                --hnd_.idx_;
                assert(cloud_);
                while (cloud_->has_garbage() && cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) --hnd_.idx_;
                return *this;
            }

        private:
            Vertex  hnd_;
            const PointCloud* cloud_;
        };


    public: //-------------------------- containers for C++11 range-based for loops

        /// @brief this helper class is a container for iterating through all
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

        /// @brief default constructor
        PointCloud();

        /// @brief destructor (is virtual, since we inherit from Geometry_representation)
        ~PointCloud() override = default;

        /// @brief copy constructor: copies \c rhs to \c *this. performs a deep copy of all properties.
        PointCloud(const PointCloud& rhs) { operator=(rhs); }

        /// @brief assign \c rhs to \c *this. performs a deep copy of all properties.
        PointCloud& operator=(const PointCloud& rhs);

		/// \brief Merges another point cloud into the current one.
		/// Shifts the indices of vertices of the other point cloud by `number_of_vertices() + number_of_removed_vertices()`.
		/// Copies entries of all property maps which have the same name in both point clouds. That is, property maps which
		/// are only in `other` are ignored.
		/// Also copies elements which are marked as removed, and concatenates the freelists of both point clouds.
		PointCloud& operator+=(const PointCloud& other) { join(other); return *this; }

		/// \brief Merges another point cloud into the current one.
		/// Shifts the indices of vertices of the other point cloud by `number_of_vertices() + number_of_removed_vertices()`.
		/// Copies entries of all property maps which have the same name in both point cloud. That is, property maps which
		/// are only in `other` are ignored.
		/// Also copies elements which are marked as removed, and concatenates the freelists of both point cloud.
		PointCloud& join(const PointCloud& other);

        /// @brief assign \c rhs to \c *this. does not copy custom properties.
        PointCloud& assign(const PointCloud& rhs);

        //@}


    public: //----------------------------------------------- add new vertex

        /// \name Add new elements by hand
        //@{

        /// @brief add a new vertex with position \c p
        Vertex add_vertex(const vec3& p);

        //@}


    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /// @brief returns number of (deleted and valid) vertices in the cloud
        unsigned int vertices_size() const { return (unsigned int) vprops_.size(); }

        /// @brief returns number of vertices in the cloud
        unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }

        /// @brief clear cloud: remove all vertices
        void clear();

        /// @brief resize space for vertices and their currently associated properties.
        void resize(unsigned int nv) { vprops_.resize(nv); }

        /// are there deleted vertices?
        bool has_garbage() const { return garbage_; }

        /// @brief remove deleted vertices
        void collect_garbage();

        /// @brief deletes the vertex \c v from the cloud
        void delete_vertex(Vertex v);

        /// @brief returns whether vertex \c v is deleted
        /// \sa collect_garbage()
        bool is_deleted(Vertex v) const
        {
            return vdeleted_[v];
        }

        /// @brief return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
        bool is_valid(Vertex v) const
        {
            return (0 <= v.idx()) && (v.idx() < (int)vertices_size());
        }

        //@}

    public: //--------------------------------------------------- property handling

        /// \name Property handling
        //@{

        /** @brief add a vertex property of type \c T with name \c name and default value \c t.
         fails if a property named \c name exists already, since the name has to be unique.
         in this case it returns an invalid property */
        template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t=T())
        {
            return VertexProperty<T>(vprops_.add<T>(name, t));
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
        template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t = T())
        {
            return ModelProperty<T>(mprops_.add<T>(name, t));
        }

        /** @brief get the vertex property named \c name of type \c T. returns an invalid
         VertexProperty if the property does not exist or if the type does not match. */
        template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const
        {
            return VertexProperty<T>(vprops_.get<T>(name));
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

        /** @brief if a vertex property of type \c T with name \c name exists, it is returned.
         otherwise this property is added (with default value \c t) */
        template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t=T())
        {
            return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
        }
        /** @brief if a model property of type \c T with name \c name exists, it is returned.
        otherwise this property is added (with default value \c t) */
        template <class T> ModelProperty<T> model_property(const std::string& name, const T t = T())
        {
            return ModelProperty<T>(mprops_.get_or_add<T>(name, t));
        }

        /// @brief remove the vertex property \c p
        template<class T>
        bool remove_vertex_property(VertexProperty<T> &p) { return vprops_.remove(p); }

        /// @brief remove the vertex property named \c n
        bool remove_vertex_property(const std::string &n) { return vprops_.remove(n); }

        /// @brief remove the model property \c p
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }

        /// @brief remove the model property named \c n
        bool remove_model_property(const std::string &n) { return mprops_.remove(n); }

        /// @brief rename a vertex property given its name
        bool rename_vertex_property(const std::string &old_name, const std::string &new_name) {
            return vprops_.rename(old_name, new_name);
        }

        /// @brief rename a model property given its name
        bool rename_model_property(const std::string &old_name, const std::string &new_name) {
            return mprops_.rename(old_name, new_name);
        }

        /** @brief get the type_info \c T of vertex property \p name. returns an typeid(void)
         if the property does not exist or if the type does not match. */
        const std::type_info& get_vertex_property_type(const std::string& name) const
        {
            return vprops_.get_type(name);
        }
        /** @brief get the type_info \c T of model property \p name. returns an typeid(void)
        if the property does not exist or if the type does not match. */
        const std::type_info& get_model_property_type(const std::string& name) const
        {
            return mprops_.get_type(name);
        }

        /// @brief returns the names of all vertex properties
        std::vector<std::string> vertex_properties() const
        {
            return vprops_.properties();
        }
        /// @brief returns the names of all model properties
        std::vector<std::string> model_properties() const
        {
            return mprops_.properties();
        }

        /// @brief prints the names of all properties to an output stream (e.g., std::cout)
        void property_stats(std::ostream &output) const override;

        //@}


    public: //--------------------------------------------- iterators

        /// \name Iterators
        //@{

        /// @brief returns start iterator for vertices
        VertexIterator vertices_begin() const
        {
            return VertexIterator(Vertex(0), this);
        }

        /// @brief returns end iterator for vertices
        VertexIterator vertices_end() const
        {
            return VertexIterator(Vertex(static_cast<int>(vertices_size())), this);
        }

        /// @brief returns vertex container for C++11 range-based for-loops
        VertexContainer vertices() const
        {
            return VertexContainer(vertices_begin(), vertices_end());
        }

        //@}

    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /// @brief position of a vertex (read only)
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /// @brief position of a vertex
        vec3& position(Vertex v) { return vpoint_[v]; }

        /// @brief vector of vertex positions (read only)
        const std::vector<vec3>& points() const override { return vpoint_.vector(); }

        /// @brief vector of vertex positions
        std::vector<vec3>& points() override { return vpoint_.vector(); }

        //@}

    private: //---------------------------------------------- allocate new elements

        /// @brief allocate a new vertex, resize vertex properties accordingly.
        Vertex new_vertex()
        {
            vprops_.push_back();
            return Vertex(static_cast<int>(vertices_size()-1));
        }


    private: //------------------------------------------------------- private data

        PropertyContainer		vprops_;
        PropertyContainer		mprops_;

        VertexProperty<bool>	vdeleted_;
        VertexProperty<vec3>    vpoint_;

        unsigned int	deleted_vertices_;
        bool			garbage_;
    };


    //------------------------------------------------------------ output operators

    /// Output stream support for PointCloud::Vertex.
    inline std::ostream& operator<<(std::ostream& os, PointCloud::Vertex v)
    {
        return (os << 'v' << v.idx());
    }

} // namespace easy3d

#endif // EASY3D_CORE_POINT_CLOUD_H
