// Title:   QueryGrid.h
// Created: Thu Sep 25 14:29:55 2003
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

#ifndef __QUERYGRID_H_
#define __QUERYGRID_H_

#include "Vector3D.h"
#include "PriorityQueue.h"


namespace kdtree  {

	/**
	* A dynamic structure for distance and nearest neighbor query in 3D using a grid
	* 
	* code is used from "Fast Surface Particle Repulsion" by Paul S. Heckbert (1997)
	*
	* @author Richard Keiser
	* @version 1.2
	*/
	template<class Obj>
	class QueryGrid {
	public:

		/**
		* a grid index consists of an object and its position
		*/
		typedef struct index {
			Obj*		obj;
			Vector3D	position;
		} Index;

		/**
		* a grid element consists of an <code>Index</code> and a pointer to another element
		*/
		typedef struct element {
			Index			index;
			struct element	*next;
		} Element;


	public:
		/**
		* Initializes the query grid
		*
		* @param nOfBits
		*			2^(nOfBits*3) cells
		* @param diam
		*			diameter of the sphere enclosing all objects
		*/
		QueryGrid(int nOfBits, float diam);
		/**
		* Destructor
		*/
		~QueryGrid();
		/**
		* set the number of nearest neighbours which have to be looked at for a query
		* For range queries, this is an upper number of neighbours inside the specified range
		*
		* @param newNOfNeighbours
		*			the number of nearest neighbours
		*/
		void setNOfNeighbours (const unsigned int nOfNeighbours);
		/**
		* get the index of the i-th nearest neighbour to the query point
		* i must be smaller than the number of found neighbours
		*
		* @param i
		*			index of the nearest neighbour
		* @return the i-th nearest neighbour
		*/
		inline Obj* getNeighbourObject (const unsigned int i);
		/** 
		* get the position of the i-th nearest neighbour
		* i must be smaller than the number of found neighbours
		*
		* @param i
		*			index of the nearest neighbour
		* @return the position of the i-th nearest neighbour
		*/
		inline Vector3D getNeighbourPosition(const unsigned int i);
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
		* statistics for resuiting the cell sizes if necessary
		* calls regrid if cubes are wrong size/number
		*
		* @param diam
		*			diameter of object cloud, in world space
		* @param rAvg
		*			average radius of queries, in world space
		*/
		void queryAdvisory(float diam, float rAvg);
		/**
		* statistics for resuiting the cell sizes if necessary
		* calls regrid if cubes are wrong size/number
		*
		* @param rAvg
		*			average radius of queries, in world space
		*/
		void queryAdvisoryAverageQueryRadius(float rAvg);
		/**
		* statistics for resuiting the cell sizes if necessary
		* calls regrid if cubes are wrong size/number
		*
		* @param diam
		*			diameter of object cloud, in world space
		*/
		void queryAdvisoryDiameter(float diam);
		/**
		* return m_bucket given bounded integer indices
		* each index must be in the range 0..m_mask
		*
		* @return List of <code>Element</code>s if the m_bucket is not empty, NULL otherwise
		*/
		inline Element** getBucket(int x, int y, int z) const {
			return &m_bucket[(( (x<<m_nOfBits) | y) <<m_nOfBits) | z];
		}
		/**
		* return m_bucket given bounded integer indices
		*
		* @return List of <code>Element</code>s if the m_bucket is not empty, NULL otherwise
		*/
		inline Element** getBucket(const Vector3D pos) const{
			return getBucket((int)floor(pos[0]/m_cubeSize) & m_mask, (int)floor(pos[1]/m_cubeSize) & m_mask, (int)floor(pos[2]/m_cubeSize) & m_mask);
		}

		/**
		* add object to set
		* 
		* @param obj
		*			the new object to add to the right cell
		*/
		inline void insert(Obj* obj, const Vector3D &position);
		/**
		* add a list of objects to set
		* 
		* @param obj
		*			the object lists
		* @param nOfObjects
		*			the number of objects
		*/
		void insert(Obj* *oList, Vector3D *positionList, const unsigned int nOfObjects);
		/**
		* removes an object from the set
		*
		* @param obj
		*			the object to remove form the corresponding cell
		*/
		inline void remove(Obj* obj, const Vector3D &position);
		/**
		* deletes all objects from the set
		*/
		void clear();
		/**
		* update particle's position to new position
		*
		* Removes the object from the old cell and adds it to a new cell corresponding to newPos.
		*
		* @param obj
		*		the object to move
		* @param newPos
		*		the new position of <code>obj</code>
		*/
		void move(Obj* obj, const Vector3D &oldPos, const Vector3D &newPos);
		/** 
		* queries for all objects within radius maxDist of position pos
		*
		* @param pos
		*			the query position
		* @param maxDist
		*			the radius of the query
		*/
		void queryRange(const Vector3D &pos, const float maxDist);
		/**
		* Get the object at position <code>pos</code>. If there is no object at this position, NULL is returned.
		*
		* @param pos
		*			the position of the object
		* @return the object or NULL if no object exists.
		*/
		Obj* getObject(const Vector3D &pos);

		/**
		* initialize statistics
		*/
		void initStats();

		/**
		* gets the statistics
		*/
		void getStats(int &nsingle, int &nwrap, int &ncube, int &nempty) const;

	protected:
		/**
		* change number of bits and cubesize, rebuild datastructure
		*/
		void regrid(int nOfBits, float size);
		/**
		* returns the number of buckets in x, y, or z
		*/
		inline int getNOfBucketsPerAxis() const {
			return 1<<m_nOfBits;
		}
		/**
		* returns the total number of buckets
		*/
		inline int getNOfBuckets() const {
			return 1<<(3*m_nOfBits);
		}
		/**
		* returns the size of the entire cubical space
		*/
		inline float getSize() const {
			return getNOfBucketsPerAxis()*m_cubeSize;
		}
		/**
		* inserts an element
		*/
		inline void insert(Element* e);

	private:
		typedef MaxPriorityQueue<Index*, float> PQueue;
		typedef typename PQueue::Element Neighbour;


		Element				**m_bucket;			// array of cubical hash m_bucket; each contains a linked list of particles
		int					m_nOfBits;			// number of bits per dimension for hash table
		int					m_mask;				// for hashing, m_mask = 2^nbit - 1
		float				m_cubeSize;			// size of a cube
		float				m_rAvg;				// average radius of queries, in world space
		float				m_diam;				// diameter of object cloud, in world space

		int					m_nOfObjects;		// number of objects

		Neighbour			*m_neighbours;		// the queried neighbors
		unsigned int		m_nOfNeighbours;	// the maximum number of query neighbors
		unsigned int		m_nOfFoundNeighbours;	// the found number of query neighbors
		PQueue				*m_queryPriorityQueue;

		// statistics for testing
		int					m_nmove_attempt;	// number of calls to move()
		int					m_nmove_actual;		// number of particles actually moved
		int					m_nqsingle, m_nqwrap, m_ncrange, m_ncempty;	// queryRange and queryPosition stats
		// testing queryAdvisory
		int					m_manualoverride;

		//perform the range query
		void doRangeQuery(const Vector3D &pos, const float maxDist);


	};

	template<class Obj>
	QueryGrid<Obj>::QueryGrid(int nOfBits, float diam) {
		int			nOfBuckets,
			i;


		m_nOfBits = nOfBits;
		m_mask = getNOfBucketsPerAxis() - 1;
		nOfBuckets = getNOfBuckets();
		m_bucket = new Element*[nOfBuckets];
		for (i=0; i<nOfBuckets; i++) {
			m_bucket[i] = NULL;
		}
		m_cubeSize = diam / getNOfBucketsPerAxis();
		m_diam = diam;
		m_nOfObjects = 0;
		m_nOfNeighbours = 0;
		m_nOfFoundNeighbours = 0;
		m_neighbours = NULL;
		m_queryPriorityQueue = new PQueue();
		m_rAvg = -1;

		m_manualoverride = 0;
		m_nmove_attempt = 0;
		m_nmove_actual = 0;
	}

	template<class Obj>
	QueryGrid<Obj>::~QueryGrid() {
		delete m_queryPriorityQueue;
		clear();
		delete[] m_bucket;
	}

	template<class Obj>
	void QueryGrid<Obj>::clear() {
		int i;
		int nOfBuckets = getNOfBuckets();
		Element *e, *next;
		for (i=0; i<nOfBuckets; i++) {
			e=m_bucket[i];
			while (e) {
				next = e->next;
				delete e;
				e = next;
			}
			m_bucket[i] = NULL;
		}
	}

	template<class Obj>
	void QueryGrid<Obj>::queryAdvisory(float diam, float rAvg) {
		m_rAvg = rAvg;
		m_diam = diam;
		float s = getSize();
		int n = m_nOfBits;
		rAvg /= m_cubeSize;


		// if queries too big or too small, then grow or shrink cubesize respectively,
		// sqrt(a*b) is the geometric mean of the numbers a and b
		if (rAvg>0 && (rAvg < 0.7f || rAvg > 1.0f)) {
			s *= (float)(rAvg / sqrt(.7f*1.0f));
		}

		// if points only take up a fraction of the space, use fewer buckets
		if (m_diam < 0.5f*s && n > 0) {
			n--;
			s /= 2.0f;
		}
		// if points wrap around use more buckets
		else if (m_diam > 1.2f*s && n<6) {
			n++;
			s *= 2.0f;
		}

		if (n != m_nOfBits || s != getSize()) {
			regrid(n, s);
		}

	}

	template<class Obj>
	void QueryGrid<Obj>::queryAdvisoryAverageQueryRadius(float rAvg) {
		queryAdvisory(m_diam, rAvg);
	}

	template<class Obj>
	void QueryGrid<Obj>::queryAdvisoryDiameter(float diam) {
		queryAdvisory(diam, m_rAvg);
	}


	template<class Obj>
	void QueryGrid<Obj>::regrid(int nOfBits, float size) {
		if (m_cubeSize == size / getNOfBucketsPerAxis() && m_nOfBits == nOfBits) {
			return;
		}

		// create a linked list of all objects in a set, empty the buckets
		Element *first = NULL, *p, *q;
		int i;
		int nOfBuckets = getNOfBuckets();
		for (i=0; i<nOfBuckets; i++) {
			p = m_bucket[i];
			if (p) {
				// find end of m_bucket's list
				for (q=p; q->next; q=q->next); 
				q->next = first;
				first = p;
			}
		}

		if (m_nOfBits != nOfBits) {
			delete[] m_bucket;
			m_nOfBits = nOfBits;
			nOfBuckets = getNOfBuckets();
			m_mask = getNOfBucketsPerAxis() - 1;
			m_bucket = new Element*[nOfBuckets];
		}

		for (i=0; i<nOfBuckets; i++) {
			m_bucket[i] = NULL;		// empty buckets
		}

		m_nOfObjects = 0;
		m_cubeSize = size / getNOfBucketsPerAxis();

		//redistribute objects to new cells
		for (p=first; p; p=q) {
			q = p->next;
			insert(p);
		}

	}

	template<class Obj>
	inline void QueryGrid<Obj>::insert(Obj* obj, const Vector3D &position) {
		Element **b;
		b = getBucket(position);
		Element *e = new Element;
		e->index.obj = obj;
		e->index.position = position;
		e->next = *b;	
		*b = e;
		m_nOfObjects++;

	}

	template<class Obj>
	inline void QueryGrid<Obj>::insert(Element* e) {
		Element **b;
		b = getBucket(e->index.position);
		e->next = *b;	
		*b = e;
		m_nOfObjects++;
	}

	template<class Obj>
	void QueryGrid<Obj>::insert(Obj* *oList, Vector3D *vecList, const unsigned int nOfObjects) {
		for (unsigned int i=0; i<nOfObjects; i++) {
			insert(oList[i], vecList[i]);
		}
	}

	template<class Obj>
	inline void QueryGrid<Obj>::remove(Obj* obj, const Vector3D &position) {
		Element **b, *e, *e0;
		b = getBucket(position);
		if ((*b)->index.obj == obj) {
			e0 = *b;
			*b = e0->next;
			delete e0;
		}
		else {
			for (e=*b; e->next && e->next->index.obj != obj; e=e->next);	// find predecessor to obj
			ASSERT(e);
			e0 = e->next;
			e->next = e->next->next;
			delete e0;
		}
		m_nOfObjects--;
	}


	template<class Obj>
	void QueryGrid<Obj>::move(Obj* obj, const Vector3D &oldPos, const Vector3D &newPos) {
		if (!obj) {
			return;
		}
		m_nmove_attempt++;
		Element **b0, **bn;
		Element *eo;							// element containing obj;
		b0 = getBucket(oldPos);	// old m_bucket
		if (!*b0) {
			return;						// no elements
		}
		bn = getBucket(newPos);			// new m_bucket
		if (bn == b0) {
			Element *e;
			for (e=*b0; e && (e->index.obj != obj); e=e->next); // find bucket of p
			if (e) {
				e->index.position = newPos;
			}
			return;
		}
		// delete Element containing obj from b0's list
		if ((*b0)->index.obj == obj) {
			eo = *b0;
			*b0 = eo->next;
		}
		else {
			Element *e;
			for (e=*b0; e->next && e->next->index.obj != obj; e=e->next); // find predecessor (e->next) to obj
			ASSERT(e->next);	// e->next == NULL -> obj is not in the list
			eo = e->next;
			e->next = e->next->next;	// leave the element with obj out;
		}

		// insert eo into m_bucket bn's list
		eo->next = *bn;
		eo->index.position = newPos;
		*bn = eo;
		m_nmove_actual++;

	}

	template<class Obj>
	Obj* QueryGrid<Obj>::getObject(const Vector3D &pos) {

		Element **b0;			// element containing obj;
		Element *e;

		b0 = getBucket(pos);		
		for (e=*b0; e; e=e->next) {
			if (e->index.position[0] == pos[0] && e->index.position[1] == pos[1] && e->index.position[2] == pos[2]) {
				return e->index.obj;
			}
		}
		return NULL;
	}

	template<class Obj>
	void QueryGrid<Obj>::queryRange(const Vector3D &pos, const float maxDist) {
		if (!m_neighbours) {
			return;
		}
		m_queryPriorityQueue->init();
		m_queryPriorityQueue->insert(NULL, maxDist*maxDist);

		doRangeQuery(pos, maxDist);
		m_nOfFoundNeighbours = 0;

		int cnt = m_queryPriorityQueue->getNofElements() - 1;

		while(cnt >= 0) {
			m_neighbours[cnt] = m_queryPriorityQueue->getMax();
			if (m_neighbours[cnt].index) {
				m_nOfFoundNeighbours++;
			}
			m_queryPriorityQueue->removeMax();
			cnt--;
		}
	}

	template<class Obj>
	void QueryGrid<Obj>::doRangeQuery(const Vector3D &pos, const float maxDist) {
		float maxSqrDist = maxDist*maxDist;
		float sqrDist;
		// indices of cube containing point pos
		int xc = (int)floor(pos[0]/m_cubeSize);
		int yc = (int)floor(pos[1]/m_cubeSize);
		int zc = (int)floor(pos[2]/m_cubeSize);
		// find bounding box of sphere of radius r around pos
		int x0 = (int)floor((pos[0]-maxDist)/m_cubeSize);
		int y0 = (int)floor((pos[1]-maxDist)/m_cubeSize);
		int z0 = (int)floor((pos[2]-maxDist)/m_cubeSize);
		int x1 = (int)floor((pos[0]+maxDist)/m_cubeSize);
		int y1 = (int)floor((pos[1]+maxDist)/m_cubeSize);
		int z1 = (int)floor((pos[2]+maxDist)/m_cubeSize);

		int size = x1-x0;
		if (y1-y0 > size) {
			size = y1-y0;
		}
		if (z1-z0 > size) {
			size = z1-z0;
		}
		size++;	//largest dimension of parallelepiped of cubes

		// 1. sphere fits in 1 cube
		if (size==1) {
			Element *e = *getBucket(x0 & m_mask, y0 & m_mask, z0 & m_mask);
			for(; e; e=e->next) {
				sqrDist = (e->index.position - pos).getSquaredLength();
				if (sqrDist < m_queryPriorityQueue->getMaxWeight()) {
					m_queryPriorityQueue->insert(&e->index, sqrDist);
				}
			}
			m_nqsingle++;
			m_ncrange++;
		}
		// 2. sphere wraps around!
		else if (size > getNOfBucketsPerAxis()) {
			// visit every object
			// (we do not use the usual sphere scan conversion method because wraparound could cause that to visit some m_bucket repeatedly)
			int i;
			int nOfBuckets = getNOfBuckets();
			Element *e;
			for (i=0; i<nOfBuckets; i++) {
				for (e=m_bucket[i]; e; e=e->next) {
					sqrDist = (e->index.position - pos).getSquaredLength();
					if (sqrDist < m_queryPriorityQueue->getMaxWeight()) {
						m_queryPriorityQueue->insert(&e->index, sqrDist);
					}
				}
			}
			m_nqwrap++;
			m_ncrange += nOfBuckets;
		}
		// 3. sphere intersects >1 cube
		else {
			// visit all cubes within bounding box
			int x, y, z;
			float dx,dy, dz, r2_x2_y2;
			Element *e;
			// find vector (dx, dy, dz) to point of cube (x, y, z) nearest to p
			for (x=x0; x<=x1; x++) {			
				if (x<xc) {
					dx = (x+1)*m_cubeSize - pos[0];
				}
				else if (x>xc) {
					dx = x*m_cubeSize - pos[0];
				}
				else {
					dx = 0;
				}

				for (y=y0; y<=y1; y++) {			
					if (y<yc) {
						dy = (y+1)*m_cubeSize - pos[1];
					}
					else if (y>yc) {
						dy = y*m_cubeSize - pos[1];
					}
					else {
						dy = 0;
					}

					r2_x2_y2 = maxSqrDist - dx*dx - dy*dy;
					if (r2_x2_y2 < 0) {
						continue;
					}

					for (z=z0; z<=z1; z++) {			
						if (z<zc) {
							dz = (z+1)*m_cubeSize - pos[2];
						}
						else if (z>zc) {
							dz = z*m_cubeSize - pos[2];
						}
						else {
							dz = 0;
						}

						// skip cubes that do not intersect sphere of radius r
						if (dz*dz < r2_x2_y2) {
							e = *getBucket(x & m_mask, y & m_mask, z & m_mask);
							if (!e) {
								m_ncempty++;
							}
							for (; e; e=e->next) {
								sqrDist = (e->index.position - pos).getSquaredLength();
								if (sqrDist < m_queryPriorityQueue->getMaxWeight()) {
									m_queryPriorityQueue->insert(&e->index, sqrDist);
								}
							}
							m_ncrange++;
						}
					}
				}
			}
		}
	}

	template<class Obj>
	void QueryGrid<Obj>::initStats() {
		m_nqsingle = 0;
		m_nqwrap = 0;
		m_ncrange = 0;
		m_ncempty = 0;
	}

	template<class Obj>
	void QueryGrid<Obj>::getStats(int &nsingle, int &nwrap, int &ncube, int &nempty) const {
		nqsingle  = m_nsqsingle;
		nwrap = m_nwrap;
		ncube = m_ncube;
		nemtpy = m_nempty;
	}

	template<class Obj>
	void QueryGrid<Obj>::setNOfNeighbours (const unsigned int nOfNeighbours) {
		if (nOfNeighbours != m_nOfNeighbours) {
			m_nOfNeighbours = nOfNeighbours;
			if (m_neighbours) {
				delete m_neighbours;
			}
			m_queryPriorityQueue->setSize(m_nOfNeighbours);
			m_nOfNeighbours = nOfNeighbours;
			m_neighbours = new Neighbour[m_nOfNeighbours];
			m_nOfFoundNeighbours = 0;
		}
	}

	template<class Obj>
	inline Obj* QueryGrid<Obj>::getNeighbourObject (const unsigned int i) {
		return m_neighbours[i].index->obj;
	}

	template<class Obj>
	inline Vector3D QueryGrid<Obj>::getNeighbourPosition(const unsigned int i) {
		return m_neighbours[i].index->position;	
	}

	template<class Obj>
	inline float QueryGrid<Obj>::getSquaredDistance (const unsigned int i) {
		return m_neighbours[i].weight;
	}

	template<class Obj>
	inline unsigned int QueryGrid<Obj>::getNOfFoundNeighbours() {
		return m_nOfFoundNeighbours;
	}

}


#endif
// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
