/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
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
 */

#ifndef EASY3D_SEGMENT_H
#define EASY3D_SEGMENT_H


#include <algorithm>

#include <easy3d/core/vec.h>
#include <easy3d/core/line.h>


namespace easy3d {


    template <int DIM, typename FT>
    class GenericSegment {
    public:
        typedef Vec<DIM, FT>			Point;
        typedef Vec<DIM, FT>			Vector;
        typedef GenericLine<DIM, FT>	Line;
        typedef GenericSegment<DIM, FT> thisclass;

    public:
        GenericSegment(const Point& s, const Point& t);
        GenericSegment()  {}

        const Point& source() const { return s_; }
        const Point& target() const { return t_; }
        void  set_source(const Point& s) { s_ = s; }
        void  set_target(const Point& t) { t_ = t; }

        Line  supporting_line() const { return Line::from_two_points(s_, t_); }

        Vector to_vector() const { return t_ - s_; }

        // the projection of p on the supporting line
        Point  projection(const Point &p) const;

        // test if the projection of p is inside the two end points
        bool   projected_inside(const Point& p) const;

        FT squared_ditance(const Point &p) const;

    private:
        Point s_;
        Point t_;
    };



    template<int DIM, typename FT> inline
    GenericSegment<DIM, FT>::GenericSegment(const Point& s, const Point& t) : s_(s), t_(t) {
      DLOG_IF(ERROR, distance2(s, t) < 1e-15) << "degenerate segment constructed from 2 points:"
                                              << "\t(" << s << ")"
                                              << "\t(" << t << ")";
    }


    template<int DIM, typename FT> inline
    typename GenericSegment<DIM, FT>::Point  GenericSegment<DIM, FT>::projection(const Point &p) const {
        Vector dir = normalize(t_ - s_);
        return (s_ + dir * dot(p - s_, dir));
    }


    template<int DIM, typename FT> inline
    bool GenericSegment<DIM, FT>::projected_inside(const Point& p) const {
        return (dot(s_ - p, t_ - p) < 0);
    }


    template<int DIM, typename FT> inline
    FT GenericSegment<DIM, FT>::squared_ditance(const Point &p) const {
        if (projected_inside(p))
            return distance2(projection(p), p);
        else {
            FT ds = distance2(s_, p);
            FT dt = distance2(t_, p);
            return std::min(ds, dt);
        }
    }

}


#endif  // EASY3D_SEGMENT_H
