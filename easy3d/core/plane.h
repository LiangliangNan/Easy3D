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

#ifndef EASY3D_PLANE_H
#define EASY3D_PLANE_H

#include <easy3d/core/vec.h>
#include <easy3d/core/line.h>


namespace easy3d {


    // A 3D Plane of equation a.x + b.y + c.z + d = 0
    template <typename FT>
    class GenericPlane
    {
    public:
        typedef Vec<2, FT>		Point2;
        typedef Vec<3, FT>		Point3;
        typedef Vec<3, FT>		Vector3;
        typedef GenericLine<3, FT>	Line3;
        typedef GenericPlane<FT>	thisclass;

    public:
        GenericPlane(const Point3& p1, const Point3& p2, const Point3& p3);
        GenericPlane(const Point3& p, const Vector3& n);
        GenericPlane(FT a, FT b, FT c, FT d) { coeff_[0] = a;	coeff_[1] = b;	coeff_[2] = c;	coeff_[3] = d; }
        GenericPlane() {}

        inline FT a() const { return coeff_[0]; }
        inline FT b() const { return coeff_[1]; }
        inline FT c() const { return coeff_[2]; }
        inline FT d() const { return coeff_[3]; }

        inline FT& operator[](size_t idx) { assert(idx < 4); return coeff_[idx]; }
        inline const FT& operator[](size_t idx) const { assert(idx < 4); return coeff_[idx]; }

        // returns the normal of the plane
        Vector3 normal() const;

        // return a point lying on this plane.
        // NOTE: it is a fixed point (anytime you can this function it returns
        //		 the same point).
        Point3  point() const;

        // return two orthogonal directions on this plane
        Vector3 base1() const;
        Vector3 base2() const;

        // 2D / 3D conversion: relative to the coordinate system defined
        // by the three orthogonal vectors: (base1, base2, normal).
        // NOTE: after 3D->2D and then 2D->3D conversion, the resulted 3D
        //		 point will remain unchanged ONLY IF the input point lies
        //		 on the plane. In case the original 3D point does not lie
        //		 on the plane, the resulted 3D point will have coordinates
        //		 equal to the projection of the input point onto the plane.
        Point2	to_2d(const Point3& p) const;
        Point3	to_3d(const Point2& p) const;

        // the projection of a point 'p' on this plane
        Point3	projection(const Point3 &p) const;

        // given a point 'p', compute the value of the equation
        inline FT value(const Point3& p) const { return (coeff_[0] * p.x + coeff_[1] * p.y + coeff_[2] * p.z + coeff_[3]); }

        // squared distance of a point 'p' to this plane
        FT	squared_ditance(const Point3 &p) const;

        // compute the intersection with 'line'.
        // returns false if the line is parallel with this plane.
        // NOTE: both line and the plane are unlimited.
        bool intersect(const Line3& line, Point3& p) const;
        // test if this plane intersects with a line.
        bool intersect(const Line3& line) const;

        // compute the intersection with a line segment (s, t).
        // returns false if there is no intersection (i.e., s and t lie in the same side).
        bool intersect(const Point3& s, const Point3& t, Point3& p) const;
        // test if this plane intersects with a line segment (s, t).
        bool intersect(const Point3& s, const Point3& t) const;

        // return values:
        //    1: p is on the positive side
        //    0: p is belonging to the plane
        //   -1: p is on the negative side
        int orient(const Point3& p) const;

        // returns the memory address of the coefficients.
        const FT* data() const { return coeff_; }
        FT* data() { return coeff_; }

        // Conversion operator returning the memory address of the coefficients.
        // Very convenient to pass the data pointer as a parameter to functions.
        operator const FT*() const { return coeff_; }
        operator FT*() { return coeff_; }

    private:
        FT coeff_[4]; // representing the a, b, c, and d, respectively
    };



    template <typename FT>	std::ostream& operator<<(std::ostream& os, const GenericPlane<FT>& plane);
    template <typename FT>	std::istream& operator>>(std::istream& is, GenericPlane<FT>& plane);


    template <typename FT> inline
    GenericPlane<FT>::GenericPlane(const Point3& p1, const Point3& p2, const Point3& p3) {
        Vector3 n = cross(p2 - p1, p3 - p1);
        n = normalize(n);

        coeff_[0] = n.x;
        coeff_[1] = n.y;
        coeff_[2] = n.z;
        coeff_[3] = -(coeff_[0] * p1.x + coeff_[1] * p1.y + coeff_[2] * p1.z);

        DLOG_IF(ERROR, length(n) < 1e-15) << "degenerate plane constructed from 3 points:"
                                        << "\t(" << p1 << ")"
                                        << "\t(" << p2 << ")"
                                        << "\t(" << p3 << ")";
    }

    template <typename FT> inline
    GenericPlane<FT>::GenericPlane(const Point3& p, const Vector3& n) {
        Vector3 nn = normalize(n);
        coeff_[0] = nn.x;
        coeff_[1] = nn.y;
        coeff_[2] = nn.z;
        coeff_[3] = -(coeff_[0] * p.x + coeff_[1] * p.y + coeff_[2] * p.z);

		DLOG_IF(ERROR, length(nn) < 1e-15) << "degenerate plane constructed from point ("
										   << p << ") and normal (" << n << ")";
	}


    template <typename FT> inline
    typename GenericPlane<FT>::Vector3 GenericPlane<FT>::normal() const {
        Vector3 n = normalize(Vector3(coeff_[0], coeff_[1], coeff_[2]));
        DLOG_IF(ERROR, length(n) < 1e-15) << "degenerate plane with normal: (" << n << ")";
        return n;
    }


    template <typename FT> inline
    typename GenericPlane<FT>::Vector3 GenericPlane<FT>::base1() const {
        if (coeff_[0] == 0)		// parallel to x-axis
            return Vector3(FT(1), FT(0), FT(0));
        else if (coeff_[1] == 0)	// parallel to y-axis
            return Vector3(FT(0), FT(1), FT(0));
        else if (coeff_[2] == 0)	// parallel to z-axis
            return Vector3(FT(0), FT(0), FT(1));
        else {
            Vector3 base(-coeff_[1], coeff_[0], FT(0));
            return normalize(base);
        }
    }


    template <typename FT> inline
    typename GenericPlane<FT>::Vector3 GenericPlane<FT>::base2() const {
        Vector3 base = cross(normal(), base1());
        return normalize(base);
    }

    template <typename FT> inline
    typename GenericPlane<FT>::Point2 GenericPlane<FT>::to_2d(const Point3& p) const {
        Vector3 vec = p - point();
        FT x = dot(vec, base1());
        FT y = dot(vec, base2());
        return Point2(x, y);
    }


    template <typename FT> inline
    typename GenericPlane<FT>::Point3 GenericPlane<FT>::to_3d(const Point2& p) const {
        return point() + base1() * p.x + base2() * p.y;
    }


    template <typename FT> inline
    typename GenericPlane<FT>::Point3 GenericPlane<FT>::point() const {
        Point3 p(FT(0), FT(0), FT(0));
       if (coeff_[0] >= coeff_[1] && coeff_[0] >= coeff_[2])
            p.x = -coeff_[3] / coeff_[0];
        else if (coeff_[1] >= coeff_[0] && coeff_[1] >= coeff_[2])
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
    //   0: the point p is and 0 if the point is belonging the plane.
    template <typename FT> inline
    int GenericPlane<FT>::orient(const Point3& p) const {
        FT v = value(p);
        if (std::abs(v) < 1e-15)
            return 0;

        return (v > 0.0 ? 1 : -1);
    }


    template <typename FT> inline
    typename GenericPlane<FT>::Point3 GenericPlane<FT>::projection(const Point3& p) const {
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


    template <typename FT> inline
    FT GenericPlane<FT>::squared_ditance(const Point3& p) const {
        FT v = value(p);
        return (v * v) / (coeff_[0] * coeff_[0] + coeff_[1] * coeff_[1] + coeff_[2] * coeff_[2]);
    }


    template <typename FT> inline
    bool GenericPlane<FT>::intersect(const Line3& line) const {
        Vector3 dir = line.direction();
        FT c = dot(dir, normal());
        if (std::abs(c) < 1e-15)
            return false;
        else
            return true;
    }


    template <typename FT> inline
    bool GenericPlane<FT>::intersect(const Line3& line, Point3& p) const {
        const Vector3& dir = line.direction();
        FT c = dot(dir, normal());
        if (std::abs(c) < 1e-15)
            return false;

        const Point3& p0 = line.point();
        // p = p0 + dir * t
        // equation: p is in the plane (so we first compute t)
        FT t = -(coeff_[0] * p0.x + coeff_[1] * p0.y + coeff_[2] * p0.z + coeff_[3]) / (coeff_[0] * dir.x + coeff_[1] * dir.y + coeff_[2] * dir.z);
        p = p0 + dir * t;
        return true;
    }


    template <typename FT> inline
    bool GenericPlane<FT>::intersect(const Point3& s, const Point3& t) const {
        int ss = orient(s);
        int st = orient(t);
        if ((ss == 1 && st == -1) || (ss == -1 && st == 1))
            return true;
        else if (ss == 0 || st == 0)
            return true;
        else
            return false;
    }


    template <typename FT> inline
    bool GenericPlane<FT>::intersect(const Point3& s, const Point3& t, Point3& p) const {
        int ss = orient(s);
        int st = orient(t);
        if ((ss == 1 && st == -1) || (ss == -1 && st == 1)) {
            if (intersection(Line3::from_two_points(s, t), p))
                return true;
            else {
                LOG(ERROR) << "fatal error. Should have intersection";
                return false;
            }
        }
        else {
            if (ss == 0) {
                p = s;
                return true;
            }
            else if (st == 0) {
                p = t;
                return true;
            }
            else {
                // no intersection with the plane
                return false;
            }
        }
    }


    template <typename FT> inline
    std::ostream& operator<<(std::ostream& os, const GenericPlane<FT>& plane) {
        return os << plane[0] << ' ' << plane[1] << ' ' << plane[2] << ' ' << plane[3];
    }

    template <typename FT> inline
    std::istream& operator>>(std::istream& is, GenericPlane<FT>& plane) {
        return is >> plane[0] >> plane[1] >> plane[2] >> plane[3];
    }

}


#endif  // EASY3D_PLANE_H
