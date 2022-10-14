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
     * @tparam DIM The dimension (i.e., 2 or 3).
     * @tparam FT The type of the floating point number (i.e., float or double).
     */

    template<int DIM, typename FT>
    class GenericBox {
    public:
        typedef Vec<DIM, FT> Point;
        typedef Vec<DIM, FT> Vector;
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
            grow(pmin);
            grow(pmax);
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
        inline const Point& min_point() const { return min_; }
        /** Return the coordinates of the min corner. */
        inline Point& min_point() { return min_; }

        /** Return the coordinates of the max corner. */
        inline const Point& max_point() const { return max_; }
        /** Return the coordinates of the max corner. */
        inline Point& max_point() { return max_; }

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

        /** Return the max range.*/
        inline FT max_range() const {
            FT max_value = max_[0] - min_[0];
            for (int i = 1; i < DIM; ++i)
                max_value = std::max(max_value, max_[i] - min_[i]);
            return max_value;
        }

        /** Return the min range.*/
        inline FT min_range() const {
            FT min_value = max_[0] - min_[0];
            for (int i = 1; i < DIM; ++i)
                min_value = std::min(min_value, max_[i] - min_[i]);
            return min_value;
        }

        /** Return the axis index of the max range of the box.*/
        inline unsigned int max_range_axis() const {
            unsigned int axis = 0;
            FT max_value = max_[0] - min_[0];
            for (int i = 1; i < DIM; ++i) {
                FT range = max_[i] - min_[i];
                if (range > max_value) {
                    axis = i;
                    max_value = range;
                }
            }
            return axis;
        }

        /** Return the axis index of the min range of the box.*/
        inline unsigned int min_range_axis() const {
            unsigned int axis = 0;
            FT min_value = max_[0] - min_[0];
            for (int i = 1; i < DIM; ++i) {
                FT range = max_[i] - min_[i];
                if (range < min_value) {
                    axis = i;
                    min_value = range;
                }
            }
            return axis;
        }

        /** Return the center of the box. */
        inline Point center() const {
            if (is_valid())
                return FT(0.5) * (min_ + max_);
            else
                return Point(0.0);
        }

        /** Return the diagonal vector of the box. */
        inline Vector diagonal_vector() const { return max_ - min_; }

        /** Return the length of the diagonal of the box. */
        inline FT diagonal_length() const {
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

        /** Return the radius of the box (i.e., half of its diagonal length). */
        inline FT radius() const {
            return diagonal_length() * FT(0.5);
        }

        /** Return the surface area of the box. */
        inline FT surface_area() const {
            Vector ext = max_ - min_;
            if (DIM == 3)
                return FT(2.0) * (ext[0] * ext[1] + ext[0] * ext[2] + ext[1] * ext[2]);
            else if (DIM == 2)
                return ext[0] * ext[1];
            else {
                std::cerr << "surface_area() is for 2D and 3D boxes" << std::endl;
                return FT(0);
            }
        }

        /** Add a point to this box. This will compute its new extent. */
        inline void grow(const Point &p) {
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

        /** Add a box to this box. This will compute its new extent. */
        inline void grow(const thisclass &b) {
            if (b.is_valid()) {
                grow(b.min_point());
                grow(b.max_point());
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
                grow(b.min_point());
                grow(b.max_point());
            }
            return *this;
        }

        /** Does this box contain a point \c p. */
        inline bool contains(Point const& p) const {
            for (int i = 0; i < DIM; ++i) {
                if (p[i] <= min_[i] || p[i] >= max_[i])
                    return false;
            }
            return true;
        }

        /** Does this box contain another box \c b. */
        inline bool contains(thisclass const& b) const {
            return contains(b.min_point()) && contains(b.max_point());
        }

        /** Does this box intersect another box \c b. */
        inline bool intersects(thisclass const& b) const {
            for (int i = 0; i < DIM; ++i) {
                if (b.min_coord(i) > max_[i] || b.max_coord(i) < min_[i])
                    return false;
            }
            return true;
        }

    private:
        Point min_;
        Point max_;
    };


    /** Does the representation of a box have NaN? */
    template<int DIM, typename FT>
    inline bool has_nan(const GenericBox<DIM, FT> &box) {
        return has_nan(box.min_point()) || has_nan(box.max_point());
    }


    namespace geom {

#if 1
        template<int DIM, typename FT>
        GenericBox<DIM, FT> box_union(GenericBox<DIM, FT> const& a, GenericBox<DIM, FT> const& b) {
            return GenericBox<DIM, FT>(comp_min(a.min_point(), b.min_point()), comp_max(a.max_point(), b.max_point()));
        }

        template<int DIM, typename FT>
        GenericBox<DIM, FT> box_intersection(GenericBox<DIM, FT> const& a, GenericBox<DIM, FT> const& b)
        {
            return GenericBox<DIM, FT>(comp_max(a.min_point(), b.min_point()), comp_min(a.max_point(), b.max_point()));
        }
#else
        template<int DIM, typename FT>
        GenericBox<DIM, FT> box_union(GenericBox<DIM, FT> const& a, GenericBox<DIM, FT> const& b) { return a + b; }

        template<int DIM, typename FT>
        GenericBox<DIM, FT> box_intersection(GenericBox<DIM, FT> const& a, GenericBox<DIM, FT> const& b) {
            if (a.intersects(b))
                return GenericBox<DIM, FT>(comp_max(a.min_point(), b.min_point()), comp_min(a.max_point(), b.max_point()));
            else
                return GenericBox<DIM, FT>();
        }
#endif

    }

} // namespace easy3d


#endif  // EASY3D_CORE_GENERIC_BOX_H
