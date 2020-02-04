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

#ifndef EASY3D_POLYGON_H
#define EASY3D_POLYGON_H


#include <vector>
#include <algorithm>

#include <easy3d/core/vec.h>

namespace easy3d {

    template<typename FT>
    class GenericPolygon : public std::vector< Vec<2, FT> > {
        typedef std::vector< Vec<2, FT> > BaseClass;

    public:
        GenericPolygon() {}

        template<class InputIterator>
        GenericPolygon(InputIterator first, InputIterator last);

        // Reverses the orientation of the polygon. The first vertex remains the same.
        // First vertex: pointed to by `p.begin()`, or [0]
        void reverse_orientation();

        // tests if point lies inside the polygon
        bool contains(const Vec<2, FT> &point) const;

        // Returns the signed area of the polygon. This means that the
        // area is positive for counter clockwise polygons and negative
        // for clockwise polygons.
        FT signed_area() const;

        // the absolute value of the signed_area
        FT area() const;

        // Returns the smallest bounding box containing this polygon.
        GenericBox<2, FT> bbox() const;
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename FT>
    template<class InputIterator>
    inline GenericPolygon<FT>::GenericPolygon(InputIterator first, InputIterator last) {
        std::copy(first, last, BaseClass::end());
    }

    template<typename FT>
    inline void GenericPolygon<FT>::reverse_orientation() {
        if (BaseClass::size() <= 1)
            return;
        auto it = BaseClass::begin();
        std::reverse(++it, BaseClass::end());
    }


    template<typename FT>
    inline bool GenericPolygon<FT>::contains(const Vec<2, FT> &p) const {
        bool inside = false;
        std::size_t n = BaseClass::size();
        for (std::size_t i = 0, j = n - 1; i < n; j = i, ++i) {
            const Vec<2, FT> &u0 = BaseClass::at(i);
            const Vec<2, FT> &u1 = BaseClass::at(j);  // current edge

            if (((u0.y <= p.y) && (p.y < u1.y)) ||  // U1 is above the ray, U0 is on or below the ray
                ((u1.y <= p.y) && (p.y < u0.y)))    // U0 is above the ray, U1 is on or below the ray
            {
                // find x-intersection of current edge with the ray.
                // Only consider edge crossings on the ray to the right of P.
                double x = u0.x + (p.y - u0.y) * (u1.x - u0.x) / (u1.y - u0.y);
                if (x > p.x)
                    inside = !inside;
            }
        }

        return inside;
    }


    template<typename FT>
    inline FT GenericPolygon<FT>::area() const {
        return std::fabs(signed_area());
    }


    // http://astronomy.swin.edu.au/~pbourke/geometry/polyarea/
    template<typename FT>
    inline FT GenericPolygon<FT>::signed_area() const {
        FT result = 0;
        for (unsigned int i = 0; i < BaseClass::size(); i++) {
            unsigned int j = (i + 1) % BaseClass::size();
            const Vec<2, FT> &t1 = BaseClass::at(i);
            const Vec<2, FT> &t2 = BaseClass::at(j);
            result += t1.x * t2.y - t2.x * t1.y;
        }
        result /= 2.0;
        return result;
    }


    template<typename FT>
    inline GenericBox<2, FT> GenericPolygon<FT>::bbox() const {
        GenericBox<2, FT> box;
        for (auto p : *this)
            box.add_point(p);
        return box;
    }

}


#endif  // EASY3D_POLYGON_H
