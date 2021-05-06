/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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

#ifndef EASY3D_CORE_GENERIC_BOX_H
#define EASY3D_CORE_GENERIC_BOX_H

#include <cassert>
#include <algorithm>
#include <limits>

#include <easy3d/core/vec.h>


namespace easy3d {

    /**
     * \brief GenericBox represents the bounding box of shapes.
     * \class GenericBox easy3d/core/box.h
     * @tparam DIM The dimention (i.e., 2 or 3).
     * @tparam FT The type of the floating point number (i.e., float or double).
     */

    template<int DIM, typename FT>
    class GenericBox {
    public:
        typedef Vec <DIM, FT> Point;
        typedef Vec <DIM, FT> Vector;
        typedef GenericBox<DIM, FT> thisclass;

    public:
        /** Construct a box uninitialized (which is invalid). */
        GenericBox()
                : min_(std::numeric_limits<FT>::max())   // is_valid to an invalid value
                , max_(-std::numeric_limits<FT>::max()) {
        }

        /** Construct a box from its diagonal corners. */
        GenericBox(const Point &pmin, const Point &pmax) {
            // the user might provide wrong order
            // min_ = pmin;
            // max_ = pmax;
            add_point(pmin);
            add_point(pmax);
        }

        /** Construct a box from its center and radius. */
        GenericBox(const Point &c, FT r) {
            Vector dir(1.0);
            dir.normalize();
            min_ = c - dir * r;
            max_ = c + dir * r;
        }

        /** Is the box valid. */
        inline bool is_valid() const {
            for (int i = 0; i < DIM; ++i) {
                if (max_[i] >= min_[i] - std::numeric_limits<FT>::epsilon())
                    return true;
            }
            return false;
        }

        /** Invalidate the box. */
        inline void clear() {
            min_ = Point(std::numeric_limits<FT>::max());
            max_ = Point(-std::numeric_limits<FT>::max());
        }

        /** Return the coordinates of the min corner. */
        inline const Point& min_point() const {
            return min_;
        }

        /** Return the coordinates of the max corner. */
        inline const Point& max_point() const {
            return max_;
        }

        /** Return a component of the coordinates of the min corner. \c axis must be in [0, DIM). */
        inline FT min_coord(unsigned int axis) const {
            assert(axis >= 0 && axis < DIM);
            if (is_valid())
                return min_[axis];
            else
                return FT(0.0);
        }

        /** Return a component of the coordinates of the max corner. \c axis must be in [0, DIM). */
        inline FT max_coord(unsigned int axis) const {
            assert(axis >= 0 && axis < DIM);
            if (is_valid())
                return max_[axis];
            else
                return FT(0.0);
        }

        /** Return the range of the box along the axis. \c axis must be in [0, DIM). */
        inline FT range(unsigned int axis) const {
            assert(axis >= 0 && axis < DIM);
            if (is_valid())
                return max_[axis] - min_[axis];
            else
                return FT(0.0);
        }

        /** Return the max of range(0), range(1), and range(2).*/
        inline FT max_range() const { return std::max(range(0), std::max(range(1), range(2))); }

        /** Return the max of range(0), range(1), and range(2).*/
        inline FT min_range() const { return std::min(range(0), std::min(range(1), range(2))); }

        /** Return the center of the box. */
        inline Point center() const {
            if (is_valid())
                return FT(0.5) * (min_ + max_);
            else
                return Point(0.0);
        }

        /** Return the length of the diagonal of the box. */
        inline FT diagonal() const {
            if (is_valid()) {
                FT sqr_len(0.0);
                for (int i = 0; i < DIM; ++i) {
                    FT d = max_[i] - min_[i];
                    sqr_len += d * d;
                }
                return std::sqrt(sqr_len);
            } else
                return FT(0.0);
        }

        /** Return the radius of the box (i.e., half of its diagonal). */
        inline FT radius() const {
            return diagonal() * FT(0.5);
        }

        /** Add a point to this box. This will compute its new extend. */
        inline void add_point(const Point &p) {
            if (is_valid()) {
                for (int i = 0; i < DIM; ++i) {
                    min_[i] = std::min(min_[i], p[i]);
                    max_[i] = std::max(max_[i], p[i]);
                }
            } else {
                min_ = p;
                max_ = p;
            }
        }

        /** Add a box to this box. This will compute its new extend. */
        inline void add_box(const thisclass &b) {
            if (b.is_valid()) {
                add_point(b.min_point());
                add_point(b.max_point());
            }
        }

        /** Return the bounding box of 'this' and another box \c b. */
        inline thisclass operator+(const thisclass &b) const {
            thisclass result = *this;
            if (b.is_valid())
                result += b;
            return result;
        }

        /** Modify this box by adding another box \c b. */
        inline thisclass &operator+=(const thisclass &b) {
            if (b.is_valid()) {
                add_point(b.min_point());
                add_point(b.max_point());
            }
            return *this;
        }

    private:
        Point min_;
        Point max_;
    };


    template<int DIM, typename FT>
    inline bool has_nan(const GenericBox<DIM, FT> &box) {
        return has_nan(box.min_point()) || has_nan(box.max_point());
    }

} // namespace easy3d


#endif  // EASY3D_CORE_GENERIC_BOX_H
