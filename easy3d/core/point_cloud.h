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
     * \brief A data structure for point clouds.
     * \class PointCloud easy3d/core/point_cloud.h
     *
     * This implementation is inspired by Surface_mesh
     * https://opensource.cit-ec.de/projects/surface_mesh
     */
    class PointCloud : public Model
    {

    public: //------------------------------------------------------ topology types

        /**
         * \brief Base class for topology types (internally it is basically an index).
         * \class BaseHandle easy3d/core/point_cloud.h
         * \sa Vertex
         */
        class BaseHandle
        {
        public:
            /**
             * \brief Constructor.
             * \param _idx The index of the handle.
             */
            explicit BaseHandle(int _idx=-1) : idx_(_idx) {}

            /**
             * \brief Get the underlying index of this handle.
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
                 * \return The hash value of the handle, which is the index.
                 */
                std::size_t operator()(const BaseHandle& h) const { return h.idx(); }
            };

        private:
            friend class PointCloud;
            int idx_;
        };


        /**
         * \brief This type represents a vertex (internally it is basically an index).
         * \class Vertex easy3d/core/point_cloud.h
         */
        struct Vertex : BaseHandle {
            /**
             * \brief Default constructor (with invalid index).
             * \param _idx The index of the vertex.
             */
            explicit Vertex(int _idx=-1) : BaseHandle(_idx) {}
            /**
             * \brief Output stream operator for Vertex.
             * \param os The output stream.
             * \return The output stream.
             */
            std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
        };


    public: //------------------------------------------------------ property types

        /**
         * \brief Vertex property of type T.
         * \class VertexProperty easy3d/core/point_cloud.h
         */
        template <class T> class VertexProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor.
             */
            VertexProperty() = default;
            /**
             * \brief Constructor with property.
             * \param p The property.
             */
            explicit VertexProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for vertex \c v.
             * \param v The vertex.
             * \return The reference to the data stored for the vertex.
             */
            typename Property<T>::reference operator[](Vertex v)
            {
                return Property<T>::operator[](v.idx());
            }

            /**
             * \brief Access the data stored for vertex \c v.
             * \param v The vertex.
             * \return The const reference to the data stored for the vertex.
             */
            typename Property<T>::const_reference operator[](Vertex v) const
            {
                return Property<T>::operator[](v.idx());
            }
        };


        /**
         * \brief Cloud property of type T.
         * \class ModelProperty easy3d/core/point_cloud.h
         * \sa VertexProperty
         */
        template <class T> class ModelProperty : public Property<T>
        {
        public:
            /**
             * \brief Default constructor.
             */
            ModelProperty() = default;
            /**
             * \brief Constructor with property.
             * \param p The property.
             */
            explicit ModelProperty(Property<T> p) : Property<T>(p) {}

            /**
             * \brief Access the data stored for the cloud.
             * \param idx The index.
             * \return The reference to the data stored for the cloud.
             */
            typename Property<T>::reference operator[](size_t idx) override {
                return Property<T>::operator[](idx);
            }

            /**
             * \brief Access the data stored for the cloud.
             * \param idx The index.
             * \return The const reference to the data stored for the cloud.
             */
            typename Property<T>::const_reference operator[](size_t idx) const override {
                return Property<T>::operator[](idx);
            }
        };



    public: //------------------------------------------------------ iterator types

        /**
         * \brief This class iterates linearly over all vertices.
         * \class VertexIterator easy3d/core/point_cloud.h
         * \sa vertices_begin(), vertices_end()
         */
        class VertexIterator
        {
        public:
            /**
             * \brief Default constructor.
             * \param v The vertex.
             * \param m The point cloud.
             */
            explicit VertexIterator(Vertex v=Vertex(), const PointCloud* m=nullptr) : hnd_(v), cloud_(m)
            {
                if (cloud_ && cloud_->has_garbage()) while (cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
            }

            /**
             * \brief Get the vertex the iterator refers to.
             * \return The vertex.
             */
            Vertex operator*()  const { return  hnd_; }

            /**
             * \brief Are two iterators equal?
             * \param rhs The other iterator.
             * \return True if the iterators are equal, false otherwise.
             */
            bool operator==(const VertexIterator& rhs) const
            {
                return (hnd_==rhs.hnd_);
            }

            /**
             * \brief Are two iterators different?
             * \param rhs The other iterator.
             * \return True if the iterators are different, false otherwise.
             */
            bool operator!=(const VertexIterator& rhs) const
            {
                return !operator==(rhs);
            }

            /**
             * \brief Pre-increment iterator.
             * \return The incremented iterator.
             */
            VertexIterator& operator++()
            {
                ++hnd_.idx_;
                assert(cloud_);
                while (cloud_->has_garbage() && cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
                return *this;
            }

            /**
             * \brief Pre-decrement iterator.
             * \return The decremented iterator.
             */
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

        /**
         * \brief This helper class is a container for iterating through all vertices using C++11
         *      range-based for-loops.
         * \class VertexContainer easy3d/core/point_cloud.h
         * \sa vertices()
         */
        class VertexContainer
        {
        public:
            /**
             * \brief Constructor.
             * \param _begin The begin iterator.
             * \param _end The end iterator.
             */
            VertexContainer(VertexIterator _begin, VertexIterator _end) : begin_(_begin), end_(_end) {}
            /**
             * \brief Get the begin iterator.
             * \return The begin iterator.
             */
            VertexIterator begin() const { return begin_; }
            /**
             * \brief Get the end iterator.
             * \return The end iterator.
             */
            VertexIterator end()   const { return end_;   }
        private:
            VertexIterator begin_, end_;
        };


    public: //-------------------------------------------- constructor / destructor

        /// \name Construct, destruct, assignment
        //@{

        /**
         * \brief Default constructor.
         */
        PointCloud();

        /**
         * \brief Destructor (is virtual, since we inherit from Model).
         */
        ~PointCloud() override = default;

        /**
         * \brief Copy constructor: copies \c rhs to \c *this. Performs a deep copy of all properties.
         * \param rhs The other point cloud.
         */
        PointCloud(const PointCloud& rhs) { operator=(rhs); }

        /**
         * \brief Assign \c rhs to \c *this. Performs a deep copy of all properties.
         * \param rhs The other point cloud.
         * \return The assigned point cloud.
         */
        PointCloud& operator=(const PointCloud& rhs);

        /**
         * \brief Merges another point cloud into the current one.
         * \details Shifts the indices of vertices of the other point cloud by `number_of_vertices() +
         *      number_of_removed_vertices()`.  Copies entries of all property maps which have the same name in both
         *      point clouds. That is, property maps which are only in `other` are ignored. Also copies elements which
         *      are marked as removed, and concatenates the freelists of both point clouds.
         * \param other The other point cloud to merge.
         * \return The merged point cloud.
         */
		PointCloud& operator+=(const PointCloud& other) { join(other); return *this; }

        /**
         * \brief Merges another point cloud into the current one.
         * \details Shifts the indices of vertices of the other point cloud by `number_of_vertices() +
         *      number_of_removed_vertices()`. Copies entries of all property maps which have the same name in both
         *      point clouds. That is, property maps which are only in `other` are ignored. Also copies elements which
         *      are marked as removed, and concatenates the freelists of both point clouds.
         * \param other The other point cloud to merge.
         * \return The merged point cloud.
         */
		PointCloud& join(const PointCloud& other);

        /**
         * \brief Assign \c rhs to \c *this. Does not copy custom properties.
         * \param rhs The other point cloud.
         * \return The assigned point cloud.
         */
        PointCloud& assign(const PointCloud& rhs);

        //@}


    public: //----------------------------------------------- add new vertex

        /// \name Add new elements by hand
        //@{

        /**
         * \brief Add a new vertex with position \c p.
         * \param p The position of the new vertex.
         * \return The added vertex.
         */
        Vertex add_vertex(const vec3& p);

        //@}


    public: //--------------------------------------------------- memory management

        /// \name Memory Management
        //@{

        /**
         * \brief Returns number of (deleted and valid) vertices in the cloud.
         * \return The number of vertices in the cloud.
         */
        unsigned int vertices_size() const { return static_cast<unsigned int>(vprops_.size()); }

        /**
         * \brief Returns number of vertices in the cloud.
         * \return The number of valid vertices in the cloud.
         */
        unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }

        /**
         * \brief Clear cloud: remove all vertices.
         */
        void clear();

        /**
         * \brief Resize space for vertices and their currently associated properties.
         * \param nv The new size for the vertices.
         */
        void resize(unsigned int nv) { vprops_.resize(nv); }

        /**
         * \brief Are there deleted vertices?
         * \return True if there are deleted vertices, false otherwise.
         * \sa delete_vertex(), is_deleted(), and collect_garbage().
         */
        bool has_garbage() const { return garbage_; }

        /**
         * \brief Remove deleted vertices.
         * \sa delete_vertex(), is_deleted(), and has_garbage().
         */
        void collect_garbage();

        /**
         * \brief Deletes the vertex \c v from the cloud.
         * \param v The vertex to delete.
         * \sa collect_garbage(), is_deleted(), and has_garbage().
         */
        void delete_vertex(Vertex v);

        /**
         * \brief Returns whether vertex \c v is deleted.
         * \param v The vertex to check.
         * \return True if the vertex is deleted, false otherwise.
         * \sa collect_garbage(), delete_vertex(), and has_garbage().
         */
        bool is_deleted(Vertex v) const {
            return vdeleted_[v];
        }

        /**
         * \brief Return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
         * \param v The vertex to check.
         * \return True if the vertex is valid, false otherwise.
         */
        bool is_valid(Vertex v) const {
            return (0 <= v.idx()) && (v.idx() < static_cast<int>(vertices_size()));
        }

        //@}

    public: //--------------------------------------------------- property handling

        /// \name Property handling
        //@{

        /**
         * \brief Add a vertex property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The added vertex property. The operation fails if a property with the same name already exists,
         *      since the name has to be unique. In this case, it returns an invalid property.
         */
        template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t=T()) {
            return VertexProperty<T>(vprops_.add<T>(name, t));
        }
        /**
         * \brief Add a vertex property of type \c T with name \c name and default value \c t.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The added vertex property. The operation fails if a property with the same name already exists,
         *      since the name has to be unique. In this case, it returns an invalid property.
         * Example:
         *      \code
         *          auto trans = cloud->add_model_property<mat4>("transformation", mat4::identity());
         *          trans[0] = mat4::translation(-x0, -y0, -z0);
         *      \endcode
         */
        template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t = T()) {
            return ModelProperty<T>(mprops_.add<T>(name, t));
        }

        /**
         * \brief Get the vertex property named \c name of type \c T.
         * \param name The name of the property.
         * \return The vertex property. An invalid VertexProperty will be returned if the property does not exist or
         *      if the type does not match.
         */
        template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const {
            return VertexProperty<T>(vprops_.get<T>(name));
        }

        /**
         * \brief Gets the model property named \c name of type \c T.
         * \param name The name of the property.
         * \return The model property. An invalid ModelProperty will be returned if the property does not exist or
         *      if the type does not match.
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
         * \brief Gets or adds a vertex property of type \c T with name \c name.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The vertex property. If a vertex property of type \c T with name \c name exists, it is returned.
         *      otherwise this property is added (with default value \c t) and then returned.
         */
        template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t=T()) {
            return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
        }
        /**
         * \brief Gets or adds a model property of type \c T with name \c name.
         * \param name The name of the property.
         * \param t The default value of the property.
         * \return The model property. If a model property of type \c T with name \c name exists, it is returned.
         *      otherwise this property is added (with default value \c t) and then returned.
         */
        template <class T> ModelProperty<T> model_property(const std::string& name, const T t = T()) {
            return ModelProperty<T>(mprops_.get_or_add<T>(name, t));
        }

        /**
         * \brief Remove the vertex property \c p.
         * \param p The vertex property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_vertex_property(VertexProperty<T> &p) { return vprops_.remove(p); }

        /**
         * \brief Remove the vertex property named \c n.
         * \param n The name of the vertex property to remove.
         * \return True if the property was removed, false otherwise.
         */
        bool remove_vertex_property(const std::string &n) { return vprops_.remove(n); }

        /**
         * \brief Remove the model property \c p.
         * \param p The model property to remove.
         * \return True if the property was removed, false otherwise.
         */
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }

        /**
         * \brief Remove the model property named \c n.
         * \param n The name of the model property to remove.
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
         * \brief Rename a model property given its name.
         * \param old_name The old name of the property.
         * \param new_name The new name of the property.
         * \return True if the property was renamed, false otherwise.
         */
        bool rename_model_property(const std::string &old_name, const std::string &new_name) {
            return mprops_.rename(old_name, new_name);
        }

        /**
         * \brief Get the type_info \c T of vertex property \p name.
         * \param name The name of the property.
         * \return The type_info of the property. A typeid(void) is returned if the property does not exist or if the
         *      type does not match.
         */
        const std::type_info& get_vertex_property_type(const std::string& name) const {
            return vprops_.get_type(name);
        }
        /**
         * \brief Get the type_info \c T of model property \p name.
         * \param name The name of the property.
         * \return The type_info of the property. A typeid(void) is returned if the property does not exist or if the
         *      type does not match.
         */
        const std::type_info& get_model_property_type(const std::string& name) const {
            return mprops_.get_type(name);
        }

        /**
         * \brief Returns the names of all vertex properties.
         * \return A vector of names of all vertex properties.
         */
        std::vector<std::string> vertex_properties() const {
            return vprops_.properties();
        }
        /**
         * \brief Returns the names of all model properties.
         * \return A vector of names of all model properties.
         */
        std::vector<std::string> model_properties() const {
            return mprops_.properties();
        }

        /**
         * \brief Prints the names of all properties to an output stream (e.g., std::cout).
         * \param output The output stream to print to.
         */
        void property_stats(std::ostream &output) const override;

        //@}


    public: //--------------------------------------------- iterators

        /// \name Iterators
        //@{

        /**
         * \brief Returns start iterator for vertices.
         * \return The start iterator for vertices.
         */
        VertexIterator vertices_begin() const {
            return VertexIterator(Vertex(0), this);
        }

        /**
         * \brief Returns end iterator for vertices.
         * \return The end iterator for vertices.
         */
        VertexIterator vertices_end() const {
            return VertexIterator(Vertex(static_cast<int>(vertices_size())), this);
        }

        /**
         * \brief Returns vertex container for C++11 range-based for-loops.
         * \return The vertex container for range-based for-loops.
         */
        VertexContainer vertices() const {
            return {vertices_begin(), vertices_end()};
        }

        //@}

    public: //------------------------------------------ geometry-related functions

        /// \name Geometry-related Functions
        //@{

        /**
         * \brief Gets the position of a vertex (read only).
         * \param v The vertex to get the position of.
         * \return A const reference to the position of the vertex.
         */
        const vec3& position(Vertex v) const { return vpoint_[v]; }

        /**
         * \brief Gets the position of a vertex.
         * \param v The vertex to get the position of.
         * \return A reference to the position of the vertex.
         */
        vec3& position(Vertex v) { return vpoint_[v]; }

        /**
         * \brief Returns the vector of vertex positions (read only).
         * \return A const reference to the vector of vertex positions.
         */
        const std::vector<vec3>& points() const override { return vpoint_.vector(); }

        /**
         * \brief Returns the vector of vertex positions.
         * \return A reference to the vector of vertex positions.
         */
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

    /**
     * \brief Output stream support for PointCloud::Vertex.
     * \param os The output stream.
     * \param v The vertex to output.
     * \return The output stream.
     */
    inline std::ostream& operator<<(std::ostream& os, PointCloud::Vertex v) {
        return (os << 'v' << v.idx());
    }

} // namespace easy3d

#endif // EASY3D_CORE_POINT_CLOUD_H
