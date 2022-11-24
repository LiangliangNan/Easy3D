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

#ifndef EASY3D_CORE_GRAPH_H
#define EASY3D_CORE_GRAPH_H


#include <easy3d/core/model.h>

#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/core/property.h>


namespace easy3d {


	/**
	 * @brief A Graph data structure with easy property management.
	 * \class Graph easy3d/core/graph.h
     *
     * This implementation is inspired by Surface_mesh
     * https://opensource.cit-ec.de/projects/surface_mesh
	 */

	class Graph : public virtual Model
	{

	public: //------------------------------------------------------ topology types


		/// Base class for all topology types (internally it is basically an index)
		/// \sa Vertex, Edge
		class BaseHandle
		{
		public:

			/// constructor
			explicit BaseHandle(int _idx = -1) : idx_(_idx) {}

			/// Get the underlying index of this handle
			int idx() const { return idx_; }

			/// reset handle to be invalid (index=-1)
			void reset() { idx_ = -1; }

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
			friend class Graph;
			int idx_;
		};


		/// this type represents a vertex (internally it is basically an index)
		///  \sa Edge
		struct Vertex : public BaseHandle
		{
			/// default constructor (with invalid index)
			explicit Vertex(int _idx = -1) : BaseHandle(_idx) {}
			std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
		};

		/// this type represents an edge (internally it is basically an index)
		/// \sa Vertex
		struct Edge : public BaseHandle
		{
			/// default constructor (with invalid index)
			explicit Edge(int _idx = -1) : BaseHandle(_idx) {}
		};


	public: //-------------------------------------------------- connectivity types

		/// This type stores the vertex connectivity
		/// \sa EdgeConnectivity
		struct VertexConnectivity
		{
			/// all edges connected with the vertex
			std::vector<Edge>  edges_;
		};


		/// This type stores the edge connectivity
		/// \sa VertexConnectivity
		struct EdgeConnectivity
		{
            Vertex source_;
            Vertex target_;
		};


	public: //------------------------------------------------------ property types

		/// Vertex property of type T
		/// \sa EdgeProperty
		template <class T> class VertexProperty : public Property<T>
		{
		public:

			/// default constructor
			explicit VertexProperty() = default;
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
		/// \sa VertexProperty
		template <class T> class EdgeProperty : public Property<T>
		{
		public:

			/// default constructor
			explicit EdgeProperty() = default;
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

		/// Graph property of type T
		/// \sa VertexProperty, EdgeProperty
		template <class T> class ModelProperty : public Property<T>
		{
		public:

			/// default constructor
			explicit ModelProperty() = default;
			explicit ModelProperty(Property<T> p) : Property<T>(p) {}

			/// access the data stored for the graph
			typename Property<T>::reference operator[](size_t idx)
			{
				return Property<T>::operator[](idx);
			}

			/// access the data stored for the graph
			typename Property<T>::const_reference operator[](size_t idx) const
			{
				return Property<T>::operator[](idx);
			}
		};



	public: //------------------------------------------------------ iterator types

		/// this class iterates linearly over all vertices
		/// \sa vertices_begin(), vertices_end()
		/// \sa EdgeIterator
		class VertexIterator
		{
		public:

			/// Default constructor
			explicit VertexIterator(Vertex v = Vertex(), const Graph* g = nullptr) : hnd_(v), graph_(g)
			{
				if (graph_ && graph_->has_garbage()) while (graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
			}

			/// get the vertex the iterator refers to
			Vertex operator*()  const { return  hnd_; }

			/// are two iterators equal?
			bool operator==(const VertexIterator& rhs) const
			{
				return (hnd_ == rhs.hnd_);
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
				assert(graph_);
				while (graph_->has_garbage() && graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
				return *this;
			}

			/// pre-decrement iterator
			VertexIterator& operator--()
			{
				--hnd_.idx_;
				assert(graph_);
				while (graph_->has_garbage() && graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) --hnd_.idx_;
				return *this;
			}

		private:
			Vertex  hnd_;
			const Graph* graph_;
		};


		/// this class iterates linearly over all edges
		/// \sa edges_begin(), edges_end()
		/// \sa VertexIterator
		class EdgeIterator
		{
		public:

			/// Default constructor
            explicit EdgeIterator(Edge e = Edge(), const Graph* g = nullptr) : hnd_(e), graph_(g)
			{
				if (graph_ && graph_->has_garbage()) while (graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
			}

			/// get the edge the iterator refers to
			Edge operator*()  const { return  hnd_; }

			/// are two iterators equal?
			bool operator==(const EdgeIterator& rhs) const
			{
				return (hnd_ == rhs.hnd_);
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
				assert(graph_);
				while (graph_->has_garbage() && graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
				return *this;
			}

			/// pre-decrement iterator
			EdgeIterator& operator--()
			{
				--hnd_.idx_;
				assert(graph_);
				while (graph_->has_garbage() && graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) --hnd_.idx_;
				return *this;
			}

		private:
			Edge  hnd_;
			const Graph* graph_;
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
			VertexIterator end()   const { return end_; }
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
			EdgeIterator end()   const { return end_; }
		private:
			EdgeIterator begin_, end_;
		};


	public: //---------------------------------------------------- circulator types


	    /// this class circulates through all edges connected with a vertex.
	    /// it also acts as a container-concept for C++11 range-based for loops.
	    /// \sa VertexAroundVertexCirculator, edges(Vertex)
	    class EdgeAroundVertexCirculator
	    {
	    public:

	        /// default constructor
            explicit EdgeAroundVertexCirculator(const Graph* g, Vertex v=Vertex())
				: graph_(g), vertex_(v), finished_(false)
	        {
				iterator_ = graph_->vconn_[vertex_].edges_.begin();
				end_ = graph_->vconn_[vertex_].edges_.end();
	        }

	        /// are two circulators equal?
	        bool operator==(const EdgeAroundVertexCirculator& rhs) const {
	            assert(graph_);
				return ((graph_ == rhs.graph_) && (vertex_ == rhs.vertex_) && (iterator_ == rhs.iterator_))
					|| (finished_);	// to behave like a circulator
	        }

	        /// are two circulators different?
	        bool operator!=(const EdgeAroundVertexCirculator& rhs) const {
	            return !operator==(rhs);
	        }

	        /// pre-increment
	        EdgeAroundVertexCirculator& operator++() {
	            assert(graph_);
	            ++iterator_;
				if (iterator_ == end_) {	// to behave like a circulator
					iterator_ = graph_->vconn_[vertex_].edges_.begin();
					finished_ = true;
				}
				return *this;
	        }

	        /// pre-decrement
	        EdgeAroundVertexCirculator& operator--()
	        {
	            assert(graph_);
	            --iterator_;
	            return *this;
	        }

	        /// get the edge the circulator refers to
	        Edge operator*() const { 
				assert(graph_);
				if (iterator_ != end_)	return *iterator_;
				else					return Edge();
			}

	        /// cast to bool: true if vertex is not isolated
			operator bool() const { return !graph_->vconn_[vertex_].edges_.empty(); }

			/// return current vertex
			Vertex vertex() const { return vertex_; }

			// helper for C++11 range-based for-loops
			EdgeAroundVertexCirculator& begin() { iterator_ = graph_->vconn_[vertex_].edges_.begin(); return *this; }
			// helper for C++11 range-based for-loops
			EdgeAroundVertexCirculator& end() { iterator_ = end_; return *this; }

	    private:
			const Graph*  graph_;
			const Vertex  vertex_;
			std::vector<Edge>::const_iterator iterator_;
			// helper for C++11 range-based for-loops
			std::vector<Edge>::const_iterator end_;
			bool finished_;	// for the circulator behavior
	    };



		/// this class circulates through all one-ring neighbors of a vertex.
		/// it also acts as a container-concept for C++11 range-based for loops.
		/// \sa EdgeAroundVertexCirculator, vertices(Vertex)
		class VertexAroundVertexCirculator
		{
		public:

			/// default constructor
            explicit VertexAroundVertexCirculator(const Graph* g, Vertex v = Vertex())
				: graph_(g), vertex_(v), finished_(false)
			{
				iterator_ = graph_->vconn_[vertex_].edges_.begin();
				end_ = graph_->vconn_[vertex_].edges_.end();
			}

			/// are two circulators equal?
			bool operator==(const VertexAroundVertexCirculator& rhs) const {
				assert(graph_);
				return ((graph_ == rhs.graph_) && (vertex_ == rhs.vertex_) && (iterator_ == rhs.iterator_))
					|| (finished_);	// to behave like a circulator
			}

			/// are two circulators different?
			bool operator!=(const VertexAroundVertexCirculator& rhs) const {
				return !operator==(rhs);
			}

			/// pre-increment
			VertexAroundVertexCirculator& operator++() {
				assert(graph_);
				++iterator_;
				if (iterator_ == end_) {	// to behave like a circulator
					iterator_ = graph_->vconn_[vertex_].edges_.begin();
					finished_ = true;
				}
				return *this;
			}

			/// pre-decrement
			VertexAroundVertexCirculator& operator--()
			{
				assert(graph_);
				--iterator_;
				return *this;
			}

			/// get the vertex the circulator refers to
			Vertex operator*() const {
				assert(graph_);
				if (iterator_ != end_) {
					Vertex v = graph_->vertex(*iterator_, 1);
					if (v != vertex_)	return v;
					else				return graph_->vertex(*iterator_, 0);
				}
				return Vertex();
			}

	        /// cast to bool: true if vertex is not isolated
	        operator bool() const { return !graph_->vconn_[vertex_].edges_.empty(); }

	        /// return current vertex
	        Vertex vertex() const { return vertex_; }

			// helper for C++11 range-based for-loops
			VertexAroundVertexCirculator& begin() {
				iterator_ = graph_->vconn_[vertex_].edges_.begin();
				return *this;
			}
			// helper for C++11 range-based for-loops
			VertexAroundVertexCirculator& end() {
				iterator_ = end_;
				return *this;
			}

		private:
			const Graph*  graph_;
			const Vertex  vertex_;
			std::vector<Edge>::const_iterator iterator_;
			// helper for C++11 range-based for-loops
			std::vector<Edge>::const_iterator end_;
			bool finished_;	// for the circulator behavior
		};



	public: //-------------------------------------------- constructor / destructor

		/// \name Construct, destruct, assignment
		//@{

		/// default constructor
		Graph();

		/// destructor
		~Graph() override = default;

		/// copy constructor: copies \c rhs to \c *this. performs a deep copy of all properties.
		Graph(const Graph& rhs) { operator=(rhs); }

		/// assign \c rhs to \c *this. performs a deep copy of all properties.
		Graph& operator=(const Graph& rhs);

		/// assign \c rhs to \c *this. does not copy custom properties.
		Graph& assign(const Graph& rhs);

		//@}

	public: //----------------------------------------------- add new vertex / face

		/// \name Add new elements by hand
		//@{

		/// add a new vertex with position \c p
		Vertex add_vertex(const vec3& p);

		/// add a new edge connecting vertices \c v1 and \c v2
		Edge add_edge(const Vertex& v1, const Vertex& v2);
		//@}

	public: //--------------------------------------------------- memory management

		/// \name Memory Management
		//@{

		/// returns number of (deleted and valid) vertices in the graph
		unsigned int vertices_size() const { return (unsigned int)vprops_.size(); }
		/// returns number of (deleted and valid)edges in the graph
		unsigned int edges_size() const { return (unsigned int)eprops_.size(); }

		/// returns number of vertices in the graph
		unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }
		/// returns number of edges in the graph
		unsigned int n_edges() const { return edges_size() - deleted_edges_; }

        /// \brief Removes all vertices, edges, and properties (and resets garbage state).
        /// \details After calling this method, the graph is the same as newly constructed.
		void clear();

		/// reserve memory (mainly used in file readers)
		void reserve(unsigned int nvertices, unsigned int nedges);

		/// Resize space for vertices, edges, and their currently associated properties.
		/// Note: ne is the number of edges.
		void resize(unsigned int nv, unsigned int ne) {
			vprops_.resize(nv);
			eprops_.resize(ne);
		}

        /// are there deleted vertices or edges?
        bool has_garbage() const { return garbage_; }

		/// remove deleted vertices/edges
		void collect_garbage();


		/// returns whether vertex \c v is deleted
		/// \sa collect_garbage()
		bool is_deleted(Vertex v) const
		{
			return vdeleted_[v];
		}
		/// returns whether edge \c e is deleted
		/// \sa collect_garbage()
		bool is_deleted(Edge e) const
		{
			return edeleted_[e];
		}

		/// return whether vertex \c v is valid, i.e. the index is stores it within the array bounds.
		bool is_valid(Vertex v) const
		{
			return (0 <= v.idx()) && (v.idx() < (int)vertices_size());
		}
		/// return whether edge \c e is valid, i.e. the index is stores it within the array bounds.
		bool is_valid(Edge e) const
		{
			return (0 <= e.idx()) && (e.idx() < (int)edges_size());
		}
		//@}


	public: //---------------------------------------------- low-level connectivity

		/// \name Low-level connectivity
		//@{

		/// returns whether \c v is isolated, i.e., not incident to any edge
		bool is_isolated(Vertex v) const
		{
			return vconn_[v].edges_.empty();
		}

		/// returns the \c i'th vertex of edge \c e. \c i has to be 0 or 1.
		Vertex vertex(Edge e, unsigned int i) const
		{
			assert(i<=1);
			if (i == 0)
			    return econn_[e].source_;
			else
                return econn_[e].target_;
		}

        /// returns the starting vertex of an edge, which is equal to vertex(e, 0).
        Vertex source(Edge e) const { return econn_[e].source_; }

        /// returns the ending vertex of an edge, which is equal to vertex(e, 1).
        Vertex target(Edge e) const { return econn_[e].target_; }
		//@}

	public: //--------------------------------------------------- property handling

		/// \name Property handling
		//@{

		/** add a vertex property of type \c T with name \c name and default value \c t.
		 fails if a property named \c name exists already, since the name has to be unique.
		 in this case it returns an invalid property */
		template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t = T())
		{
			return VertexProperty<T>(vprops_.add<T>(name, t));
		}
		/** add a edge property of type \c T with name \c name and default value \c t.
		 fails if a property named \c name exists already, since the name has to be unique.
		 in this case it returns an invalid property */
		template <class T> EdgeProperty<T> add_edge_property(const std::string& name, const T t = T())
		{
			return EdgeProperty<T>(eprops_.add<T>(name, t));
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
		/** get the edge property named \c name of type \c T. returns an invalid
		 VertexProperty if the property does not exist or if the type does not match. */
		template <class T> EdgeProperty<T> get_edge_property(const std::string& name) const
		{
			return EdgeProperty<T>(eprops_.get<T>(name));
		}
		/** get the model property named \c name of type \c T. returns an invalid
		 ModelProperty if the property does not exist or if the type does not match. */
		template <class T> ModelProperty<T> get_model_property(const std::string& name) const
		{
			return ModelProperty<T>(mprops_.get<T>(name));
		}


		/** if a vertex property of type \c T with name \c name exists, it is returned.
		 otherwise this property is added (with default value \c t) */
		template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t = T())
		{
			return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
		}
		/** if an edge property of type \c T with name \c name exists, it is returned.
		 otherwise this property is added (with default value \c t) */
		template <class T> EdgeProperty<T> edge_property(const std::string& name, const T t = T())
		{
			return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
		}

		/** if a model property of type \c T with name \c name exists, it is returned.
		otherwise this property is added (with default value \c t) */
		template <class T> ModelProperty<T> model_property(const std::string& name, const T t = T())
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

        /// remove the model property \c p
        template<class T>
        bool remove_model_property(ModelProperty<T> &p) { return mprops_.remove(p); }

        /// remove the model property named \c n
        bool remove_model_property(const std::string &n) { return mprops_.remove(n); }

        /// rename a vertex property given its name
        bool rename_vertex_property(const std::string &old_name, const std::string &new_name) {
            return vprops_.rename(old_name, new_name);
        }

        /// rename an edge property given its name
        bool rename_edge_property(const std::string &old_name, const std::string &new_name) {
            return eprops_.rename(old_name, new_name);
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
		/// returns the names of all model properties
		std::vector<std::string> model_properties() const
		{
			return mprops_.properties();
		}

		/// prints the names of all properties to an output stream (e.g., std::cout)
		void property_stats(std::ostream& output) const override;

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

		/// returns circulator for vertices around vertex \c v
		VertexAroundVertexCirculator vertices(Vertex v) const
		{
			return VertexAroundVertexCirculator(this, v);
		}

		/// returns circulator for edges around vertex \c v
		EdgeAroundVertexCirculator edges(Vertex v) const
		{
		    return EdgeAroundVertexCirculator(this, v);
		}
		//@}


	public: //--------------------------------------------- higher-level operations

		/// \name Higher-level Topological Operations
		//@{


		/** returns the valence (number of incident edges or neighboring vertices)
		 of vertex \c v. */
		unsigned int valence(Vertex v) const;

		/// find the edge (a,b)
		Edge find_edge(Vertex a, Vertex b) const;

		/// deletes the vertex \c v from the graph
		void delete_vertex(Vertex v);

		/// deletes the edge \c e from the graph
		void delete_edge(Edge e);
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

		/// compute the length of edge \c e.
		float edge_length(Edge e) const;

		//@}


	private: //---------------------------------------------- allocate new elements

		/// allocate a new vertex, resize vertex properties accordingly.
		Vertex new_vertex()
		{
			vprops_.push_back();
			return Vertex(static_cast<int>(vertices_size() - 1));
		}

		/// allocate a new edge, resize edge properties accordingly.
		Edge new_edge()
		{
			eprops_.push_back();
			return Edge(static_cast<int>(edges_size() - 1));
		}


	private: //------------------------------------------------------- private data

		PropertyContainer vprops_;
		PropertyContainer eprops_;
		PropertyContainer mprops_;

		VertexProperty<VertexConnectivity>      vconn_;
		EdgeProperty<EdgeConnectivity>          econn_;

		VertexProperty<bool>  vdeleted_;
		EdgeProperty<bool>    edeleted_;

		VertexProperty<vec3>  vpoint_;

		unsigned int deleted_vertices_;
		unsigned int deleted_edges_;
		bool garbage_;
	};


	//------------------------------------------------------------ output operators


    /** Output stream support for Graph::Vertex. */
	inline std::ostream& operator<<(std::ostream& os, Graph::Vertex v)
	{
		return (os << 'v' << v.idx());
	}

    /** Output stream support for Graph::Edge. */
	inline std::ostream& operator<<(std::ostream& os, Graph::Edge e)
	{
		return (os << 'e' << e.idx());
	}

} // namespace easy3d

#endif // EASY3D_CORE_GRAPH_H


