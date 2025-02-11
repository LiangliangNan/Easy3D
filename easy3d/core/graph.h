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
	 * \brief A Graph data structure with easy property management.
	 * \class Graph easy3d/core/graph.h
	 *
	 * This implementation is inspired by Surface_mesh
	 * https://opensource.cit-ec.de/projects/surface_mesh
	 */
	class Graph : public Model
	{

	public: //------------------------------------------------------ topology types

		/**
		 * \brief Base class for all topology types (internally it is basically an index).
		 * \sa Vertex, Edge
		 */
		class BaseHandle
		{
		public:
			/**
			 * \brief Constructor with index.
			 * \param _idx The index of the handle.
			 */
			explicit BaseHandle(int _idx = -1) : idx_(_idx) {}

			/**
			 * \brief Get the underlying index of this handle.
			 * \return The index of the handle.
			 */
			int idx() const { return idx_; }

			/**
			 * \brief Reset handle to be invalid (index=-1).
			 */
			void reset() { idx_ = -1; }

			/**
			 * \brief Return whether the handle is valid, i.e., the index is not equal to -1.
			 * \return True if the handle is valid, false otherwise.
			 */
			bool is_valid() const { return idx_ != -1; }

			/**
			 * \brief Equality operator.
			 * \param _rhs The other handle to compare with.
			 * \return True if the handles are equal, false otherwise.
			 */
			bool operator==(const BaseHandle& _rhs) const {
				return idx_ == _rhs.idx_;
			}

			/**
			 * \brief Inequality operator.
			 * \param _rhs The other handle to compare with.
			 * \return True if the handles are different, false otherwise.
			 */
			bool operator!=(const BaseHandle& _rhs) const {
				return idx_ != _rhs.idx_;
			}

			/**
			 * \brief Less than operator useful for sorting handles.
			 * \param _rhs The other handle to compare with.
			 * \return True if this handle is less than the other handle, false otherwise.
			 */
			bool operator<(const BaseHandle& _rhs) const {
				return idx_ < _rhs.idx_;
			}

            /// Helper structure to be able to use std::unordered_map
            struct Hash {
				/**
				 * \brief Hash function for BaseHandle.
				 * \param h The handle to hash.
				 * \return The hash value of the handle, which is the index.
				 */
                std::size_t operator()(const BaseHandle& h) const { return h.idx(); }
            };

		private:
			friend class Graph;
			int idx_;
		};


		/**
		 * \brief This type represents a vertex (internally it is basically an index).
		 * \sa Edge
		 */
		struct Vertex : BaseHandle
		{
			/**
			 * \brief Default constructor (with invalid index).
			 * \param _idx The index of the vertex.
			 */
			explicit Vertex(int _idx = -1) : BaseHandle(_idx) {}
			/**
			 * \brief Output operator.
			 * \param os The output stream.
			 * \return The output stream.
			 */
			std::ostream& operator<<(std::ostream& os) const { return os << 'v' << idx(); }
		};

		/**
		 * \brief This type represents an edge (internally it is basically an index).
		 * \sa Vertex
		 */
		struct Edge : BaseHandle
		{
			/**
			 * \brief Default constructor (with invalid index).
			 * \param _idx The index of the edge.
			 */
			explicit Edge(int _idx = -1) : BaseHandle(_idx) {}
		};


	public: //-------------------------------------------------- connectivity types

		/**
		 * \brief This type stores the vertex connectivity.
		 * \sa EdgeConnectivity
		 */
		struct VertexConnectivity
		{
			/// All edges connected with the vertex.
			std::vector<Edge>  edges_;
		};

		/**
		 * \brief This type stores the edge connectivity.
		 * \sa VertexConnectivity
		 */
		struct EdgeConnectivity
		{
            Vertex source_;	///< The source vertex of the edge.
            Vertex target_;	///< The target vertex of the edge.
		};


	public: //------------------------------------------------------ property types

		/**
		 * \brief Vertex property of type T.
		 * \tparam T The type of the property.
		 * \sa EdgeProperty
		 */
		template <class T> class VertexProperty : public Property<T>
		{
		public:
			/// Default constructor
			explicit VertexProperty() = default;
			/**
			 * \brief Constructor with a property.
			 * \param p The property.
			 */
			explicit VertexProperty(Property<T> p) : Property<T>(p) {}

			/**
			 * \brief Access the data stored for vertex \c v.
			 * \param v The vertex.
			 * \return A reference to the data stored for the vertex.
			 */
			typename Property<T>::reference operator[](Vertex v)
			{
				return Property<T>::operator[](v.idx());
			}

			/**
			 * \brief Access the data stored for vertex \c v.
			 * \param v The vertex.
			 * \return A const reference to the data stored for the vertex.
			 */
			typename Property<T>::const_reference operator[](Vertex v) const
			{
				return Property<T>::operator[](v.idx());
			}
		};


		/**
		 * \brief Edge property of type T.
		 * \tparam T The type of the property.
		 * \sa VertexProperty
		 */
		template <class T> class EdgeProperty : public Property<T>
		{
		public:
			/// Default constructor
			explicit EdgeProperty() = default;
			/**
			 * \brief Constructor with a property.
			 * \param p The property.
			 */
			explicit EdgeProperty(Property<T> p) : Property<T>(p) {}

			/**
			 * \brief Access the data stored for edge \c e.
			 * \param e The edge.
			 * \return A reference to the data stored for the edge.
			 */
			typename Property<T>::reference operator[](Edge e)
			{
				return Property<T>::operator[](e.idx());
			}

			/**
			 * \brief Access the data stored for edge \c e.
			 * \param e The edge.
			 * \return A const reference to the data stored for the edge.
			 */
			typename Property<T>::const_reference operator[](Edge e) const
			{
				return Property<T>::operator[](e.idx());
			}
		};

		/**
		 * \brief Graph property of type T.
		 * \tparam T The type of the property.
		 * \sa VertexProperty, EdgeProperty
		 */
		template <class T> class ModelProperty : public Property<T>
		{
		public:
			/// Default constructor
			explicit ModelProperty() = default;
			/**
			 * \brief Constructor with a property.
			 * \param p The property.
			 */
			explicit ModelProperty(Property<T> p) : Property<T>(p) {}

			/**
			 * \brief Access the data stored for the graph.
			 * \param idx The index.
			 * \return A reference to the data stored for the graph.
			 */
			typename Property<T>::reference operator[](size_t idx) override
			{
				return Property<T>::operator[](idx);
			}

			/**
			 * \brief Access the data stored for the graph.
			 * \param idx The index.
			 * \return A const reference to the data stored for the graph.
			 */
			typename Property<T>::const_reference operator[](size_t idx) const override
			{
				return Property<T>::operator[](idx);
			}
		};



	public: //------------------------------------------------------ iterator types

		/**
		 * \brief This class iterates linearly over all vertices.
		 * \sa vertices_begin(), vertices_end()
		 * \sa EdgeIterator
		 */
		class VertexIterator
		{
		public:
			/**
			 * \brief Default constructor.
			 * \param v The vertex.
			 * \param g The graph.
			 */
			explicit VertexIterator(Vertex v = Vertex(), const Graph* g = nullptr) : hnd_(v), graph_(g)
			{
				if (graph_ && graph_->has_garbage()) while (graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
			}

			/**
			 * \brief Get the vertex the iterator refers to.
			 * \return The vertex.
			 */
			Vertex operator*()  const { return  hnd_; }

			/**
			 * \brief Equality operator.
			 * \param rhs The other iterator to compare with.
			 * \return True if the iterators are equal, false otherwise.
			 */
			bool operator==(const VertexIterator& rhs) const
			{
				return (hnd_ == rhs.hnd_);
			}

			/**
			 * \brief Inequality operator.
			 * \param rhs The other iterator to compare with.
			 * \return True if the iterators are different, false otherwise.
			 */
			bool operator!=(const VertexIterator& rhs) const
			{
				return !operator==(rhs);
			}

			/**
			 * \brief Pre-increment iterator.
			 * \return A reference to the incremented iterator.
			 */
			VertexIterator& operator++()
			{
				++hnd_.idx_;
				assert(graph_);
				while (graph_->has_garbage() && graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
				return *this;
			}

			/**
			 * \brief Pre-decrement iterator.
			 * \return A reference to the decremented iterator.
			 */
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


		/**
		 * \brief This class iterates linearly over all edges.
		 * \sa edges_begin(), edges_end()
		 * \sa VertexIterator
		 */
		class EdgeIterator
		{
		public:

			/// Default constructor
            explicit EdgeIterator(Edge e = Edge(), const Graph* g = nullptr) : hnd_(e), graph_(g)
			{
				if (graph_ && graph_->has_garbage()) while (graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
			}

			/**
			 * \brief Get the edge the iterator refers to.
			 * \return The edge.
			 */
			Edge operator*()  const { return  hnd_; }

			/**
			 * \brief Equality operator.
			 * \param rhs The other iterator to compare with.
			 * \return True if the iterators are equal, false otherwise.
			 */
			bool operator==(const EdgeIterator& rhs) const
			{
				return (hnd_ == rhs.hnd_);
			}

			/**
			 * \brief Inequality operator.
			 * \param rhs The other iterator to compare with.
			 * \return True if the iterators are different, false otherwise.
			 */
			bool operator!=(const EdgeIterator& rhs) const
			{
				return !operator==(rhs);
			}

			/**
			 * \brief Pre-increment iterator.
			 * \return A reference to the incremented iterator.
			 */
			EdgeIterator& operator++()
			{
				++hnd_.idx_;
				assert(graph_);
				while (graph_->has_garbage() && graph_->is_valid(hnd_) && graph_->is_deleted(hnd_)) ++hnd_.idx_;
				return *this;
			}

			/**
			 * \brief Pre-decrement iterator.
			 * \return A reference to the decremented iterator.
			 */
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

		/**
		 * \brief This helper class is a container for iterating through all vertices using C++11 range-based for-loops.
		 * \sa vertices()
		 */
		class VertexContainer
		{
		public:
			/**
			 * \brief Constructor with begin and end iterators.
			 * \param _begin The begin iterator.
			 * \param _end The end iterator.
			 */
			VertexContainer(VertexIterator _begin, VertexIterator _end) : begin_(_begin), end_(_end) {}
			/// Returns the begin iterator.
			VertexIterator begin() const { return begin_; }
			/// Returns the end iterator.
			VertexIterator end()   const { return end_; }
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
			 * \brief Constructor with begin and end iterators.
			 * \param _begin The begin iterator.
			 * \param _end The end iterator.
			 */
			EdgeContainer(EdgeIterator _begin, EdgeIterator _end) : begin_(_begin), end_(_end) {}
			/// Returns the begin iterator.
			EdgeIterator begin() const { return begin_; }
			/// Returns the end iterator.
			EdgeIterator end()   const { return end_; }
		private:
			EdgeIterator begin_, end_;
		};


	public: //---------------------------------------------------- circulator types

		/**
		 * \brief This class circulates through all edges connected with a vertex.
		 * It also acts as a container-concept for C++11 range-based for loops.
		 * \sa VertexAroundVertexCirculator, edges(Vertex)
		 */
	    class EdgeAroundVertexCirculator
	    {
	    public:
	    	/**
			 * \brief Default constructor.
			 * \param g The graph.
			 * \param v The vertex.
			 */
            explicit EdgeAroundVertexCirculator(const Graph* g, Vertex v=Vertex())
				: graph_(g), vertex_(v), finished_(false)
	        {
				iterator_ = graph_->vconn_[vertex_].edges_.begin();
				end_ = graph_->vconn_[vertex_].edges_.end();
	        }

	    	/**
			 * \brief Equality operator.
			 * \param rhs The other circulator to compare with.
			 * \return True if the circulators are equal, false otherwise.
			 */
	        bool operator==(const EdgeAroundVertexCirculator& rhs) const {
	            assert(graph_);
				return ((graph_ == rhs.graph_) && (vertex_ == rhs.vertex_) && (iterator_ == rhs.iterator_))
					|| (finished_);	// to behave like a circulator
	        }

	    	/**
			 * \brief Inequality operator.
			 * \param rhs The other circulator to compare with.
			 * \return True if the circulators are different, false otherwise.
			 */
	        bool operator!=(const EdgeAroundVertexCirculator& rhs) const {
	            return !operator==(rhs);
	        }

	    	/**
			 * \brief Pre-increment circulator.
			 * \return A reference to the incremented circulator.
			 */
	        EdgeAroundVertexCirculator& operator++() {
	            assert(graph_);
	            ++iterator_;
				if (iterator_ == end_) {	// to behave like a circulator
					iterator_ = graph_->vconn_[vertex_].edges_.begin();
					finished_ = true;
				}
				return *this;
	        }

	    	/**
			 * \brief Pre-decrement circulator.
			 * \return A reference to the decremented circulator.
			 */
	        EdgeAroundVertexCirculator& operator--()
	        {
	            assert(graph_);
	            --iterator_;
	            return *this;
	        }

	    	/**
			 * \brief Get the edge the circulator refers to.
			 * \return The edge.
			 */
	        Edge operator*() const { 
				assert(graph_);
				if (iterator_ != end_)	return *iterator_;
				else					return Edge();
			}

	    	/**
			 * \brief Cast to bool: true if vertex is not isolated.
			 * \return True if vertex is not isolated, false otherwise.
			 */
			operator bool() const { return !graph_->vconn_[vertex_].edges_.empty(); }

	    	/**
			 * \brief Return current vertex.
			 * \return The vertex.
			 */
			Vertex vertex() const { return vertex_; }

			/// Helper for C++11 range-based for-loops
			EdgeAroundVertexCirculator& begin() { iterator_ = graph_->vconn_[vertex_].edges_.begin(); return *this; }
			/// Helper for C++11 range-based for-loops
			EdgeAroundVertexCirculator& end() { iterator_ = end_; return *this; }

	    private:
			const Graph*  graph_;
			const Vertex  vertex_;
			std::vector<Edge>::const_iterator iterator_;
			// helper for C++11 range-based for-loops
			std::vector<Edge>::const_iterator end_;
			bool finished_;	// for the circulator behavior
	    };


		/**
		 * \brief This class circulates through all one-ring neighbors of a vertex.
		 * It also acts as a container-concept for C++11 range-based for loops.
		 * \sa EdgeAroundVertexCirculator, vertices(Vertex)
		 */
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

			/**
			 * \brief Equality operator.
			 * \param rhs The other circulator to compare with.
			 * \return True if the circulators are equal, false otherwise.
			 */
			bool operator==(const VertexAroundVertexCirculator& rhs) const {
				assert(graph_);
				return ((graph_ == rhs.graph_) && (vertex_ == rhs.vertex_) && (iterator_ == rhs.iterator_))
					|| (finished_);	// to behave like a circulator
			}

			/**
			 * \brief Inequality operator.
			 * \param rhs The other circulator to compare with.
			 * \return True if the circulators are different, false otherwise.
			 */
			bool operator!=(const VertexAroundVertexCirculator& rhs) const {
				return !operator==(rhs);
			}

			/**
			 * \brief Pre-increment circulator.
			 * \return A reference to the incremented circulator.
			 */
			VertexAroundVertexCirculator& operator++() {
				assert(graph_);
				++iterator_;
				if (iterator_ == end_) {	// to behave like a circulator
					iterator_ = graph_->vconn_[vertex_].edges_.begin();
					finished_ = true;
				}
				return *this;
			}

			/**
			 * \brief Pre-decrement circulator.
			 * \return A reference to the decremented circulator.
			 */
			VertexAroundVertexCirculator& operator--()
			{
				assert(graph_);
				--iterator_;
				return *this;
			}

			/**
			 * \brief Get the vertex the circulator refers to.
			 * \return The vertex.
			 */
			Vertex operator*() const {
				assert(graph_);
				if (iterator_ != end_) {
					Vertex v = graph_->vertex(*iterator_, 1);
					if (v != vertex_)	return v;
					else				return graph_->vertex(*iterator_, 0);
				}
				return Vertex();
			}

			/**
			 * \brief Cast to bool: true if vertex is not isolated.
			 * \return True if vertex is not isolated, false otherwise.
			 */
	        operator bool() const { return !graph_->vconn_[vertex_].edges_.empty(); }

			/**
			 * \brief Return current vertex.
			 * \return The vertex.
			 */
	        Vertex vertex() const { return vertex_; }

			/// Helper for C++11 range-based for-loops
			VertexAroundVertexCirculator& begin() {
				iterator_ = graph_->vconn_[vertex_].edges_.begin();
				return *this;
			}
			/// Helper for C++11 range-based for-loops
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

		/**
		 * \brief Copy constructor: copies \c rhs to \c *this. Performs a deep copy of all properties.
		 * \param rhs The graph to copy from.
		 */
		Graph(const Graph& rhs) { operator=(rhs); }

		/**
		 * \brief Assign \c rhs to \c *this. Performs a deep copy of all properties.
		 * \param rhs The graph to assign from.
		 * \return A reference to this graph.
		 */
		Graph& operator=(const Graph& rhs);

		/**
		 * \brief Assign \c rhs to \c *this. Does not copy custom properties.
		 * \param rhs The graph to assign from.
		 * \return A reference to this graph.
		 */
		Graph& assign(const Graph& rhs);

		//@}

	public: //----------------------------------------------- add new vertex / face

		/// \name Add new elements by hand
		//@{

		/**
		 * \brief Add a new vertex with position \c p.
		 * \param p The position of the vertex.
		 * \return The added vertex.
		 */
		Vertex add_vertex(const vec3& p);

		/**
		 * \brief Add a new edge connecting vertices \c v1 and \c v2.
		 * \param v1 The first vertex.
		 * \param v2 The second vertex.
		 * \return The added edge.
		 */
		Edge add_edge(const Vertex& v1, const Vertex& v2);
		//@}

	public: //--------------------------------------------------- memory management

		/// \name Memory Management
		//@{

		/**
		 * \brief Returns number of (deleted and valid) vertices in the graph.
		 * \return The number of vertices.
		 */
		unsigned int vertices_size() const { return static_cast<unsigned int>(vprops_.size()); }
		/**
		 * \brief Returns number of (deleted and valid) edges in the graph.
		 * \return The number of edges.
		 */
		unsigned int edges_size() const { return static_cast<unsigned int>(eprops_.size()); }

		/**
		 * \brief Returns number of vertices in the graph.
		 * \return The number of vertices.
		 */
		unsigned int n_vertices() const { return vertices_size() - deleted_vertices_; }
		/**
		 * \brief Returns number of edges in the graph.
		 * \return The number of edges.
		 */
		unsigned int n_edges() const { return edges_size() - deleted_edges_; }

		/**
		 * \brief Removes all vertices, edges, and properties, and resets the garbage state.
		 * \details After calling this method, the graph is the same as newly constructed.
		 */
		void clear();
		/**
		 * \brief Reserves memory for vertices and edges (mainly used in file readers)
		 * \param nvertices The number of vertices to reserve.
		 * \param nedges The number of edges to reserve.
		 */
		void reserve(unsigned int nvertices, unsigned int nedges);
		/**
		 * \brief Resizes the space for vertices, edges, and their currently associated properties.
		 * \param nv The number of vertices.
		 * \param ne The number of edges.
		 */
		void resize(unsigned int nv, unsigned int ne) {
			vprops_.resize(nv);
			eprops_.resize(ne);
		}

		/**
		 * \brief Checks if there are deleted vertices or edges.
		 * \return True if there are deleted vertices or edges, false otherwise.
		 */
        bool has_garbage() const { return garbage_; }
		/**
		 * \brief Removes deleted vertices and edges.
		 */
		void collect_garbage();

		/**
		 * \brief Checks if a vertex is deleted.
		 * \param v The vertex to check.
		 * \return True if the vertex is deleted, false otherwise.
		 * \sa collect_garbage()
		 */
		bool is_deleted(Vertex v) const { return vdeleted_[v]; }
		/**
		 * \brief Checks if an edge is deleted.
		 * \param e The edge to check.
		 * \return True if the edge is deleted, false otherwise.
		 * \sa collect_garbage()
		 */
		bool is_deleted(Edge e) const { return edeleted_[e]; }

		/**
		 * \brief Checks if a vertex is valid, i.e. the index is within the array bounds.
		 * \param v The vertex to check.
		 * \return True if the vertex is valid, false otherwise.
		 */
		bool is_valid(Vertex v) const
		{
			return (0 <= v.idx()) && (v.idx() < static_cast<int>(vertices_size()));
		}
		/**
		 * \brief Checks if an edge is valid, i.e. the index is within the array bounds.
		 * \param e The edge to check.
		 * \return True if the edge is valid, false otherwise.
		 */
		bool is_valid(Edge e) const
		{
			return (0 <= e.idx()) && (e.idx() < static_cast<int>(edges_size()));
		}
		//@}


	public: //---------------------------------------------- low-level connectivity

		/// \name Low-level connectivity
		//@{

		/**
		 * \brief Checks if a vertex is isolated (not incident to any edge).
		 * \param v The vertex to check.
		 * \return True if the vertex is isolated, false otherwise.
		 */
		bool is_isolated(Vertex v) const
		{
			return vconn_[v].edges_.empty();
		}

		/**
		 * \brief Returns the i-th vertex of an edge.
		 * \param e The edge.
		 * \param i The index (0 or 1).
		 * \return The i-th vertex of the edge.
		 */
		Vertex vertex(Edge e, unsigned int i) const
		{
			assert(i<=1);
			if (i == 0)
			    return econn_[e].source_;
			else
                return econn_[e].target_;
		}

		/**
		 * \brief Returns the starting vertex of an edge, which is equal to vertex(e, 0).
		 * \param e The edge.
		 */
        Vertex source(Edge e) const { return econn_[e].source_; }
		/**
		 * \brief Returns the ending vertex of an edge, which is equal to vertex(e, 1).
		 * \param e The edge.
		 */
        Vertex target(Edge e) const { return econn_[e].target_; }
		//@}

	public: //--------------------------------------------------- property handling

		/// \name Property handling
		//@{

		/**
		 * \brief Adds a vertex property.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \param t The default value of the property.
		 * \return The added vertex property. The operation fails if a property with the same name already exists,
		 *		since the name has to be unique. In this case, it returns an invalid property.
		 */
		template <class T> VertexProperty<T> add_vertex_property(const std::string& name, const T t = T())
		{
			return VertexProperty<T>(vprops_.add<T>(name, t));
		}
		/**
		 * \brief Adds an edge property.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \param t The default value of the property.
		 * \return The added edge property. The operation fails if a property with the same name already exists,
		 *		since the name has to be unique. In this case, it returns an invalid property.
		 */
		template <class T> EdgeProperty<T> add_edge_property(const std::string& name, const T t = T())
		{
			return EdgeProperty<T>(eprops_.add<T>(name, t));
		}
		/**
		 * \brief Adds a model property.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \param t The default value of the property.
		 * \return The added model property. The operation fails if a property with the same name already exists,
		 *		since the name has to be unique. In this case, it returns an invalid property.
		 */
		template <class T> ModelProperty<T> add_model_property(const std::string& name, const T t = T())
		{
			return ModelProperty<T>(mprops_.add<T>(name, t));
		}

		/**
		 * \brief Gets the vertex property of the specified type and name.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \return The vertex property. If the property does not exist or the type does not match, it returns an
		 *		invalid property.
		 */
		template <class T> VertexProperty<T> get_vertex_property(const std::string& name) const
		{
			return VertexProperty<T>(vprops_.get<T>(name));
		}
		/**
		 * \brief Gets the edge property of the specified type and name.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \return The edge property. If the property does not exist or the type does not match, it returns an
		 *		invalid property.
		 */
		template <class T> EdgeProperty<T> get_edge_property(const std::string& name) const
		{
			return EdgeProperty<T>(eprops_.get<T>(name));
		}
		/**
		 * \brief Gets the model property of the specified type and name.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \return The model property. If the property does not exist or the type does not match, it returns an
		 *		invalid property.
		 */
		template <class T> ModelProperty<T> get_model_property(const std::string& name) const
		{
			return ModelProperty<T>(mprops_.get<T>(name));
		}

		/**
		 * \brief Gets or adds a vertex property.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \param t The default value of the property.
		 * \return The vertex property.
		 */
		template <class T> VertexProperty<T> vertex_property(const std::string& name, const T t = T())
		{
			return VertexProperty<T>(vprops_.get_or_add<T>(name, t));
		}
		/**
		 * \brief Gets or adds an edge property.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \param t The default value of the property.
		 * \return The edge property.
		 */
		template <class T> EdgeProperty<T> edge_property(const std::string& name, const T t = T())
		{
			return EdgeProperty<T>(eprops_.get_or_add<T>(name, t));
		}

		/**
		 * \brief Gets or adds a model property.
		 * \tparam T The type of the property.
		 * \param name The name of the property.
		 * \param t The default value of the property.
		 * \return The model property.
		 */
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

		/**
		 * \brief Gets the type information of a vertex property.
		 * \param name The name of the property.
		 * \return The type information of the property. Returns a typeid(void) if the property does not exist or
		 *		if the type does not match.
		 */
		const std::type_info& get_vertex_property_type(const std::string& name) const
		{
			return vprops_.get_type(name);
		}
		/**
		 * \brief Gets the type information of an edge property.
		 * \param name The name of the property.
		 * \return The type information of the property. Returns a typeid(void) if the property does not exist or
		 *		if the type does not match.
		 */
		const std::type_info& get_edge_property_type(const std::string& name) const
		{
			return eprops_.get_type(name);
		}
		/**
		 * \brief Gets the type information of a model property.
		 * \param name The name of the property.
		 * \return The type information of the property. Returns a typeid(void) if the property does not exist or
		 *		if the type does not match.
		 */
		const std::type_info& get_model_property_type(const std::string& name) const
		{
			return mprops_.get_type(name);
		}


		/// Returns the names of all vertex properties
		std::vector<std::string> vertex_properties() const
		{
			return vprops_.properties();
		}
		/// Returns the names of all edge properties
		std::vector<std::string> edge_properties() const
		{
			return eprops_.properties();
		}
		/// Returns the names of all model properties
		std::vector<std::string> model_properties() const
		{
			return mprops_.properties();
		}

		/**
		 * \brief Prints the names of all properties to an output stream.
		 * \param output The output stream.
		 */
		void property_stats(std::ostream& output) const override;

		//@}


	public: //--------------------------------------------- iterators & circulators

		/// \name Iterators & Circulators
		//@{

		/**
		 * \brief Returns the start iterator for vertices.
		 * \return The start iterator for vertices.
		 */
		VertexIterator vertices_begin() const
		{
			return VertexIterator(Vertex(0), this);
		}

		/**
		 * \brief Returns the end iterator for vertices.
		 * \return The end iterator for vertices.
		 */
		VertexIterator vertices_end() const
		{
			return VertexIterator(Vertex(static_cast<int>(vertices_size())), this);
		}

		/**
		 * \brief Returns the vertex container for range-based for-loops.
		 * \return The vertex container.
		 */
		VertexContainer vertices() const
		{
			return VertexContainer(vertices_begin(), vertices_end());
		}

		/**
		 * \brief Returns the start iterator for edges.
		 * \return The start iterator for edges.
		 */
		EdgeIterator edges_begin() const
		{
			return EdgeIterator(Edge(0), this);
		}

		/**
		 * \brief Returns the end iterator for edges.
		 * \return The end iterator for edges.
		 */
		EdgeIterator edges_end() const
		{
			return EdgeIterator(Edge(static_cast<int>(edges_size())), this);
		}

		/**
		 * \brief Returns the edge container for range-based for-loops.
		 * \return The edge container.
		 */
		EdgeContainer edges() const
		{
			return EdgeContainer(edges_begin(), edges_end());
		}

		/**
		 * \brief Returns the circulator for vertices around a vertex.
		 * \param v The vertex.
		 * \return The circulator for vertices around the vertex.
		 */
		VertexAroundVertexCirculator vertices(Vertex v) const
		{
			return VertexAroundVertexCirculator(this, v);
		}

		/**
		 * \brief Returns the circulator for edges around a vertex.
		 * \param v The vertex.
		 * \return The circulator for edges around the vertex.
		 */
		EdgeAroundVertexCirculator edges(Vertex v) const
		{
		    return EdgeAroundVertexCirculator(this, v);
		}
		//@}


	public: //--------------------------------------------- higher-level operations

		/// \name Higher-level Topological Operations
		//@{

		/**
		 * \brief Returns the valence (number of incident edges or neighboring vertices) of a vertex.
		 * \param v The vertex.
		 * \return The valence of the vertex.
		 */
		unsigned int valence(Vertex v) const;

		/**
		 * \brief Finds the edge connecting two vertices.
		 * \param a The first vertex.
		 * \param b The second vertex.
		 * \return The edge connecting the vertices.
		 */
		Edge find_edge(Vertex a, Vertex b) const;

		/**
		 * \brief Deletes a vertex from the graph.
		 * \param v The vertex to delete.
		 */
		void delete_vertex(Vertex v);

		/**
		 * \brief Deletes an edge from the graph.
		 * \param e The edge to delete.
		 */
		void delete_edge(Edge e);
		//@}


	public: //------------------------------------------ geometry-related functions

		/// \name Geometry-related Functions
		//@{

		/**
		 * \brief Returns the position of a vertex (read-only).
		 * \param v The vertex.
		 * \return The position of the vertex.
		 */
		const vec3& position(Vertex v) const { return vpoint_[v]; }

		/**
		 * \brief Returns the position of a vertex.
		 * \param v The vertex.
		 * \return The position of the vertex.
		 */
		vec3& position(Vertex v) { return vpoint_[v]; }

		/**
		 * \brief Returns the vector of vertex positions (read-only).
		 * \return The vector of vertex positions.
		 */
		const std::vector<vec3>& points() const override { return vpoint_.vector(); }

		/**
		 * \brief Returns the vector of vertex positions.
		 * \return The vector of vertex positions.
		 */
		std::vector<vec3>& points() override { return vpoint_.vector(); }

		/**
		 * \brief Computes the length of an edge.
		 * \param e The edge.
		 * \return The length of the edge.
		 */
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


    /**
     * \brief Output stream support for Graph::Vertex.
     * \param os The output stream.
     * \param v The vertex.
     */
	inline std::ostream& operator<<(std::ostream& os, Graph::Vertex v) {
		return (os << 'v' << v.idx());
	}

    /**
     * \brief Output stream support for Graph::Edge.
     * \param os The output stream.
     * \param e The edge.
     */
	inline std::ostream& operator<<(std::ostream& os, Graph::Edge e) {
		return (os << 'e' << e.idx());
	}

} // namespace easy3d

#endif // EASY3D_CORE_GRAPH_H


