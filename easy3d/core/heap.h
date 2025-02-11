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
#include <cassert>
#include <iostream>


namespace easy3d {

    /**
     * \brief A class implementing a heap.
     * \details This class provides methods to manage a heap data structure, including insertion, removal, and
     *      updating of elements.
     * \tparam HeapEntry The type of the elements stored in the heap.
     * \tparam HeapInterface The interface class that provides methods for comparing and managing heap positions.
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
        typedef Heap<HeapEntry, HeapInterface> This;    ///< This class.

        /**
         * \brief Default constructor.
         */
        Heap() : HeapVector() {}

        /**
         * \brief Constructs a heap with a given \c HeapInterface.
         * \param i The heap interface.
         */
        explicit Heap(const HeapInterface &i) : HeapVector(), interface_(i) {}

        /**
         * \brief Destructor.
         */
        ~Heap() = default;

        /**
         * \brief Clears the heap.
         */
        void clear() { HeapVector::clear(); }

        /**
         * \brief Checks if the heap is empty.
         * \return True if the heap is empty, false otherwise.
         */
        bool empty() { return HeapVector::empty(); }

        /**
         * \brief Returns the size of the heap.
         * \return The number of elements in the heap.
         */
        unsigned int size() { return static_cast<unsigned int>(HeapVector::size()); }

        /**
         * \brief Reserves space for \c n entries.
         * \param n The number of entries to reserve space for.
         */
        void reserve(unsigned int n) { HeapVector::reserve(n); }

        /**
         * \brief Resets the heap position of an entry to -1 (not in heap).
         * \param h The entry to reset.
         */
        void reset_heap_position(HeapEntry h) {
            interface_.set_heap_position(h, -1);
        }

        /**
         * \brief Checks if an entry is stored in the heap.
         * \param h The entry to check.
         * \return True if the entry is in the heap, false otherwise.
         */
        bool is_stored(HeapEntry h) {
            return interface_.get_heap_position(h) != -1;
        }

        /**
         * \brief Inserts an entry into the heap.
         * \param h The entry to insert.
         */
        void insert(HeapEntry h) {
            This::push_back(h);
            upheap(size() - 1);
        }

        /**
         * \brief Retrieves the first entry in the heap.
         * \return The first entry in the heap.
         */
        HeapEntry front() {
            assert(!empty());
            return entry(0);
        }

        /**
         * \brief Deletes the first entry in the heap.
         */
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

        /**
         * \brief Removes an entry from the heap.
         * \param h The entry to remove.
         */
        void remove(HeapEntry h) {
            const int pos = interface_.get_heap_position(h);
            interface_.set_heap_position(h, -1);

            assert(pos != -1);
            assert(static_cast<unsigned int>(pos) < size());

            // last item ?
            if (static_cast<unsigned int>(pos) == size() - 1)
                HeapVector::resize(size() - 1);

            else {
                entry(pos, entry(size() - 1)); // move last elem to pos
                HeapVector::resize(size() - 1);
                downheap(pos);
                upheap(pos);
            }
        }

        /**
         * \brief Updates an entry in the heap.
         * \details Changes the key of the entry and updates its position to reestablish the heap property.
         * \param h The entry to update.
         */
        void update(HeapEntry h) {
            const int pos = interface_.get_heap_position(h);
            assert(pos != -1);
            assert(static_cast<unsigned int>(pos) < size());
            downheap(pos);
            upheap(pos);
        }

        /**
         * \brief Checks the heap condition.
         * \return True if the heap condition is satisfied, false otherwise.
         */
        bool check() {
            bool ok(true);
            unsigned int j;
            for (unsigned int i = 0; i < size(); ++i) {
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

        /**
         * \brief Upheap operation to establish heap property.
         * \param idx The index to upheap from.
         */
        void upheap(unsigned int idx) {
            HeapEntry h = entry(idx);
            unsigned int parentIdx;

            while ((idx > 0) && interface_.less(h, entry(parentIdx = parent(idx)))) {
                entry(idx, entry(parentIdx));
                idx = parentIdx;
            }

            entry(idx, h);
        }

        /**
         * \brief Downheap operation to establish heap property.
         * \param idx The index to downheap from.
         */
        void downheap(unsigned int idx) {
            HeapEntry h = entry(idx);
            const unsigned int s = size();
            while (idx < s) {
                unsigned int childIdx = left(idx);
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

        /**
         * \brief Gets the entry at the specified index.
         * \param idx The index of the entry.
         * \return The entry at the specified index.
         */
        HeapEntry entry(unsigned int idx) {
            assert(idx < size());
            return (This::operator[](idx));
        }

        /**
         * \brief Sets the entry at the specified index and updates its heap position.
         * \param idx The index to set the entry at.
         * \param h The entry to set.
         */
        void entry(unsigned int idx, HeapEntry h) {
            assert(idx < size());
            This::operator[](idx) = h;
            interface_.set_heap_position(h, idx);
        }

        /**
         * \brief Gets the parent's index.
         * \param i The index of the child.
         * \return The index of the parent.
         */
        unsigned int parent(unsigned int i) { return (i - 1) >> 1; }

        /**
         * \brief Gets the left child's index.
         * \param i The index of the parent.
         * \return The index of the left child.
         */
        unsigned int left(unsigned int i) { return (i << 1) + 1; }

        /**
         * \brief Gets the right child's index.
         * \param i The index of the parent.
         * \return The index of the right child.
         */
        unsigned int right(unsigned int i) { return (i << 1) + 2; }

        //! Instance of HeapInterface
        HeapInterface interface_;
    };

} // namespace easy3d

#endif  // EASY3D_CORE_HEAP_H