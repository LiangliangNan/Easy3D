// Title:   PriorityQueue.h
// Created: Thu Sep 25 14:29:57 2003
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

#ifndef __PRIORITYQUEUE_H_
#define __PRIORITYQUEUE_H_

#include <vector>
#include <assert.h>


namespace kdtree  {


	/**
	* Fix sized maximum priority queue 
	*
	* @author Richard Keiser
	* @version 2.0
	*/
	template<class Index, class Weight>
	class MaxPriorityQueue {


	public:
		typedef struct el{
			Index	index;
			Weight weight;
		} Element;

		/** 
		* set maximum size of queue
		*
		* @param size
		*		  size of the queue
		*/
		void setSize(int size) {
			m_queue.resize(size+1);
			m_nOfElements = size;
			init();
		}

		/**
		* initialize queue with zero elements
		*/
		inline void init() {
			m_current = 0;
		}

		/** 
		* returns true if queue is empty
		*/
		inline bool isEmpty() {
			return m_current == 0;
		}

		/** 
		* returns true if queue is full
		*/
		inline bool isFull() {
			return m_current==(m_nOfElements);
		}

		/** 
		* returns number of elements in queue
		*/
		inline int getNofElements() {
			return m_current;
		}

		/**
		* inserts a new element in O(log n)
		* An element consists of an index and a weight
		*/
		inline void insert(Index index, Weight weight, bool expand = false ) {
			if( expand && isFull() ) {
				int szadd = m_nOfElements / 2;
				if( 0 == szadd ) szadd = 1;
				m_nOfElements += szadd;
				m_queue.resize( m_nOfElements + 1 );
			}
			if (isFull()) {
				// if full replace max
				m_queue[1].index = index;
				m_queue[1].weight = weight;
				restore(1, m_nOfElements);
			} 
			else {
				m_current++;
				m_queue[m_current].index = index;
				m_queue[m_current].weight = weight;
                int i=m_current;
				while(i>1 && (m_queue[i].weight > m_queue[i>>1].weight)) {
					swapElements(i, i>>1);
					i >>= 1;
				}
			}
		}

		/**
		* gets the element with maximal weight in O(1)
		*/
		inline Element getMax() {
			assert(!isEmpty());
			return m_queue[1];
		}

		/**
		* gets the index of the element with maximal weight in O(1)
		*/
		inline Index getMaxIndex() {
			assert(!isEmpty());
			return m_queue[1].index;
		}

		/** 
		* gets the maximal weight in O(1)
		*/
		inline Weight getMaxWeight() {
			assert(!isEmpty());
			return m_queue[1].weight;
		}

		/**
		* deletes the element with maximal weight in O(log n)
		*/
		inline void removeMax() {
			assert(!isEmpty());
			m_queue[1] = m_queue[m_current];
			restore(1, m_current);
			m_current--;
		}

		/**
		* gets all elements in the queue
		*/
		inline Element* getQueue() {
			return &(*(m_queue.begin() + 1));
		}


	protected:
		inline void restore(int L, int R) {
            int i, j;
			i = L;
			while (i <= (R>>1)) {
				if( 2*i < R && m_queue[2*i+1].weight > m_queue[2*i].weight) {
					j = 2*i+1;
				}
				else {
					j = 2*i;
				}
				if (m_queue[j].weight > m_queue[i].weight) {
					swapElements(j, i);
					swap(&i, &j);

				}
				else {
					i = R;
				}
			}
		}

	private:
		std::vector<Element> m_queue;
		int m_nOfElements;
		int m_current;

		inline void swapElements(const int i, const int j) {
			Element tmp;
			tmp = m_queue[i];
			m_queue[i] = m_queue[j];
			m_queue[j] = tmp;
		}

		inline void swap(int *i, int *j) {
			int tmp = *i;
			*i = *j;
			*j = tmp;
		}

	};

	/**
	* Fix sized minimum priority queue 
	*
	* @author Richard Keiser
	* @version 2.0
	*/
	template<class Index, class Weight>
	class MinPriorityQueue {


	public:
		typedef struct el{
			Index	index;
			Weight	weight;
		} Element;

		/** 
		* set maximum size of queue
		*
		* @param size
		*		  size of the queue
		*/
		void setSize(int size) {
			m_queue.resize(size+1);
			m_nOfElements = size;
			init();
		}

		/**
		* initialize queue with zero elements
		*/
		inline void init() {
			m_current = 0;
		}

		/** 
		* returns true if queue is empty
		*/
		inline bool isEmpty() {
			return m_current == 0;
		}

		/** 
		* returns true if queue is full
		*/
		inline bool isFull() {
			return m_current==(m_nOfElements);
		}

		/** 
		* returns number of elements in queue
		*/
		inline int getNofElements() {
			return m_current;
		}

		/**
		* inserts a new element in O(log n)
		* An element consists of an index and a weight
		*/
		inline void insert(Index index, Weight weight, bool expand = false) {
			if( expand && isFull() ) {
				int szadd = m_nOfElements / 2;
				if( 0 == szadd ) szadd = 1;
				m_nOfElements += szadd;
				m_queue.resize( m_nOfElements + 1 );
			}
			if (isFull()) {
				// if full replace max
				m_queue[1].index = index;
				m_queue[1].weight = weight;
				restore(1, m_nOfElements);
			} 
			else {
				m_current++;
				m_queue[m_current].index = index;
				m_queue[m_current].weight = weight;
                int i=m_current;
				while(i>1 && (m_queue[i].weight < m_queue[i>>1].weight)) {
					swapElements(i, i>>1);
					i >>= 1;
				}
			}
		}

		/**
		* gets the element with minimal weight in O(1)
		*/
		inline Element getMin() {
			assert(!isEmpty());
			return m_queue[1];
		}

		/**
		* gets the index of the element with minimum weight in O(1)
		*/
		inline Index getMinIndex() {
			assert(!isEmpty());
			return m_queue[1].index;
		}

		/** 
		* gets the minimal weight in O(1)
		*/
		inline Weight getMinWeight() {
			assert(!isEmpty());
			return m_queue[1].weight;
		}

		/**
		* deletes the element with minimal weight in O(log n)
		*/
		inline void removeMin() {
			assert(!isEmpty());
			m_queue[1] = m_queue[m_current];
			restore(1, m_current);
			m_current--;
		}

		/**
		* gets the element in the queue
		*/
		inline Element* getQueue() {
			return &(*(m_queue.begin() + 1));
		}


	protected:
		inline void restore(int L, int R) {
            int i, j;
			i = L;
			while (i <= (R>>1)) {
				if( 2*i < R && m_queue[2*i+1].weight < m_queue[2*i].weight) {
					j = 2*i+1;
				}
				else {
					j = 2*i;
				}
				if (m_queue[j].weight < m_queue[i].weight) {
					swapElements(j, i);
					swap(&i, &j);

				}
				else {
					i = R;
				}
			}
		}

	private:
		std::vector<Element> m_queue;
		int m_nOfElements;
		int m_current;

		inline void swapElements(const int i, const int j) {
			Element tmp;
			tmp = m_queue[i];
			m_queue[i] = m_queue[j];
			m_queue[j] = tmp;
		}

		inline void swap(int *i, int *j) {
			int tmp = *i;
			*i = *j;
			*j = tmp;
		}

	};


}

#endif
// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
