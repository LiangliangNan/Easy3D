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

#ifndef EASY3D_CORE_PLANE_H
#define EASY3D_CORE_PLANE_H

#include <easy3d/core/vec.h>
#include <easy3d/core/line.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    /// \brief A 3D Plane of equation a*x + b*y + c*z + d = 0.
    /// \class GenericPlane easy3d/core/plane.h
    template<typename FT>
    class GenericPlane {
    public:
        typedef Vec<2, FT>          Point2;
        typedef Vec<3, FT>          Point3;
        typedef Vec<3, FT>          Vector3;
        typedef GenericLine<3, FT>  Line3;
        typedef GenericPlane<FT>    thisclass;

    public:
        /// \brief Constructs a plane from three points \p p1, \p p2, and \p p3.
        GenericPlane(const Point3 &p1, const Point3 &p2, const Point3 &p3);

        /// \brief Constructs a plane from a point \p p and the plane normal \p n.
        GenericPlane(const Point3 &p, const Vector3 &n);

        /// \brief Constructs a plane from its equation parameters \p a, \p b, \p c, and \p d.
        GenericPlane(FT a, FT b, FT c, FT d) {
            coeff_[0] = a;
            coeff_[1] = b;
            coeff_[2] = c;
            coeff_[3] = d;
        }

        GenericPlane() = default;

        /// \brief Returns plane equation parameter \p a.
        inline FT a() const { return coeff_[0]; }

        /// \brief Returns plane equation parameter \p b.
        inline FT b() const { return coeff_[1]; }

        /// \brief Returns plane equation parameter \p c.
        inline FT c() const { return coeff_[2]; }

        /// \brief Returns plane equation parameter \p d.
        inline FT d() const { return coeff_[3]; }

        /// \brief Returns the \p idx_th parameter of the plane equation.
        inline FT &operator[](size_t idx) {
            assert(idx < 4);
            return coeff_[idx];
        }

        /// \brief Returns the \p idx_th const parameter of the plane equation.
        inline const FT &operator[](size_t idx) const {
            assert(idx < 4);
            return coeff_[idx];
        }

        /// \brief Returns the normal of the plane.
        Vector3 normal() const;

        /// \brief Returns a point lying on this plane.
        /// \note It is a fixed point (anytime you can this function it returns the same point).
        Point3 point() const;

        /// \brief Returns the first ortho-base defined on this plane.
        Vector3 base1() const;

        /// \brief Returns the second ortho-base defined on this plane.
        Vector3 base2() const;

        /// \brief Converts a 3D point into a 2D point relative to the local coordinate system defined by the three
        /// orthogonal vectors base1(), base2(), and normal().
        /// \note After 3D->2D and then 2D->3D conversion, the resulted 3D point remains unchanged ONLY IF the input
        /// point lies on the plane. In case the original 3D point does not lie on the plane, the resulted 3D point will
        /// have coordinates equal to the projection of the input point onto the plane.
        Point2 to_2d(const Point3 &p) const;

        /// \brief Converts a 2D point in the local coordinate system defined by the three orthogonal vectors base1(),
        /// base2(), and normal() into the 3D space.
        /// \note After 3D->2D and then 2D->3D conversion, the resulted 3D point remains unchanged ONLY IF the input
        /// point lies on the plane. In case the original 3D point does not lie on the plane, the resulted 3D point will
        /// have coordinates equal to the projection of the input point onto the plane.
        Point3 to_3d(const Point2 &p) const;

        /// \brief The projection of a point \p p on this plane.
        Point3 projection(const Point3 &p) const;

        /// \brief Evaluates the plane (i.e., computes the value of a * x + b * y + c * z + d) for the given point \p p.
        inline FT value(const Point3 &p) const {
            return (coeff_[0] * p.x + coeff_[1] * p.y + coeff_[2] * p.z + coeff_[3]);
        }

        /// \brief Computes the squared distance of a point \p p to this plane.
        FT squared_distance(const Point3 &p) const;

        /// \brief Tests if a \p line intersects with this plane.
        /// Returns true if they do intersect. In this case, \p p stores the intersecting point.
        /// Otherwise, it returns false (i.e., they don't intersect, meaning the line is parallel to this plane).
        /// \note Both line and the plane are unlimited.
        bool intersect(const Line3 &line, Point3 &p) const;

        /// \brief Tests if a \p line intersects with this plane.
        /// Returns true if they do intersect. Otherwise, it returns false (i.e., they don't intersect, meaning the line
        /// is parallel to this plane).
        /// \note Both line and the plane are unlimited.
        bool intersect(const Line3 &line) const;

        /// \brief Tests if a \p line segment (given by its two end points \p s and \p t) intersects with this plane.
        /// Returns true if they do intersect. In this case, \p p stores the intersecting point.
        /// Otherwise, it returns false (i.e., they don't intersect, meaning the two end points of the line segment
        /// lie on the same side of the the plane).
        bool intersect(const Point3 &s, const Point3 &t, Point3 &p) const;

        /// \brief Tests if a \p line segment (given by its two end points \p s and \p t) intersects with this plane.
        /// Returns true if they do intersect. Otherwise, it returns false (i.e., they don't intersect, meaning the two
        /// end points of the line segment lie on the same side of the the plane).
        bool intersect(const Point3 &s, const Point3 &t) const;

        /// \brief Tests if this plane intersects with \p another plane.
        /// Returns true if they do intersect. In this case, \p line stores the intersecting line.
        /// Otherwise, it returns false (i.e., they don't intersect, meaning the two planes are parallel).
        bool intersect(const thisclass &another, Line3 &line) const;

        /// \brief Tests if this plane intersects with \p another plane.
        /// Returns true if they do intersect and false if they don't intersect (i.e.,the two planes are parallel).
        bool intersect(const thisclass &another) const;

        /// \brief Determines the relative orientation of a point with respect to this plane.
        /// The return value is one of the following:
        ///   - 1: p is on the positive side
        ///   - 0: p is belonging to the plane
        ///   - -1: p is on the negative side
        int orient(const Point3 &p) const;

        /// \brief Returns the constant memory address of the coefficients.
        const FT *data() const { return coeff_; }

        /// \brief Returns the memory address of the coefficients.
        FT *data() { return coeff_; }

        /// \brief Conversion operator returning the constant memory address of the coefficients. This is very
        /// convenient to pass a plane as a parameter to functions.
        operator const FT *() const { return coeff_; }

        /// \brief Conversion operator returning the memory address of the coefficients. This is very convenient to
        /// pass a plane as a parameter to functions.
        operator FT *() { return coeff_; }

    private:
        FT coeff_[4]; // representing the a, b, c, and d, respectively
    };


    template<typename FT>
    std::ostream &operator<<(std::ostream &os, const GenericPlane<FT> &plane);

    template<typename FT>
    std::istream &operator>>(std::istream &is, GenericPlane<FT> &plane);


    template<typename FT>
    inline
    GenericPlane<FT>::GenericPlane(const Point3 &p1, const Point3 &p2, const Point3 &p3) {
        Vector3 n = cross(p2 - p1, p3 - p1);
        n = normalize(n);

        coeff_[0] = n.x;
        coeff_[1] = n.y;
        coeff_[2] = n.z;
        coeff_[3] = -(coeff_[0] * p1.x + coeff_[1] * p1.y + coeff_[2] * p1.z);

        DLOG_IF(length(n) < 1e-15, ERROR) << "degenerate plane constructed from 3 points:"
                                          << "\t(" << p1 << ")"
                                          << "\t(" << p2 << ")"
                                          << "\t(" << p3 << ")";
    }

    template<typename FT>
    inline
    GenericPlane<FT>::GenericPlane(const Point3 &p, const Vector3 &n) {
        Vector3 nn = normalize(n);
        coeff_[0] = nn.x;
        coeff_[1] = nn.y;
        coeff_[2] = nn.z;
        coeff_[3] = -(coeff_[0] * p.x + coeff_[1] * p.y + coeff_[2] * p.z);

        DLOG_IF(length(nn) < 1e-15, ERROR) << "degenerate plane constructed from point ("
                                           << p << ") and normal (" << n << ")";
    }


    template<typename FT>
    inline
    typename GenericPlane<FT>::Vector3 GenericPlane<FT>::normal() const {
        Vector3 n = normalize(Vector3(coeff_[0], coeff_[1], coeff_[2]));
        DLOG_IF(length(n) < 1e-15, ERROR) << "degenerate plane with normal: (" << n << ")";
        return n;
    }


    template<typename FT>
    inline
    typename GenericPlane<FT>::Vector3 GenericPlane<FT>::base1() const {
        if (coeff_[0] == 0)        // parallel to x-axis
            return Vector3(FT(1), FT(0), FT(0));
        else if (coeff_[1] == 0)    // parallel to y-axis
            return Vector3(FT(0), FT(1), FT(0));
        else if (coeff_[2] == 0)    // parallel to z-axis
            return Vector3(FT(0), FT(0), FT(1));
        else {
            Vector3 base(-coeff_[1], coeff_[0], FT(0));
            return normalize(base);
        }
    }


    template<typename FT>
    inline
    typename GenericPlane<FT>::Vector3 GenericPlane<FT>::base2() const {
        Vector3 base = cross(normal(), base1());
        return normalize(base);
    }

    template<typename FT>
    inline
    typename GenericPlane<FT>::Point2 GenericPlane<FT>::to_2d(const Point3 &p) const {
        Vector3 vec = p - point();
        FT x = dot(vec, base1());
        FT y = dot(vec, base2());
        return Point2(x, y);
    }


    template<typename FT>
    inline
    typename GenericPlane<FT>::Point3 GenericPlane<FT>::to_3d(const Point2 &p) const {
        return point() + base1() * p.x + base2() * p.y;
    }


    template<typename FT>
    inline
    typename GenericPlane<FT>::Point3 GenericPlane<FT>::point() const {
        Point3 p(FT(0), FT(0), FT(0));
        if (std::abs(coeff_[0]) >= std::abs(coeff_[1]) && std::abs(coeff_[0]) >= std::abs(coeff_[2]))
            p.x = -coeff_[3] / coeff_[0];
        else if (std::abs(coeff_[1]) >= std::abs(coeff_[0]) && std::abs(coeff_[1]) >= std::abs(coeff_[2]))
            p.y = -coeff_[3] / coeff_[1];
        else
            p.z = -coeff_[3] / coeff_[2];
        return p;

        // the code below is not numerically stable
        //Point3 p(FT(0), FT(0), FT(0));
        //if (coeff_[0] != 0)
        //	p.x = -coeff_[3] / coeff_[0];
        //else if (coeff_[1] != 0)
        //	p.y = -coeff_[3] / coeff_[1];
        //else
        //	p.z = -coeff_[3] / coeff_[2];
        //return p;
    }


    // return values:
    //   1: p is on the positive side
    //  -1: p is on the negative side
    //   0: the point p is on the plane.
    template<typename FT>
    inline
    int GenericPlane<FT>::orient(const Point3 &p) const {
        FT v = value(p);
        if (std::abs(v) < 1e-15)
            return 0;

        return (v > 0.0 ? 1 : -1);
    }


    template<typename FT>
    inline
    typename GenericPlane<FT>::Point3 GenericPlane<FT>::projection(const Point3 &p) const {
        // the equation of the plane is Ax+By+Cz+D=0
        // the normal direction is (A,B,C)
        // the projected point is p-lambda(A,B,C) where
        // A(x-lambda*A) + B(y-lambda*B) + C(z-lambda*C) + D = 0

        FT num = coeff_[0] * p.x + coeff_[1] * p.y + coeff_[2] * p.z + coeff_[3];
        FT den = coeff_[0] * coeff_[0] + coeff_[1] * coeff_[1] + coeff_[2] * coeff_[2];
        FT lambda = num / den;

        FT x = p.x - lambda * coeff_[0];
        FT y = p.y - lambda * coeff_[1];
        FT z = p.z - lambda * coeff_[2];
        return Point3(x, y, z);
    }


    template<typename FT>
    inline
    FT GenericPlane<FT>::squared_distance(const Point3 &p) const {
        FT v = value(p);
        return (v * v) / (coeff_[0] * coeff_[0] + coeff_[1] * coeff_[1] + coeff_[2] * coeff_[2]);
    }


    template<typename FT>
    inline
    bool GenericPlane<FT>::intersect(const Line3 &line) const {
        Vector3 dir = line.direction();
        FT c = dot(dir, normal());
        if (std::abs(c) < 1e-15)
            return false;
        else
            return true;
    }


    template<typename FT>
    inline
    bool GenericPlane<FT>::intersect(const Line3 &line, Point3 &p) const {
        const Vector3 &dir = line.direction();
        FT c = dot(dir, normal());
        if (std::abs(c) < 1e-15)
            return false;

        const Point3 &p0 = line.point();
        // p = p0 + dir * t
        // equation: p is in the plane (so we first compute t)
        FT t = -(coeff_[0] * p0.x + coeff_[1] * p0.y + coeff_[2] * p0.z + coeff_[3]) /
               (coeff_[0] * dir.x + coeff_[1] * dir.y + coeff_[2] * dir.z);
        p = p0 + dir * t;
        return true;
    }


    template<typename FT>
    inline
    bool GenericPlane<FT>::intersect(const Point3 &s, const Point3 &t) const {
        int ss = orient(s);
        int st = orient(t);
        if ((ss == 1 && st == -1) || (ss == -1 && st == 1))
            return true;
        else if (ss == 0 || st == 0)
            return true;
        else
            return false;
    }


    template<typename FT>
    inline
    bool GenericPlane<FT>::intersect(const Point3 &s, const Point3 &t, Point3 &p) const {
        int ss = orient(s);
        int st = orient(t);
        if ((ss == 1 && st == -1) || (ss == -1 && st == 1)) {
            if (intersect(Line3::from_two_points(s, t), p))
                return true;
            else {
                LOG(ERROR) << "fatal error. Should have intersection";
                return false;
            }
        } else {
            if (ss == 0) {
                p = s;
                return true;
            } else if (st == 0) {
                p = t;
                return true;
            } else {
                // no intersection with the plane
                return false;
            }
        }
    }


    template<typename FT>
    inline
    bool GenericPlane<FT>::intersect(const thisclass &another) const {
        const FT &a = coeff_[0];
        const FT &b = coeff_[1];
        const FT &c = coeff_[2];
        const FT &d = coeff_[3];
        const FT &p = another.coeff_[0];
        const FT &q = another.coeff_[1];
        const FT &r = another.coeff_[2];
        const FT &s = another.coeff_[3];

        FT det = a * q - p * b;
        if (det != 0)
            return true;
        det = a * r - p * c;
        if (det != 0)
            return true;
        det = b * r - c * q;
        if (det != 0)
            return true;

        // degenerate case
        if (a != 0 || p != 0)
            return (a * s == p * d);
        if (b != 0 || q != 0)
            return (b * s == q * d);
        if (c != 0 || r != 0)
            return (c * s == r * d);

        return true;
    }


    template<typename FT>
    inline
    bool GenericPlane<FT>::intersect(const thisclass &another, Line3 &line) const {
        const FT &a = coeff_[0];
        const FT &b = coeff_[1];
        const FT &c = coeff_[2];
        const FT &d = coeff_[3];
        const FT &p = another.coeff_[0];
        const FT &q = another.coeff_[1];
        const FT &r = another.coeff_[2];
        const FT &s = another.coeff_[3];

        FT det = a * q - p * b;
        if (det != 0) {
            const Point3 pt((b * s - d * q) / det, (p * d - a * s) / det, 0);
            const Vector3 dir((b * r - c * q), (p * c - a * r), det);
            line = Line3::from_point_and_direction(pt, dir);
            return true;
        }
        det = a * r - p * c;
        if (det != 0) {
            const Point3 pt((c * s - d * r) / det, 0, (p * d - a * s) / det);
            const Vector3 dir((c * q - b * r), det, (p * b - a * q));
            line = Line3::from_point_and_direction(pt, dir);
            return true;
        }
        det = b * r - c * q;
        if (det != 0) {
            const Point3 pt(0, (c * s - d * r) / det, (d * q - b * s) / det);
            const Vector3 dir(det, (c * p - a * r), (a * q - b * p));
            line = Line3::from_point_and_direction(pt, dir);
            return true;
        }

        // degenerate case
        return false; // coplanar
    }


    /// Output stream support for GenericPlane.
    template<typename FT>
    inline
    std::ostream &operator<<(std::ostream &os, const GenericPlane<FT> &plane) {
        return os << plane[0] << ' ' << plane[1] << ' ' << plane[2] << ' ' << plane[3];
    }

    /// Input stream support for GenericPlane.
    template<typename FT>
    inline
    std::istream &operator>>(std::istream &is, GenericPlane<FT> &plane) {
        return is >> plane[0] >> plane[1] >> plane[2] >> plane[3];
    }


    namespace geom {
        /**
         * Check if three planes intersect. Returns false if one of the planes is parallel to any of the others.
         * \param plane1 The first plane.
         * \param plane2 The second plane.
         * \param plane3 The third plane.
         * \param point The intersecting point. \note The intersecting point is valid only if the function returns true.
         * \return true if the three planes intersect at a point.
         */
        template<typename FT>
        inline
        bool intersect(const GenericPlane<FT> &plane1, const GenericPlane<FT> &plane2, const GenericPlane<FT> &plane3,
                       typename GenericPlane<FT>::Point3 &point) {
            typename GenericPlane<FT>::Line3 line;
            if (plane1.intersect(plane2, line))
                return plane3.intersect(line, point);
            return false;
        }
    }

}


#endif  // EASY3D_CORE_PLANE_H
