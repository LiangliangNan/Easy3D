// Title:   kdTree.cpp
// Created: Thu Sep 25 14:19:18 2003
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

#include "kdTree.h"
#include <float.h>
#include <stdlib.h>


namespace kdtree  {


#define SWAP_POINTS(a,b) \
	KdTreePoint tmp = points[a];\
	points[a] = points[b];\
	points[b] = tmp;


	// ******************
	// global definitions
	// ******************
	bool     g_queryAll;

	//=====================================================
	// global parameters for range search
	//-----------------------------------------------------
	float    g_queryOffsets[3];
	Vector3D g_queryPosition;
	//=====================================================

	//=====================================================
	// global parameters for line intersection search
	//-----------------------------------------------------
	bool     g_queryToLine;
	Vector3D g_queryLine[2];
	Vector3D g_queryLineDir;
	//-----------------------------------------------------
	// parameters for cylinder intersection
	//-----------------------------------------------------
	float g_queryMaxDist, g_queryMaxSqrDist, g_queryMaxSqrRange;
	//-----------------------------------------------------
	// parameters for cone intersection
	//-----------------------------------------------------
	Vector3D g_queryEye;
	float g_queryMaxCosAngle, g_queryMaxTanAngle, g_queryMinSqrRange;
	//=====================================================

	KdTree::KdTree(const Vector3D *positions, unsigned int nOfPositions, unsigned int maxBucketSize) {
		m_bucketSize			= maxBucketSize;
		m_nOfPositions			= nOfPositions;
		m_points				= new KdTreePoint[nOfPositions];
		m_nOfFoundNeighbours	= 0;
		m_nOfNeighbours			= 0;
		m_queryPriorityQueue	= new PQueue();
		for (unsigned int i=0; i<nOfPositions; i++) {
			m_points[i].pos = positions[i];
			m_points[i].index = i;
		}
		m_root = new KdNode();
		Vector3D maximum, minimum;
		getSpread(m_points, nOfPositions, maximum, minimum);
		createTree(*m_root, 0, nOfPositions, maximum, minimum);
		m_root->createBoundingBox(m_boundingBoxLowCorner, m_boundingBoxHighCorner);
		setNOfNeighbours(1);
	}


	KdTree::~KdTree() {
		delete m_root;
		delete[] m_points;
		delete m_queryPriorityQueue;
	}

	void KdTree::queryPosition(const Vector3D &position) {
		if (m_neighbours.size() == 0) {
			return;
		}
		g_queryAll          =   false;
		g_queryOffsets[0]   =   0.0;
		g_queryOffsets[1]   =   0.0;
		g_queryOffsets[2]   =   0.0;
		m_queryPriorityQueue->init();
		m_queryPriorityQueue->insert(-1, FLT_MAX);
		g_queryPosition     =   position;
		float dist = BaseKdNode::computeBoxDistance(position, m_boundingBoxLowCorner, m_boundingBoxHighCorner);
		m_root->queryNode(dist, m_queryPriorityQueue);

		if (m_queryPriorityQueue->getMax().index == -1) {
			m_queryPriorityQueue->removeMax();
		}

		m_nOfFoundNeighbours = m_queryPriorityQueue->getNofElements();
		if( m_nOfFoundNeighbours > m_nOfNeighbours )
		{
			m_nOfNeighbours = m_nOfFoundNeighbours;
			m_neighbours.resize(m_nOfNeighbours);
		}

		for(int i=m_nOfFoundNeighbours-1; i>=0; i--) {
			m_neighbours[i] = m_queryPriorityQueue->getMax();
			m_queryPriorityQueue->removeMax();
		}
	}

	void KdTree::queryRange(const Vector3D &position, float maxSqrDistance, bool queryAll ) {
		if (m_neighbours.size() == 0) {
			if ( queryAll ) {
				setNOfNeighbours ( 32 );
			} else {
				return;
			}
		}
		g_queryAll          =   queryAll;
		g_queryOffsets[0]   =   0.0;
		g_queryOffsets[1]   =   0.0;
		g_queryOffsets[2]   =   0.0;
		m_queryPriorityQueue->init();
		m_queryPriorityQueue->insert(-1, maxSqrDistance);
		g_queryPosition     =   position;

		float dist = BaseKdNode::computeBoxDistance(position, m_boundingBoxLowCorner, m_boundingBoxHighCorner);	
		m_root->queryNode(dist, m_queryPriorityQueue);

		if (m_queryPriorityQueue->getMax().index == -1) {
			m_queryPriorityQueue->removeMax();
		}

		m_nOfFoundNeighbours = m_queryPriorityQueue->getNofElements();
		if( m_nOfFoundNeighbours > m_nOfNeighbours )
		{
			m_nOfNeighbours = m_nOfFoundNeighbours;
			m_neighbours.resize(m_nOfNeighbours);
		}

		for(int i=m_nOfFoundNeighbours-1; i>=0; i--) {
			m_neighbours[i] = m_queryPriorityQueue->getMax();
			m_queryPriorityQueue->removeMax();
		}
	}

	void KdTree::queryLineIntersection( const Vector3D& v1, const Vector3D& v2, float maxDist, bool toLine, bool queryAll )
	{
		if (m_neighbours.size() == 0) {
			if ( queryAll ) {
				setNOfNeighbours ( 32 );
			} else {
				return;
			}
		}
		g_queryAll          =   queryAll;
		g_queryToLine       =   toLine;
		g_queryMaxDist      =   maxDist;
		g_queryMaxSqrDist   =   maxDist * maxDist;
		g_queryLine[0]      =   v1;
		g_queryLine[1]      =   v2;
		g_queryLineDir      =   v2 - v1;
		g_queryMaxSqrRange  =   g_queryLineDir.getSquaredLength();  // maximal square range
		g_queryLineDir.normalize();
		m_queryPriorityQueue->init();
		m_queryPriorityQueue->insert(-1, FLT_MAX);

		m_root->queryLineIntersection(m_queryPriorityQueue);

		if (m_queryPriorityQueue->getMax().index == -1) {
			m_queryPriorityQueue->removeMax();
		}

		m_nOfFoundNeighbours = m_queryPriorityQueue->getNofElements();
		if( m_nOfFoundNeighbours > m_nOfNeighbours )
		{
			m_nOfNeighbours = m_nOfFoundNeighbours;
			m_neighbours.resize(m_nOfNeighbours);
		}

		for(int i=m_nOfFoundNeighbours-1; i>=0; i--) {
			m_neighbours[i] = m_queryPriorityQueue->getMax();
			m_queryPriorityQueue->removeMax();
		}
	}

	void KdTree::queryConeIntersection( const Vector3D& eye, const Vector3D& v1, const Vector3D& v2, float maxAngle, bool toLine, bool queryAll )
	{
		if (m_neighbours.size() == 0) {
			if ( queryAll ) {
				setNOfNeighbours ( 32 );
			} else {
				return;
			}
		}
		g_queryAll          =   queryAll;
		g_queryToLine       =   toLine;
		g_queryMaxCosAngle  =   cosf(maxAngle);
		g_queryMaxTanAngle  =   tanf(maxAngle);
		g_queryEye          =   eye;
		g_queryLine[0]      =   v1;
		g_queryLine[1]      =   v2;
		g_queryMinSqrRange  =   (v1 - eye).getSquaredLength();      // minimal square range
		g_queryLineDir      =   v2 - eye;
		g_queryMaxSqrRange  =   g_queryLineDir.getSquaredLength();  // maximal square range
		g_queryLineDir.normalize();
		m_queryPriorityQueue->init();
		m_queryPriorityQueue->insert(-1, FLT_MAX);

		m_root->queryConeIntersection(m_queryPriorityQueue);

		if (m_queryPriorityQueue->getMax().index == -1) {
			m_queryPriorityQueue->removeMax();
		}

		m_nOfFoundNeighbours = m_queryPriorityQueue->getNofElements();
		if( m_nOfFoundNeighbours > m_nOfNeighbours )
		{
			m_nOfNeighbours = m_nOfFoundNeighbours;
			m_neighbours.resize(m_nOfNeighbours);
		}

		for(int i=m_nOfFoundNeighbours-1; i>=0; i--) {
			m_neighbours[i] = m_queryPriorityQueue->getMax();
			m_queryPriorityQueue->removeMax();
		}
	}

	void KdTree::setNOfNeighbours (const unsigned int newNOfNeighbours) {
		if (newNOfNeighbours != m_nOfNeighbours) {
			m_nOfNeighbours = newNOfNeighbours;
			m_queryPriorityQueue->setSize(m_nOfNeighbours);
			m_nOfNeighbours = newNOfNeighbours;
			m_neighbours.resize(m_nOfNeighbours);
			m_nOfFoundNeighbours = 0;
		}
	}

	void KdTree::createTree(KdNode &node, int start, int end, Vector3D maximum, Vector3D minimum) {
		int	mid;

		int n = end-start;
		Vector3D diff = maximum - minimum;
		short dim;
		// get longest axe
		if (diff[0] > diff[1]) {
			if (diff[0] > diff[2]) {
				dim = 0;	//x-axe is longest axe
			}
			else {
				dim = 2;	// z-axe is longest axe
			}
		}
		else {
			if (diff[1] > diff[2]) {
				dim = 1;	// y-axe is longest axe
			}
			else {
				dim = 2;	// z-axe is longest axe
			}
		}

		node.m_dim = dim;
		float bestCut = (maximum[dim]+minimum[dim])/2.0;
		float min, max;
		getMinMax(m_points+start, n, dim, min, max);	// find min/max coordinates
		if (bestCut < min)		// slide to min or max as needed
			node.m_cutVal = min;
		else if (bestCut > max)
			node.m_cutVal = max;
		else
			node.m_cutVal = bestCut;

		int br1, br2;
		splitAtMid(m_points+start, n, dim, node.m_cutVal, br1, br2);	// permute points accordingly

		if (bestCut < min) mid = start+1;
		else if (bestCut > max) mid = end-1;
		else if (br1 > n/2.0) mid = start+br1;
		else if (br2 < n/2.0) mid = start+br2;
		else mid = start + (n>>1);

		BaseKdNode** childNodes = new BaseKdNode*[2];
		node.m_children = childNodes;
		if (mid-start <= m_bucketSize) {
			// new leaf
			KdLeaf* leaf = new KdLeaf();
			node.m_children[0] = leaf;
			leaf->m_points = (m_points+start);
			leaf->m_nOfElements = mid-start;
		}
		else {
			// new node
			KdNode* childNode = new KdNode();
			node.m_children[0] = childNode;
			float oldMax = maximum[dim];
			maximum[dim] = node.m_cutVal;
			createTree(*childNode, start, mid, maximum, minimum);
			maximum[dim] = oldMax;
		}

		if (end-mid <= m_bucketSize) {
			// new leaf
			KdLeaf* leaf = new KdLeaf();
			node.m_children[1] = leaf;
			leaf->m_points = (m_points+mid);
			leaf->m_nOfElements = end-mid;
		}
		else {
			// new node
			minimum[dim] = node.m_cutVal;
			KdNode* childNode = new KdNode();
			node.m_children[1] = childNode;
			createTree(*childNode, mid, end, maximum, minimum);
		}
	}

	void KdTree::getSpread(KdTreePoint* points, int nOfPoints, Vector3D &maximum, Vector3D &minimum) {
		Vector3D pos = points->pos;
		maximum = Vector3D(pos[0], pos[1], pos[2]);
		minimum = Vector3D(pos[0], pos[1], pos[2]);
		points++;
		for (int i = 1; i < nOfPoints; i++) {
			pos = points->pos;
			if (pos[0] < minimum[0]) {
				minimum[0] = pos[0];
			}
			if (pos[0] > maximum[0]) {
				maximum[0] = pos[0];
			}
			if (pos[1] < minimum[1]) {
				minimum[1] = pos[1];
			}
			if (pos[1] > maximum[1]) {
				maximum[1] = pos[1];
			}
			if (pos[2] < minimum[2]) {
				minimum[2] = pos[2];
			}
			if (pos[2] > maximum[2]) {
				maximum[2] = pos[2];
			}
			points++;
		}
	}

	void KdTree::getMinMax(KdTreePoint *points, int nOfPoints, int dim, float &min, float &max) {
		min = points->pos[dim];
		max = points->pos[dim];
		points++;
		for (int i=1; i<nOfPoints; i++) {
			if (points->pos[dim] < min) {
				min = points->pos[dim];
			}
			else if (points->pos[dim] > max) {
				max = points->pos[dim];
			}
			points++;
		}
	}


	void KdTree::splitAtMid(KdTreePoint *points, int nOfPoints, int dim, float cutVal, int &br1, int &br2) {
		int l = 0;
		int r = nOfPoints-1;
		for(;;) {				// partition points[0..n-1] about the cut value
			while (l < nOfPoints && points[l].pos[dim] < cutVal) {
				l++;
			}
			while (r >= 0 && points[r].pos[dim] >= cutVal) {
				r--;
			}
			if (l > r) 
				break;
			SWAP_POINTS(l,r);
			l++; 
			r--;
		}
		br1 = l;			// now: points[0..br1-1] < cutVal <= points[br1..n-1]
		r = nOfPoints-1;
		for(;;) {				// partition points[br1..n-1] about cutVal
			while (l < nOfPoints && points[l].pos[dim] <= cutVal) { 
				l++;
			}
			while (r >= br1 && points[r].pos[dim] > cutVal) {
				r--;
			}
			if (l > r) 
				break;
			SWAP_POINTS(l,r);
			l++; 
			r--;
		}
		br2 = l;			// now: points[br1..br2-1] == cutVal < points[br2..n-1]
	}

	void BaseKdNode::computeEnclosingBoundingBox(unsigned int nOfPositions, const KdTreePoint *points,
		Vector3D &lowCorner, Vector3D &hiCorner) {
			Vector3D tmp;
			hiCorner = lowCorner = points[0].pos;
			for (unsigned int i=1; i<nOfPositions; i++) {
				tmp = points[i].pos;
				if (hiCorner[0] < tmp[0]) {
					hiCorner[0] = tmp[0];
				}
				else if (lowCorner[0] > tmp[0]) {
					lowCorner[0] = tmp[0];
				}
				if (hiCorner[1] < tmp[1]) {
					hiCorner[1] = tmp[1];
				}
				else if (lowCorner[1] > tmp[1]) {
					lowCorner[1] = tmp[1];
				}
				if (hiCorner[2] < tmp[2]) {
					hiCorner[2] = tmp[2];
				}
				else if (lowCorner[2] > tmp[2]) {
					lowCorner[2] = tmp[2];
				}
			}		 
	}

	float BaseKdNode::computeBoxDistance(const Vector3D &q, const Vector3D &lo, const Vector3D &hi) {
        float dist = 0.0;
        float t;

		if (q[0] < lo[0]) {
			t = lo[0] - q[0];
			dist = t*t;
		}
		else if (q[0] > hi[0]) {
			t = q[0] - hi[0];
			dist = t*t;
		}
		if (q[1] < lo[1]) {
			t = lo[1] - q[1];
			dist += t*t;
		}
		else if (q[1] > hi[1]) {
			t = q[1] - hi[1];
			dist += t*t;
		}
		if (q[2] < lo[2]) {
			t = lo[2] - q[2];
			dist += t*t;
		}
		else if (q[2] > hi[2]) {
			t = q[2] - hi[2];
			dist += t*t;
		}

		return dist;
	}

	float BaseKdNode::computeBoxMaxDistance(const Vector3D &q, const Vector3D &lo, const Vector3D &hi) {
        float dist;
        float t1, t2;

		t1 = fabsf( q[0] - lo[0] );
		t2 = fabsf( q[0] - hi[0] );
		if( t1 > t2 )
		{
			dist = t1 * t1;
		}
		else
		{
			dist = t2 * t2;
		}
		t1 = fabsf( q[1] - lo[1] );
		t2 = fabsf( q[1] - hi[1] );
		if( t1 > t2 )
		{
			dist += t1 * t1;
		}
		else
		{
			dist += t2 * t2;
		}
		t1 = fabsf( q[2] - lo[2] );
		t2 = fabsf( q[2] - hi[2] );
		if( t1 > t2 )
		{
			dist += t1 * t1;
		}
		else
		{
			dist += t2 * t2;
		}

		return sqrtf(dist);
	}

	bool BaseKdNode::intersectBox( const Vector3D* line, const Vector3D &lo, const Vector3D &hi, float tol )
	{
		//       Z
		//       ^      6           7
		//       |      o----------o
		//       |     /|         /|
		//       |   /          /  |
		//     2 | /    |  3  /    |
		//       o----------o      |
		//       |    4 |   |    5 |
		//       |      o-  | -  - o
		//       |    /     |     /
		//       |          |   /
		//       | /        | /
		//       o----------o----------> X
		//      0          1
		//    /
		//  -Y
		//
		// the bounding box is enlarged by tol. this is to ensure that all points with distance to the
		// line less than tol will be found
		float lox = lo.x - tol, loy = lo.y - tol, loz = lo.z - tol;
		float hix = hi.x + tol, hiy = hi.y + tol, hiz = hi.z + tol;

		Vector3D vert[8] =
		{
			Vector3D( lox, loy, loz ), Vector3D( hix, loy, loz ), Vector3D( lox, loy, hiz ), Vector3D( hix, loy, hiz ),
			Vector3D( lox, hiy, loz ), Vector3D( hix, hiy, loz ), Vector3D( lox, hiy, hiz ), Vector3D( hix, hiy, hiz ),
		};

		KdBoxFace face[6] = 
		{
			KdBoxFace( vert[0], vert[1], vert[3], vert[2] ), KdBoxFace( vert[4], vert[6], vert[7], vert[5] ),
			KdBoxFace( vert[0], vert[2], vert[6], vert[4] ), KdBoxFace( vert[1], vert[5], vert[7], vert[3] ),
			KdBoxFace( vert[2], vert[3], vert[7], vert[6] ), KdBoxFace( vert[0], vert[4], vert[5], vert[1] ),
		};

        int i, j;
		unsigned char bOutside[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		// Check two end points against all 6 face planes, 
		// and store result (1 if outside) in a bitfield
		for( i = 0; i < 2; i++ )
		{
			for( j = 0; j < 6; j++ )
			{
				if( face[j].distanceFromPoint( line[i] ) > 0 )
				{
					bOutside[i] |= (1 << j);
				}
			}
			// If any point is inside all 6 planes, it is inside
			// the box, so the line must intersect the box.
			if( bOutside[i] == 0 ) return true;
		}

		// If both end points are outside any single plane, the line is outside the box
		if( (bOutside[0] & bOutside[1]) != 0 )
		{
			return false;
		}

		// Now have to see if the line penetrates any of the faces of the bounding box
		for( i = 0; i < 6; i++ )
		{
			if( intersectFace( line, face[i] ) )
			{
				return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	// Name: EdgeIntersectsFace()
	// Desc: Determine if the edge bounded by the two vectors in pEdges intersects
	//       the quadrilateral described by the four vectors in pFacePoints.  
	//       Note: pPlane could be derived from pFacePoints using 
	//       D3DXPlaneFromPoints, but it is precomputed in advance for greater
	//       speed.
	//-----------------------------------------------------------------------------
	bool BaseKdNode::intersectFace( const Vector3D* line, const KdBoxFace& face )
	{
		// If both end points are on the same side of the plane, the line does not intersect the face
		float dist1 = face.distanceFromPoint( line[0] ), dist2 = face.distanceFromPoint( line[1] );
		if( ( dist1 > 0 && dist2 > 0 ) || ( dist1 < 0 && dist2 < 0 ) )
		{
			return false;
		}

		// Find point of intersection between line and face plane (if they're
		// parallel, line does not intersect face
		Vector3D vIntersection;
		if( !face.intersection( vIntersection, line ) )
		{
			return false;
		}

		// Project onto a 2D plane to make the pt-in-poly test easier
		float absa = ( face.a > 0 ? face.a : -face.a );
		float absb = ( face.b > 0 ? face.b : -face.b );
		float absc = ( face.c > 0 ? face.c : -face.c );

        int i;
		Vector2D facePoints[4], point;

		if( absa > absb && absa > absc )
		{
			// Plane is mainly pointing along X axis, so use Y and Z
			for( i = 0; i < 4; i++)
			{
				facePoints[i].x = face.v[i].y;
				facePoints[i].y = face.v[i].z;
			}
			point.x = vIntersection.y;
			point.y = vIntersection.z;
		}
		else if( absb > absa && absb > absc )
		{
			// Plane is mainly pointing along Y axis, so use X and Z
			for( i = 0; i < 4; i++)
			{
				facePoints[i].x = face.v[i].x;
				facePoints[i].y = face.v[i].z;
			}
			point.x = vIntersection.x;
			point.y = vIntersection.z;
		}
		else
		{
			// Plane is mainly pointing along Z axis, so use X and Y
			for( i = 0; i < 4; i++)
			{
				facePoints[i].x = face.v[i].x;
				facePoints[i].y = face.v[i].y;
			}
			point.x = vIntersection.x;
			point.y = vIntersection.y;
		}

		// If point is on the outside of any of the face edges, it is
		// outside the face.  
		// We can do this by taking the determinant of the following matrix:
		// | x0 y0 1 |
		// | x1 y1 1 |
		// | x2 y2 1 |
		// where (x0,y0) and (x1,y1) are points on the face edge and (x2,y2) 
		// is our test point.  If this value is positive, the test point is
		// "to the left" of the line.  To determine whether a point needs to
		// be "to the right" or "to the left" of the four lines to qualify as
		// inside the face, we need to see if the faces are specified in 
		// clockwise or counter-clockwise order (it could be either, since the
		// edge could be penetrating from either side).  To determine this, we
		// do the same test to see if the third point is "to the right" or 
		// "to the left" of the line formed by the first two points.
		// See http://forum.swarthmore.edu/dr.math/problems/scott5.31.96.html
		float x0, x1, x2, y0, y1, y2;
		x0 = facePoints[0].x;
		y0 = facePoints[0].y;
		x1 = facePoints[1].x;
		y1 = facePoints[1].y;
		x2 = facePoints[2].x;
		y2 = facePoints[2].y;
		bool bClockwise = false;
		if( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 < 0 )
			bClockwise = true;
		x2 = point.x;
		y2 = point.y;
		for( i = 0; i < 4; i++ )
		{
			x0 = facePoints[i].x;
			y0 = facePoints[i].y;
			if( i < 3 )
			{
				x1 = facePoints[i+1].x;
				y1 = facePoints[i+1].y;
			}
			else
			{
				x1 = facePoints[0].x;
				y1 = facePoints[0].y;
			}
			if( ( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 > 0 ) == bClockwise )
				return false;
		}

		// If we get here, the point is inside all four face edges, 
		// so it's inside the face.
		return true;
	}

	void KdNode::queryNode(float rd, PQueue* queryPriorityQueue) {
        float old_off = g_queryOffsets[m_dim];
        float new_off = g_queryPosition[m_dim] - m_cutVal;
		if (new_off < 0) {
			m_children[0]->queryNode(rd, queryPriorityQueue);
			rd = rd - SQR(old_off) + SQR(new_off);
			if (rd < queryPriorityQueue->getMaxWeight()) {
				g_queryOffsets[m_dim] = new_off;
				m_children[1]->queryNode(rd, queryPriorityQueue);
				g_queryOffsets[m_dim] = old_off;
			}
		}
		else {
			m_children[1]->queryNode(rd, queryPriorityQueue);
			rd = rd - SQR(old_off) + SQR(new_off);
			if (rd < queryPriorityQueue->getMaxWeight()) {
				g_queryOffsets[m_dim] = new_off;
				m_children[0]->queryNode(rd, queryPriorityQueue);
				g_queryOffsets[m_dim] = old_off;
			}
		}
	}

	void KdNode::createBoundingBox( Vector3D& lowCorner, Vector3D& highCorner )
	{
		KdTreePoint points[4];
		Vector3D vlow, vhigh;
		for( unsigned int i = 0; i < 2; i++ ) {
			m_children[i]->createBoundingBox( vlow, vhigh );
			points[2*i].pos = vlow;
			points[2*i+1].pos = vhigh;
		}
		BaseKdNode::computeEnclosingBoundingBox( 4, points, lowCorner, highCorner );
		m_boundingBoxLowCorner = lowCorner;
		m_boundingBoxHighCorner = highCorner;
	}

	void KdNode::queryLineIntersection(PQueue *queryPriorityQueue)
	{
		if( BaseKdNode::intersectBox( g_queryLine, m_boundingBoxLowCorner, m_boundingBoxHighCorner, g_queryMaxDist ) )
		{
			m_children[0]->queryLineIntersection( queryPriorityQueue );
			m_children[1]->queryLineIntersection( queryPriorityQueue );
		}
	}

	void KdNode::queryConeIntersection(PQueue* queryPriorityQueue)
	{
		float fMaxDist;
		fMaxDist = BaseKdNode::computeBoxMaxDistance( g_queryEye, m_boundingBoxLowCorner, m_boundingBoxHighCorner );
		fMaxDist = fMaxDist * g_queryMaxTanAngle; // g_queryMaxValue2 = tan( cone_angle )
		if( BaseKdNode::intersectBox( g_queryLine, m_boundingBoxLowCorner, m_boundingBoxHighCorner, fMaxDist ) )
		{
			m_children[0]->queryConeIntersection( queryPriorityQueue );
			m_children[1]->queryConeIntersection( queryPriorityQueue );
		}
	}

	void KdLeaf::queryNode(float rd, PQueue* queryPriorityQueue) {
		float sqrDist;
		//use pointer arithmetic to speed up the linear traversing
		KdTreePoint* point = m_points;
		for ( unsigned int i=0; i<m_nOfElements; i++) {
			sqrDist = (point->pos - g_queryPosition).getSquaredLength();
			if (sqrDist < queryPriorityQueue->getMaxWeight()) {
				queryPriorityQueue->insert(point->index, sqrDist, g_queryAll);
			}
			point++;
		}		
	}

	void KdLeaf::createBoundingBox( Vector3D& lowCorner, Vector3D& highCorner )
	{
		BaseKdNode::computeEnclosingBoundingBox( m_nOfElements, m_points, lowCorner, highCorner );
		m_boundingBoxLowCorner = lowCorner;
		m_boundingBoxHighCorner = highCorner;
	}

	void KdLeaf::queryLineIntersection(PQueue *queryPriorityQueue)
	{
		if( BaseKdNode::intersectBox( g_queryLine, m_boundingBoxLowCorner, m_boundingBoxHighCorner, g_queryMaxDist ) )
		{
			Vector3D vc;
			float sqrDist, sqrDistLine, sqrDistVert;
			KdTreePoint* point = m_points;
			// check points individually
			for( unsigned int i = 0; i < m_nOfElements; i++ ) {
				vc = point->pos - g_queryLine[0];
				sqrDist = vc.getSquaredLength();
				sqrDistLine = Vector3D::dotProduct( vc, g_queryLineDir );
				sqrDistLine *= sqrDistLine;
				if( sqrDistLine > g_queryMaxSqrRange ) continue;
				sqrDistVert = sqrDist - sqrDistLine;
				if( sqrDistVert < g_queryMaxSqrDist )
				{
					if( g_queryToLine && sqrDistVert < queryPriorityQueue->getMaxWeight() )
					{
						// cloest to line first
						queryPriorityQueue->insert(point->index, sqrDistVert, g_queryAll);
					}
					else if( sqrDistLine < queryPriorityQueue->getMaxWeight() )
					{
						// cloest to eye first
						queryPriorityQueue->insert(point->index, sqrDistLine, g_queryAll);
					}
				}
				point++;
			}
		}
	}

	void KdLeaf::queryConeIntersection(PQueue* queryPriorityQueue)
	{
		float fMaxDist;
		fMaxDist = BaseKdNode::computeBoxMaxDistance( g_queryEye, m_boundingBoxLowCorner, m_boundingBoxHighCorner );
		fMaxDist = fMaxDist * g_queryMaxTanAngle;
		if( BaseKdNode::intersectBox( g_queryLine, m_boundingBoxLowCorner, m_boundingBoxHighCorner, fMaxDist ) )
		{
			Vector3D vc;
			float sqrDist, distLine, sqrDistVert, cosAngle;
			KdTreePoint* point = m_points;
			// check points individually
			for( unsigned int i = 0; i < m_nOfElements; i++ ) {
				vc = point->pos - g_queryEye;
				sqrDist = vc.getSquaredLength();
				if( sqrDist < g_queryMinSqrRange ) continue;
				if( sqrDist > g_queryMaxSqrRange ) continue;

				distLine = Vector3D::dotProduct( vc, g_queryLineDir );
				cosAngle =  distLine / sqrtf(sqrDist);
				if( cosAngle > g_queryMaxCosAngle )
				{
					if( g_queryToLine )
					{
						// cloest to line first
						sqrDistVert = sqrDist - distLine * distLine;
						if( sqrDistVert < queryPriorityQueue->getMaxWeight() )
						{
							queryPriorityQueue->insert(point->index, sqrDistVert, g_queryAll);
						}
					}
					else if( sqrDist < queryPriorityQueue->getMaxWeight() )
					{
						// cloest to eye first
						queryPriorityQueue->insert(point->index, sqrDist, g_queryAll);
					}
				}
				point++;
			}
		}
	}

}

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
