/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_CORE_POLYGON_H
#define EASY3D_CORE_POLYGON_H


#include <vector>
#include <algorithm>
#include <cmath>
#include <utility>

#include <easy3d/core/vec.h>
#include <easy3d/core/box.h>


namespace easy3d {

    /**
     * \brief A polygon represented by an ordered sequence of 2D vertices.
     * \details The polygon is implicitly closed, i.e., the last vertex is connected to the first one. The closing
     *      edge is not stored, so the first vertex must not be repeated at the end. The vertices are accessed
     *      through the inherited std::vector interface (size(), operator[], begin(), end(), etc.).
     *
     *      The orientation is that of the vertex order in the xy-plane: a counter-clockwise polygon has a
     *      positive signed area and is_clockwise() returns false, a clockwise polygon has a negative signed area
     *      and is_clockwise() returns true.
     *
     *      Point containment uses an even-odd ray-crossing test. Results are only unambiguous for a simple
     *      polygon, i.e., one whose vertices are pairwise distinct and whose non-adjacent edges do not cross
     *      or touch each other (see is_simple()).
     * \see Polygon2, is_simple(), is_clockwise()
     */
    /// \class GenericPolygon easy3d/core/polygon.h
    template<typename FT>
    class GenericPolygon : public std::vector< Vec<2, FT> > {
        typedef std::vector< Vec<2, FT> > BaseClass;

    public:
        /// Construct an empty polygon.
        GenericPolygon() = default;

        /// Construct a polygon with a fixed number of vertices, all at the origin (0, 0).
        /// \param size Number of vertices.
        explicit GenericPolygon(std::size_t size) { BaseClass::resize(size); }

        /**
         * \brief Construct a polygon by copying a vertex range.
         * \param first Start of the input range.
         * \param last End of the input range.
         */
        template<class InputIterator>
        GenericPolygon(InputIterator first, InputIterator last);

        /**
         * \brief Checks whether the polygon is simple.
         * \details A polygon is simple if its vertices are pairwise distinct and no two non-adjacent edges
         *      intersect or touch each other; adjacent edges share a vertex by definition. Because the polygon
         *      is implicitly closed, pairwise distinctness includes the last vertex not repeating the first
         *      position. A polygon with fewer than three vertices is not simple.
         *
         *      Vertices are compared by exact equality and the edge test evaluates pairwise
         *      orientation cross products in FT arithmetic, so nearly coincident vertices and nearly
         *      touching edges are not detected. Algorithms that require
         *      a simple polygon (e.g., the convex partition in easy3d/algo/polygon_partition.h) expect input
         *      that passes this check; a polygon that fails it must first be normalized, for instance with the
         *      CSG operators in easy3d/algo/tessellator.h.
         * \return True when the polygon is simple.
         * \sa is_clockwise()
         */
        bool is_simple() const;

        /**
         * \brief Checks whether the polygon is clockwise.
         * \details The result is the sign of signed_area(): a clockwise polygon has a negative signed area and
         *      returns true, a counter-clockwise polygon has a positive signed area and returns false. The
         *      orientation is a property of the vertex order and does not depend on which vertex is first. For
         *      a self-intersecting polygon the sign is that of the algebraic area, which is not a meaningful
         *      orientation; use is_simple() to detect such input.
         * \return True when the vertex order is clockwise.
         * \sa is_simple(), signed_area()
         */
        bool is_clockwise() const;

        /// \brief Reverses the orientation of the polygon, i.e., turns a clockwise polygon into a
        /// counter-clockwise one and vice versa. The first vertex p[0] remains the same.
        void reverse_orientation();

        /**
         * \brief Test whether a point lies inside the polygon.
         * \param point Point to test.
         * \return True when the even-odd ray-crossing test classifies it inside. A point exactly on an edge or
         *      vertex is not handled specially and may be classified either way.
         */
        bool contains(const Vec<2, FT> &point) const;

        /**
         * \brief Test whether all vertices of another polygon lie inside this polygon.
         * \param plg Polygon whose vertices are tested.
         * \return True when every vertex passes contains(point); edge crossings
         *         between the polygons are not tested. An empty polygon is trivially contained.
         */
        bool contains(const GenericPolygon<FT> &plg) const;

        /// Return the signed area of the polygon.
        /// \return Positive for counter-clockwise vertex order, negative for clockwise order, in squared
        ///         coordinate units. For a self-intersecting polygon this is the algebraic sum, whose magnitude
        ///         can be smaller than the area that is actually enclosed.
        FT signed_area() const;

        /// Return the absolute area of the polygon.
        /// \return Non-negative area in squared coordinate units.
        FT area() const;

        /// Return the smallest axis-aligned box containing the vertices.
        /// \return Polygon bounding box; it is invalid (see GenericBox::is_valid()) for an empty polygon.
        GenericBox<2, FT> bbox() const;
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename FT>
    template<class InputIterator>
    inline GenericPolygon<FT>::GenericPolygon(InputIterator first, InputIterator last)
        : BaseClass(first, last) {
    }

    template<typename FT>
    inline bool GenericPolygon<FT>::is_simple() const {
        const std::size_t n = BaseClass::size();
        if (n < 3)
            return false;

        // The polygon is implicitly closed, so a position must not occur twice (in particular, the last vertex
        // must not repeat the first one).
        std::vector<std::pair<FT, FT> > coords;
        coords.reserve(n);
        for (const auto &p : *this)
            coords.emplace_back(p.x, p.y);
        std::sort(coords.begin(), coords.end());
        for (std::size_t i = 1; i < n; ++i) {
            if (coords[i] == coords[i - 1])
                return false;
        }

        const auto cross = [](const Vec<2, FT> &u, const Vec<2, FT> &v) {
            return u.x * v.y - u.y * v.x;
        };
        const auto between = [](FT p, FT q, FT r) {
            return std::min(p, q) <= r && r <= std::max(p, q);
        };
        // Closed segments ab and cd intersect or touch. Adjacent segments share an endpoint, so callers skip them.
        const auto intersect = [&](const Vec<2, FT> &a, const Vec<2, FT> &b,
                                   const Vec<2, FT> &c, const Vec<2, FT> &d) {
            const FT d1 = cross(b - a, c - a);
            const FT d2 = cross(b - a, d - a);
            const FT d3 = cross(d - c, a - c);
            const FT d4 = cross(d - c, b - c);
            if (((d1 > FT(0) && d2 < FT(0)) || (d1 < FT(0) && d2 > FT(0))) &&
                ((d3 > FT(0) && d4 < FT(0)) || (d3 < FT(0) && d4 > FT(0))))
                return true;
            if (d1 == FT(0) && between(a.x, b.x, c.x) && between(a.y, b.y, c.y))
                return true;
            if (d2 == FT(0) && between(a.x, b.x, d.x) && between(a.y, b.y, d.y))
                return true;
            if (d3 == FT(0) && between(c.x, d.x, a.x) && between(c.y, d.y, a.y))
                return true;
            if (d4 == FT(0) && between(c.x, d.x, b.x) && between(c.y, d.y, b.y))
                return true;
            return false;
        };

        // Sort the edges by their left end and only compare an edge with those whose x-range overlaps it.
        struct Segment {
            std::size_t index;
            FT min_x, max_x, min_y, max_y;
        };
        std::vector<Segment> segments(n);
        for (std::size_t i = 0; i < n; ++i) {
            const Vec<2, FT> &p = BaseClass::at(i);
            const Vec<2, FT> &q = BaseClass::at((i + 1) % n);
            segments[i] = {i, std::min(p.x, q.x), std::max(p.x, q.x), std::min(p.y, q.y), std::max(p.y, q.y)};
        }
        std::sort(segments.begin(), segments.end(),
                  [](const Segment &a, const Segment &b) { return a.min_x < b.min_x; });

        for (std::size_t i = 0; i < n; ++i) {
            const Segment &s = segments[i];
            for (std::size_t j = i + 1; j < n && segments[j].min_x <= s.max_x; ++j) {
                const Segment &t = segments[j];
                if (t.min_y > s.max_y || t.max_y < s.min_y)
                    continue;
                if (t.index == (s.index + 1) % n || (t.index + 1) % n == s.index)  // adjacent edges
                    continue;
                if (intersect(BaseClass::at(s.index), BaseClass::at((s.index + 1) % n),
                              BaseClass::at(t.index), BaseClass::at((t.index + 1) % n)))
                    return false;
            }
        }
        return true;
    }

    template<typename FT>
    inline bool GenericPolygon<FT>::is_clockwise() const {
        return signed_area() < 0;
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
        const std::size_t n = BaseClass::size();
        for (std::size_t i = 0, j = n - 1; i < n; j = i, ++i) {
            const Vec<2, FT> &u0 = BaseClass::operator[](i);
            const Vec<2, FT> &u1 = BaseClass::operator[](j);  // current edge

            if (((u0.y <= p.y) && (p.y < u1.y)) ||  // U1 is above the ray, U0 is on or below the ray
                ((u1.y <= p.y) && (p.y < u0.y)))    // U0 is above the ray, U1 is on or below the ray
            {
                // find x-intersection of current edge with the ray.
                // Only consider edge crossings on the ray to the right of P.
                const FT x = u0.x + (p.y - u0.y) * (u1.x - u0.x) / (u1.y - u0.y);
                if (x > p.x)
                    inside = !inside;
            }
        }

        return inside;
    }

    template<typename FT>
    inline bool GenericPolygon<FT>::contains(const GenericPolygon<FT> &plg) const {
        for (const auto &p : plg) {
            if (!contains(p))
                return false;
        }
        return true;
    }

    template<typename FT>
    inline FT GenericPolygon<FT>::area() const {
        return std::fabs(signed_area());
    }


    // http://astronomy.swin.edu.au/~pbourke/geometry/polyarea/
    template<typename FT>
    inline FT GenericPolygon<FT>::signed_area() const {
        FT result = 0;
        const std::size_t n = BaseClass::size();
        for (std::size_t i = 0; i < n; ++i) {
            const std::size_t j = (i + 1) % n;
            const Vec<2, FT> &t1 = BaseClass::operator[](i);
            const Vec<2, FT> &t2 = BaseClass::operator[](j);
            result += t1.x * t2.y - t2.x * t1.y;
        }
        result /= FT(2);
        return result;
    }


    template<typename FT>
    inline GenericBox<2, FT> GenericPolygon<FT>::bbox() const {
        if (BaseClass::empty())
            return GenericBox<2, FT>();
        const Vec<2, FT> &first = BaseClass::operator[](0);
        FT min_x = first.x, max_x = first.x, min_y = first.y, max_y = first.y;
        for (std::size_t i = 1; i < BaseClass::size(); ++i) {
            const Vec<2, FT> &p = BaseClass::operator[](i);
            min_x = std::min(min_x, p.x);
            max_x = std::max(max_x, p.x);
            min_y = std::min(min_y, p.y);
            max_y = std::max(max_y, p.y);
        }
        return GenericBox<2, FT>(Vec<2, FT>(min_x, min_y), Vec<2, FT>(max_x, max_y));
    }

}


#endif  // EASY3D_CORE_POLYGON_H
