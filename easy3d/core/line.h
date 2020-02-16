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

#ifndef EASY3D_LINE_H
#define EASY3D_LINE_H

#include <easy3d/core/vec.h>


namespace easy3d {


    template <int DIM, typename FT>
    class GenericLine {
    public:
        typedef Vec<DIM, FT>		 Point;
        typedef Vec<DIM, FT>		 Vector;
        typedef GenericLine<DIM, FT> thisclass;

    public:
        static GenericLine from_point_and_direction(const Point& p, const Vector& dir) { return GenericLine(p, dir); }
        static GenericLine from_two_points(const Point& p, const Point& q)   { return GenericLine(p, q - p); }

        GenericLine() {}
        void set(const Point& p, const Vector& dir) {
            p_ = p;
            dir_ = normalize(dir);
        }

        const Vector& direction() const { return dir_; }

        const Point&  point() const { return p_; }

        GenericLine opposite() const { return GenericLine(p_, -dir_); }

        // the projection of p on this line
        Point  projection(const Point &p) const { return p_ + dir_ * dot(p - p_, dir_); }

        FT	   squared_distance(const Point &p) const { return length2(projection(p) - p); }

    private:  // Ambiguities exist for this one.
        GenericLine(const Point & p, const Vector & dir);

    private:
        Point	p_;
        Vector  dir_;
    };


    template <int DIM, typename FT> inline
    GenericLine<DIM, FT>::GenericLine(const Point & p, const Vector & dir) : p_(p) {
      dir_ = normalize(dir);
      DLOG_IF(ERROR, length(dir_) < 1e-15)
              << "degenerate line constructed from point (" << p << ") and direction (" << dir << ")";
    }


    template <int DIM, typename FT> inline
    std::ostream& operator<<(std::ostream& os, const GenericLine<DIM, FT>& line) {
        return os << line.point() << " " << line.direction();
    }


    template <int DIM, typename FT> inline
    std::istream& operator>>(std::istream& is, GenericLine<DIM, FT>& line) {
        typename GenericLine<DIM, FT>::Point	p;
        typename GenericLine<DIM, FT>::Vector dir;
        is >> p >> dir;
        line.set(p, dir);
        return is;
    }

}

#endif  // EASY3D_LINE_H

