/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications.
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


// I would like to process huge scans (tens of millions of points), an earlier 
// implementation based on double-connected list has issues in the following aspects:
// 	1) no random access of the data;
//  2) OpenGL renderring overhead (needs packing to transfer data to GPU);
//  3) hard to employ OMP support;
//  4) file management (unable reading and writing large blocks);
//  5) selection, etc.
// Thus I have this implementation inspired by (actually some code is taken from) 
// SurfaceMesh: https://opensource.cit-ec.de/projects/surface_mesh
// The idea behind is that all coordinate and associated attribute values
// are stored as std::vector<T


#ifndef POINT_CLOUD_H
#define POINT_CLOUD_H


//== INCLUDES =================================================================

#include <easy3d/model.h>
#include <easy3d/types.h>
#include <easy3d/properties.h>


//== NAMESPACE ================================================================


namespace easy3d {


//== CLASS DEFINITION =========================================================


/// A for point clouds.
class PointCloud : public Model
{

public: //------------------------------------------------------ topology types


    /// Base class for topology types (internally it is basically an index)
    /// \sa Vertex
    class Base_handle
    {
    public:

        /// constructor
        explicit Base_handle(int _idx=-1) : idx_(_idx) {}

        /// Get the underlying index of this handle
        int idx() const { return idx_; }

        /// reset handle to be invalid (index=-1)
        void reset() { idx_=-1; }

        /// return whether the handle is valid, i.e., the index is not equal to -1.
        bool is_valid() const { return idx_ != -1; }

        /// are two handles equal?
        bool operator==(const Base_handle& _rhs) const {
            return idx_ == _rhs.idx_;
        }

        /// are two handles different?
        bool operator!=(const Base_handle& _rhs) const {
            return idx_ != _rhs.idx_;
        }

        /// compare operator useful for sorting handles
        bool operator<(const Base_handle& _rhs) const {
            return idx_ < _rhs.idx_;
        }

    private:
        friend class Vertex_iterator;
        friend class PointCloud;
        int idx_;
    };


    /// this type represents a vertex (internally it is basically an index)
    struct Vertex : public Base_handle
    {
        /// default constructor (with invalid index)
        explicit Vertex(int _idx=-1) : Base_handle(_idx) {}
        std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
    };


public: //------------------------------------------------------ property types

    /// Vertex property of type T
    template <class T> class Vertex_property : public Property<T>
    {
    public:

        /// default constructor
        explicit Vertex_property() {}
        explicit Vertex_property(Property<T> p) : Property<T>(p) {}

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
    /// \sa Vertex_property
    template <class T> class Model_property : public Property<T>
    {
    public:

        /// default constructor
        explicit Model_property() {}
        explicit Model_property(Property<T> p) : Property<T>(p) {}

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
    class Vertex_iterator
    {
    public:

        /// Default constructor
        Vertex_iterator(Vertex v=Vertex(), const PointCloud* m=NULL) : hnd_(v), cloud_(m)
        {
            if (cloud_ && cloud_->garbage()) while (cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
        }

        /// get the vertex the iterator refers to
        Vertex operator*()  const { return  hnd_; }

        /// are two iterators equal?
        bool operator==(const Vertex_iterator& rhs) const
        {
            return (hnd_==rhs.hnd_);
        }

        /// are two iterators different?
        bool operator!=(const Vertex_iterator& rhs) const
        {
            return !operator==(rhs);
        }

        /// pre-increment iterator
        Vertex_iterator& operator++()
        {
            ++hnd_.idx_;
            assert(cloud_);
            while (cloud_->garbage() && cloud_->is_valid(hnd_) && cloud_->is_deleted(hnd_)) ++hnd_.idx_;
            return *this;
        }

        /// pre-decrement iterator
        Vertex_iterator& operator--()
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
    class Vertex_container
    {
    public:
        Vertex_container(Vertex_iterator _begin, Vertex_iterator _end) : begin_(_begin), end_(_end) {}
        Vertex_iterator begin() const { return begin_; }
        Vertex_iterator end()   const { return end_;   }
    private:
        Vertex_iterator begin_, end_;
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


public: //------------------------------------------------------------- file IO

    /// \name File IO
    //@{

    /// read cloud from file \c filename. file extension determines file type.
    /// \sa write(const std::string& filename)
    bool read(const std::string& filename);

    /// write cloud to file \c filename. file extensions determines file type.
    /// \sa read(const std::string& filename)
    bool write(const std::string& filename) const;

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
    template <class T> Vertex_property<T> add_vertex_property(const std::string& name, const T t=T())
    {
        return Vertex_property<T>(vprops_.add<T>(name, t));
    }
	/** add a model property of type \c T with name \c name and default value \c t.
	fails if a property named \c name exists already, since the name has to be unique.
	in this case it returns an invalid property */
	template <class T> Model_property<T> add_model_property(const std::string& name, const T t = T())
	{
		return Model_property<T>(mprops_.add<T>(name, t));
	}

    /** get the vertex property named \c name of type \c T. returns an invalid
     Vertex_property if the property does not exist or if the type does not match. */
    template <class T> Vertex_property<T> get_vertex_property(const std::string& name) const
    {
        return Vertex_property<T>(vprops_.get<T>(name));
    }
	/** get the model property named \c name of type \c T. returns an invalid
	Model_property if the property does not exist or if the type does not match. */
	template <class T> Model_property<T> get_model_property(const std::string& name) const
	{
		return Model_property<T>(mprops_.get<T>(name));
	}

    /** if a vertex property of type \c T with name \c name exists, it is returned.
     otherwise this property is added (with default value \c t) */
    template <class T> Vertex_property<T> vertex_property(const std::string& name, const T t=T())
    {
        return Vertex_property<T>(vprops_.get_or_add<T>(name, t));
    }
	/** if a model property of type \c T with name \c name exists, it is returned.
	otherwise this property is added (with default value \c t) */
	template <class T> Model_property<T> model_property(const std::string& name, const T t = T())
	{
		return Model_property<T>(mprops_.get_or_add<T>(name, t));
	}

    /// remove the vertex property \c p
    template <class T> void remove_vertex_property(Vertex_property<T>& p)
    {
        vprops_.remove(p);
    }
	/// remove the model property \c p
	template <class T> void remove_model_property(Model_property<T>& p)
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
    Vertex_iterator vertices_begin() const
    {
        return Vertex_iterator(Vertex(0), this);
    }

    /// returns end iterator for vertices
    Vertex_iterator vertices_end() const
    {
        return Vertex_iterator(Vertex(vertices_size()), this);
    }

    /// returns vertex container for C++11 range-based for-loops
    Vertex_container vertices() const
    {
        return Vertex_container(vertices_begin(), vertices_end());
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

    Property_container		vprops_;
	Property_container		mprops_;

    Vertex_property<bool>	vdeleted_;
    Vertex_property<vec3>  vpoint_;

    unsigned int	deleted_vertices_;
    bool			garbage_;
};


//------------------------------------------------------------ output operators


inline std::ostream& operator<<(std::ostream& os, PointCloud::Vertex v)
{
    return (os << 'v' << v.idx());
}

//=============================================================================
} // namespace easy3d
//=============================================================================
#endif // POINT_CLOUD_H
//=============================================================================
