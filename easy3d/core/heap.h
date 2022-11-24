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

#ifndef EASY3D_CORE_HEAP_H
#define EASY3D_CORE_HEAP_H

#include <vector>

namespace easy3d {

    /**
     * \brief A class implementing a heap.
     * \class Heap easy3d/core/heap.h
     *
     * An example of heap interface
     * \code
     *   class HeapInterface {
     *   public:
     *       HeapInterface(SurfaceMesh::VertexProperty<float> prio, SurfaceMesh::VertexProperty<int> pos)
     *               : prio_(prio), pos_(pos) {
     *       }
     *
     *       bool less(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) { return prio_[v0] < prio_[v1]; }
     *       bool greater(SurfaceMesh::Vertex v0, SurfaceMesh::Vertex v1) { return prio_[v0] > prio_[v1]; }
     *       int get_heap_position(SurfaceMesh::Vertex v) { return pos_[v]; }
     *       void set_heap_position(SurfaceMesh::Vertex v, int pos) { pos_[v] = pos; }
     *
     *   private:
     *       SurfaceMesh::VertexProperty<float> prio_;
     *       SurfaceMesh::VertexProperty<int> pos_;
     *   };
     * \endcode
     */

    template<class HeapEntry, class HeapInterface>
    class Heap : private std::vector<HeapEntry> {
    public:
        typedef Heap<HeapEntry, HeapInterface> This;

        //! \brief Constructor
        Heap() : HeapVector() {}

        //! \brief Construct with a given \c HeapInterface.
        explicit Heap(const HeapInterface &i) : HeapVector(), interface_(i) {}

        //! \brief Destructor.
        ~Heap() = default;

        //! \brief clear the heap
        void clear() { HeapVector::clear(); }

        //! \brief is heap empty?
        bool empty() { return HeapVector::empty(); }

        //! \brief returns the size of heap
        unsigned int size() { return (unsigned int) HeapVector::size(); }

        //! \brief reserve space for N entries
        void reserve(unsigned int n) { HeapVector::reserve(n); }

        //! \brief reset heap position to -1 (not in heap)
        void reset_heap_position(HeapEntry h) {
            interface_.set_heap_position(h, -1);
        }

        //! \brief is an entry in the heap?
        bool is_stored(HeapEntry h) {
            return interface_.get_heap_position(h) != -1;
        }

        //! \brief insert the entry h
        void insert(HeapEntry h) {
            This::push_back(h);
            upheap(size() - 1);
        }

        //! \brief get the first entry
        HeapEntry front() {
            assert(!empty());
            return entry(0);
        }

        //! \brief delete the first entry
        void pop_front() {
            assert(!empty());
            interface_.set_heap_position(entry(0), -1);
            if (size() > 1) {
                entry(0, entry(size() - 1));
                HeapVector::resize(size() - 1);
                downheap(0);
            } else
                HeapVector::resize(size() - 1);
        }

        //! \brief remove an entry
        void remove(HeapEntry h) {
            int pos = interface_.get_heap_position(h);
            interface_.set_heap_position(h, -1);

            assert(pos != -1);
            assert((unsigned int) pos < size());

            // last item ?
            if ((unsigned int) pos == size() - 1)
                HeapVector::resize(size() - 1);

            else {
                entry(pos, entry(size() - 1)); // move last elem to pos
                HeapVector::resize(size() - 1);
                downheap(pos);
                upheap(pos);
            }
        }

        //! \brief update an entry: change the key and update the position to
        //! reestablish the heap property.
        void update(HeapEntry h) {
            int pos = interface_.get_heap_position(h);
            assert(pos != -1);
            assert((unsigned int) pos < size());
            downheap(pos);
            upheap(pos);
        }

        //! \brief check heap condition
        bool check() {
            bool ok(true);
            unsigned int i, j;
            for (i = 0; i < size(); ++i) {
                if (((j = left(i)) < size()) &&
                    interface_.greater(entry(i), entry(j))) {
                    std::cerr << "Heap condition violated\n";
                    ok = false;
                }
                if (((j = right(i)) < size()) &&
                    interface_.greater(entry(i), entry(j))) {
                    std::cerr << "Heap condition violated\n";
                    ok = false;
                }
            }
            return ok;
        }

    private:
        // typedef
        typedef std::vector<HeapEntry> HeapVector;

        //! Upheap. Establish heap property.
        void upheap(unsigned int idx) {
            HeapEntry h = entry(idx);
            unsigned int parentIdx;

            while ((idx > 0) && interface_.less(h, entry(parentIdx = parent(idx)))) {
                entry(idx, entry(parentIdx));
                idx = parentIdx;
            }

            entry(idx, h);
        }

        //! Downheap. Establish heap property.
        void downheap(unsigned int idx) {
            HeapEntry h = entry(idx);
            unsigned int childIdx;
            unsigned int s = size();

            while (idx < s) {
                childIdx = left(idx);
                if (childIdx >= s)
                    break;

                if ((childIdx + 1 < s) &&
                    (interface_.less(entry(childIdx + 1), entry(childIdx))))
                    ++childIdx;

                if (interface_.less(h, entry(childIdx)))
                    break;

                entry(idx, entry(childIdx));
                idx = childIdx;
            }

            entry(idx, h);
        }

        //! Get the entry at index idx
        inline HeapEntry entry(unsigned int idx) {
            assert(idx < size());
            return (This::operator[](idx));
        }

        //! Set entry H to index idx and update H's heap position.
        inline void entry(unsigned int idx, HeapEntry h) {
            assert(idx < size());
            This::operator[](idx) = h;
            interface_.set_heap_position(h, idx);
        }

        //! Get parent's index
        inline unsigned int parent(unsigned int i) { return (i - 1) >> 1; }

        //! Get left child's index
        inline unsigned int left(unsigned int i) { return (i << 1) + 1; }

        //! Get right child's index
        inline unsigned int right(unsigned int i) { return (i << 1) + 2; }

        //! Instance of HeapInterface
        HeapInterface interface_;
    };

//=============================================================================
} // namespace easy3d

#endif  // EASY3D_CORE_HEAP_H