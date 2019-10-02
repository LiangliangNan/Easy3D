// Title:   kdTree.h
// Created: Thu Sep 25 14:29:38 2003
// Authors: Richard Keiser, Oliver Knoll, Mark Pauly, Matthias Zwicker
//
// Copyright (c) 2001, 2002, 2003 Computer Graphics Lab, ETH Zurich
//
// This file is part of the Pointshop3D system.
// See http://www.pointshop3d.com/ for more information.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.
//
// Contact info@pointshop3d.com if any conditions of this
// licensing are not clear to you.
//

#ifndef __kdtree_H_
#define __kdtree_H_

#include "vector2D.h"
#include "vector3D.h"
#include "PriorityQueue.h"
#include <vector>


namespace kdtree  {


#define SQR(x) (x*x)


	typedef MaxPriorityQueue<int, float> PQueue;
	typedef PQueue::Element Neighbour;



	typedef struct kdTreePoint {
		Vector3D	pos;
		int			index;
	} KdTreePoint;



	class KdBoxFace {
	public:
		// v[0], v[1], v[2], and v[3] are in CCW order
		Vector3D v[4];
		float a, b, c, d; // plane defined by the face  ax + by + cz + d = 0

		KdBoxFace( const Vector3D& vert1, const Vector3D& vert2, const Vector3D& vert3, const Vector3D& vert4 )
		{
			Vector3D n;
			v[0] = vert1; v[1] = vert2; v[2] = vert3; v[3] = vert4;
			n = Vector3D::crossProduct( v[1] - v[0], v[3] - v[0] );
			n.normalize();
			a = n.x; b = n.y; c = n.z;
			d = - Vector3D::dotProduct( v[0], n );
		}

		float distanceFromPoint( const Vector3D& vert ) const
		{
			return a * vert.x + b * vert.y + c * vert.z + d;
		}

		bool intersection( Vector3D& vIntersection, const Vector3D* line ) const
		{
			Vector3D dir = line[1] - line[0];
			float t, alpha = a * dir.x + b * dir.y + c * dir.z;
			if( alpha == 0.0f ) return false;
			t = - distanceFromPoint( line[0] ) / alpha;
			if( t < 0.0f || t > 1.0f ) return false;
			vIntersection = line[0] + t * dir;
			return true;
		}
	};

	/**
	* abstract node class
	* base class for leaves and nodes
	*
	* @author Richard Keiser
	* @version 2.0
	*/ 
	class BaseKdNode {
	public:
		Vector3D    m_boundingBoxLowCorner;
		Vector3D	m_boundingBoxHighCorner;
		/**
		* look for the nearest neighbours
		* @param rd 
		*		  the distance of the query position to the node box
		* @param queryPriorityQueue
		*		  a priority queue
		*/
		virtual void queryNode(float rd, PQueue* queryPriorityQueue) = 0;
		virtual void createBoundingBox( Vector3D& lowCorner, Vector3D& highCorner ) = 0;
		virtual void queryLineIntersection(PQueue* queryPriorityQueue) = 0;
		virtual void queryConeIntersection(PQueue* queryPriorityQueue) = 0;

		/**
		* compute distance from point to box
		*
		* @param q 
		*		the point position
		* @param lo
		*		low point of box
		* @param hi
		*		high point of box
		* @return the distance to the box
		*/
		static float computeBoxDistance(const Vector3D &q, const Vector3D &lo, const Vector3D &hi);

		/**
		* compute the maximal distance from point to one of the eight vertices of the box
		*
		* @param q 
		*		the point position
		* @param lo
		*		low point of box
		* @param hi
		*		high point of box
		* @return the maximal distance to the box
		*/
		static float computeBoxMaxDistance(const Vector3D &q, const Vector3D &lo, const Vector3D &hi);

		/**
		* computes the enclosing box of the points
		*
		* @param lowCorner
		*			the low corner of the enclosing box is returned
		* @param hiCorner
		*			the high corner of the enclosing box is returned
		*/
		static void computeEnclosingBoundingBox(unsigned int nOfPositions, const KdTreePoint *points,
			Vector3D &lowCorner, Vector3D &hiCorner);
		/**
		* compute if a line segment intersects the bounding box
		*
		* @param v1, v2 
		*		the end points of the line segment
		* @param lo
		*		low point of box
		* @param hi
		*		high point of box
		* @param tol
		*      the error tolerance of the intersection
		* @return the distance to the box
		*/
		static bool intersectFace( const Vector3D* line, const KdBoxFace& face );
		static bool intersectBox( const Vector3D* line, const Vector3D &lo, const Vector3D &hi, float tol );

		virtual ~BaseKdNode() {}
	};

	/**
	* Node of a kd tree. The node stands for a box in space.
	*
	* @author Richard Keiser
	* @version 2.0
	*/
	class KdNode : public BaseKdNode {
	public:
		/**
		* deletes the children of the node
		*/
		virtual ~KdNode() {
			delete m_children[0];
			delete m_children[1];
			delete[] m_children;
		}

		/**
		* array of child nodes
		*/
		BaseKdNode**    m_children;		
		/**
		* cut value of the splitting plane
		*/
		float			m_cutVal;		
		/*
		* actual dimension of this node
		*/
		unsigned char	m_dim;			

		/**
		* look for the nearest neighbours
		* @param rd 
		*		  the distance of the query position to the node box
		* @param queryPriorityQueue
		*		  a priority queue
		*/
		void queryNode(float rd, PQueue* queryPriorityQueue);
		void createBoundingBox( Vector3D& lowCorner, Vector3D& highCorner );
		void queryLineIntersection(PQueue* queryPriorityQueue);
		void queryConeIntersection(PQueue* queryPriorityQueue);
	};


	/**
	* A leaf node of the tree (bucket node) containing the primitives
	*
	* @author Richard Keiser
	* @version 2.0
	*/
	class KdLeaf : public BaseKdNode {

	public:
		/**
		* the primitives of this leaf
		*/
		KdTreePoint*	m_points;		
		/**
		* the number of elements in this leaf
		*/
		unsigned int	m_nOfElements;

		/**
		* look for the nearest neighbours
		* @param rd 
		*		  the distance of the query position to the node box
		* @param queryPriorityQueue
		*		  a priority queue
		*/
		void queryNode(float rd, PQueue* queryPriorityQueue);
		void createBoundingBox( Vector3D& lowCorner, Vector3D& highCorner );
		void queryLineIntersection(PQueue* queryPriorityQueue);
		void queryConeIntersection(PQueue* queryPriorityQueue);
	};


	/**
	* An efficient k-d tree for 3 dimensions
	* It is very similar to the k-d tree 
	* described by Arya and Mount in ``Algorithms
	*	for fast vector quantization,'' Proc.  of DCC '93: Data Compression
	*	Conference, eds. J. A. Storer and M. Cohn, IEEE Press, 1993, 381-390
	*  and their ANN software library
	*
	* @author Richard Keiser
	* @version 2.0
	*/
	class KdTree
	{
	public:
		/**
		* Creates a k-d tree from the positions
		*
		* @param positions
		*			point positions
		* @param nOfPositions
		*			number of points
		* @param maxBucketSize
		*			number of points per bucket
		*/
		KdTree(const Vector3D *positions, unsigned int nOfPositions, unsigned int maxBucketSize);

		/**
		* Destructor
		*/
		virtual ~KdTree();

		/**
		* look for the nearest neighbours at <code>position</code>
		*
		* @param position
		*			the position of the point to query with
		*/
		void queryPosition(const Vector3D &position);

		/**
		* look for the nearest neighbours with a maximal squared distance <code>maxSqrDistance</code>. 
		* If the set number of neighbours is smaller than the number of neighbours at this maximum distance, 
		* this call is equal to queryPosition but may be faster if the maximum distance is close to the distance 
		* of the nearest neighbour with the largest distance.
		*
		* @param position
		*			the position of the point to query with
		* @param maxSqrDistance
		*			the maximal squared distance of a nearest neighbour
		* @param queryAll
		*          return all points found if <code>queryAll</code> is true, otherwise return K nearest points,
		*          where K is defined by setNOfNeighbours()
		*/
		void queryRange(const Vector3D &position, float maxSqrDistance, bool queryAll = false );

		/**
		* look for the nearest neighbours with a maximal distance <code>maxDistance</code> to line segment
		* defined by v1 and v2. 
		*
		* @param v1, v2
		*			the end points of a line segment
		* @param maxDistance
		*			the maximal distance to the line segment allowed
		* @param toLine
		*          the points found are ordered by the distance to the line if <code>toLine</code> is true
		*          the points found are ordered by the distance to the view point if <code>toLine</code> is false
		* @param queryAll
		*          return all points found if <code>queryAll</code> is true, otherwise return K nearest points,
		*          where K is defined by setNOfNeighbours()
		*/
		void queryLineIntersection( const Vector3D& v1, const Vector3D& v2, float maxDist, 
			bool toLine = true, bool queryAll = false );

		/**
		* look for the nearest neighbours with an cone from $v1$ to $v2$
		* defined by v1 and v2. 
		*
		* @param v1, v2
		*			the end points of a line segment
		* @param maxAngle
		*			the maximal angle in radian allowed between $v1$$P$ and $v1$$v2$, where $P$ is a candidate point
		* @param toLine
		*          the points found are ordered by the distance to the line if <code>toLine</code> is true
		*          the points found are ordered by the distance to the view point if <code>toLine</code> is false
		* @param queryAll
		*          return all points found if <code>queryAll</code> is true, otherwise return K nearest points,
		*          where K is defined by setNOfNeighbours()
		*/
		void queryConeIntersection( const Vector3D& eye, const Vector3D& v1, const Vector3D& v2, float maxAngle,
			bool toLine = true, bool queryAll = false );

		/**
		* set the number of nearest neighbours which have to be looked at for a query
		*
		* @params newNOfNeighbours
		*			the number of nearest neighbours
		*/
		void setNOfNeighbours (const unsigned int newNOfNeighbours);

		/**
		* get the index of the i-th nearest neighbour to the query point
		* i must be smaller than the number of found neighbours
		*
		* @param i
		*			index of the nearest neighbour
		* @return the index of the i-th nearest neighbour
		*/
		inline unsigned int getNeighbourPositionIndex (const unsigned int i);

		/** 
		* get the position of the i-th nearest neighbour
		* i must be smaller than the number of found neighbours
		*
		* @param i
		*			index of the nearest neighbour
		* @return the position of the i-th nearest neighbour
		*/
		//inline Vector3D getNeighbourPosition(const unsigned int i);
		/**
		* get the squared distance of the query point and its i-th nearest neighbour
		* i must be smaller than the number of found neighbours
		*
		* @param i
		*			index of the nearest neighbour
		* @return the squared distance to the i-th nearest neighbour
		*/
		inline float getSquaredDistance (const unsigned int i);

		/**
		* get the number of found neighbours
		* Generally, this is equal to the number of query neighbours
		* except for range queries, where this number may be smaller than the number of query neigbhbours
		*
		* @return the number of found neighbours
		*/
		inline unsigned int getNOfFoundNeighbours();

		/**
		* get the number of query neighbors
		* Generally, this is equal to the number of found neighbours
		* except for range queries, where this number may be larger than the number of found neigbhbours
		*
		* @return the number of querye neighbours
		*/
		inline unsigned int getNOfQueryNeighbours();

	protected:
		/** 
		* creates the tree using the sliding midpoint splitting rule
		* 
		* @param node
		*		  the node to split
		* @param start
		*		  first index in the data array
		* @param end
		*		  last index in the data array
		* @param maximum
		*		  maximum coordinates of the data points
		* @param minimum
		*		  minimum coordinates of the data points
		*/
		void createTree(KdNode &node, int start, int end, Vector3D maximum, Vector3D minimum);


	private:

		KdTreePoint*				m_points;
		//const Vector3D*				m_positions;
		std::vector<Neighbour>  	m_neighbours;
		int							m_bucketSize;
		KdNode*						m_root;
		unsigned int				m_nOfFoundNeighbours,
			m_nOfNeighbours,
			m_nOfPositions;
		PQueue*						m_queryPriorityQueue;
		Vector3D                    m_boundingBoxLowCorner;
		Vector3D	                m_boundingBoxHighCorner;

		// gets the minimum and maximum value of all points at dimension dim
		void getMinMax(KdTreePoint *points, int nOfPoints, int dim, float &min, float &max);
		// splits the points such that on return for all points:
		//		points[0..br1-1] < cutVal
		//		points[br1-1..br2-1] == cutVal
		//		points[br2..nOfPoints-1] > cutVal
		void splitAtMid(KdTreePoint *points, int nOfPoints, int dim, float cutVal, int &br1, int &br2);
		// get the axis aligned bounding box of points
		void getSpread(KdTreePoint* points, int nOfPoints, Vector3D &maximum, Vector3D &minimum);
	};

	inline unsigned int KdTree::getNOfFoundNeighbours() {
		return m_nOfFoundNeighbours;
	}

	inline unsigned int KdTree::getNOfQueryNeighbours() {
		return m_nOfNeighbours;
	}

	inline unsigned int KdTree::getNeighbourPositionIndex(const unsigned int neighbourIndex) {
		return m_neighbours[neighbourIndex].index;
	}

	/*inline Vector3D KdTree::getNeighbourPosition(const unsigned int neighbourIndex) {
	return m_positions[m_neighbours[neighbourIndex].index];
	}*/

	inline float KdTree::getSquaredDistance (const unsigned int neighbourIndex) {
		return m_neighbours[neighbourIndex].weight;
	}


}

#endif

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
