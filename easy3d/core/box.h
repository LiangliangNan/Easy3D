/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_GENERIC_BOX_H
#define EASY3D_GENERIC_BOX_H

#include <cassert>
#include <algorithm>
#include <limits>

#include <easy3d/core/vec.h>


namespace easy3d {


    template <int DIM, typename FT>
    class GenericBox {
    public:
        typedef Vec<DIM, FT>		 Point;
        typedef Vec<DIM, FT>		 Vector;
        typedef GenericBox<DIM, FT>  thisclass;

    public:
        GenericBox()
            : min_(std::numeric_limits<FT>::max())   // initialized to an invalid value
            , max_(-std::numeric_limits<FT>::max())
        {
        }

        // defined by the diagonal corners
        GenericBox(const Point& pmin, const Point& pmax)
        {
            // the user might provide wrong order
            // min_ = pmin;
            // max_ = pmax;
            add_point(pmin);
            add_point(pmax);
        }

        // defined by center and radius
        GenericBox(const Point& c, FT r) {
            Vector dir(1.0);
            dir.normalize();
            min_ = c - dir * r;
            max_ = c + dir * r;
        }

        inline bool initialized() const {
            return max_.x >= min_.x - std::numeric_limits<FT>::epsilon();
        }

        inline void clear() {
            min_ = Point(std::numeric_limits<FT>::max());
            max_ = Point(-std::numeric_limits<FT>::max());
        }

        inline Point min() const {
            if (initialized())
                return min_;
            else
                return Point(FT(0.0));
        }

        inline Point max() const {
            if (initialized())
                return max_;
            else
                return Point(FT(0.0));
        }

        inline FT min(unsigned int axis) const {
            assert(axis >= 0 && axis < DIM);
            if (initialized())
                return min_[axis];
            else
                return FT(0.0);
        }
        inline FT max(unsigned int axis) const {
            assert(axis >= 0 && axis < DIM);
            if (initialized())
                return max_[axis];
            else
                return FT(0.0);
        }

        inline FT range(unsigned int axis) const {
            assert(axis >= 0 && axis < DIM);
            if (initialized())
                return max_[axis] - min_[axis];
            else
                return FT(0.0);
        }

        inline Point center() const {
            if (initialized())
                return FT(0.5) * (min_ + max_);
            else
                return Point(0.0);
        }

        inline FT diagonal() const {
            if (initialized()) {
                FT sqr_len(0.0);
                for (int i=0; i<DIM; ++i) {
                    FT d = max_[i] - min_[i];
                    sqr_len += d * d;
                }
                return std::sqrt(sqr_len);
            }
            else
                return FT(0.0);
        }

        inline void add_point(const Point& p) {
            if (initialized()) {
                for (int i=0; i<DIM; ++i) {
                    min_[i] = std::min(min_[i], p[i]);
                    max_[i] = std::max(max_[i], p[i]);
                }
            }
            else {
                min_ = p;
                max_ = p;
            }
        }

        inline void add_box(const thisclass& b) {
            add_point(b.min());
            add_point(b.max());
        }

        inline thisclass operator+(const thisclass& b) const {
            thisclass result = *this;
            result += b;
            return result;
        }

        inline thisclass& operator+=(const thisclass& b) {
            add_point(b.min());
            add_point(b.max());
            return *this;
        }

    private:
        Point min_;
        Point max_;
    };

} // namespace easy3d


#endif  // EASY3D_GENERIC_BOX_H
