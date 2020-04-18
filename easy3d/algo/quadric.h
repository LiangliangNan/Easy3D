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

#pragma once
//=============================================================================

#include <easy3d/core/types.h>

//=============================================================================

namespace easy3d {

//=============================================================================

//! \addtogroup algorithms algorithms
//!@{

//! This class stores a quadric as a symmetrix 4x4 matrix. Used by the error
//! quadric mesh decimation algorithms.
    class Quadric {
    public: // clang-format off

        //! construct quadric from upper triangle of symmetrix 4x4 matrix
        Quadric(double a, double b, double c, double d,
                double e, double f, double g,
                double h, double i,
                double j)
                : a_(a), b_(b), c_(c), d_(d),
                  e_(e), f_(f), g_(g),
                  h_(h), i_(i),
                  j_(j) {}

        //! constructor quadric from given plane equation: ax+by+cz+d=0
        Quadric(double a = 0.0, double b = 0.0, double c = 0.0, double d = 0.0)
                : a_(a * a), b_(a * b), c_(a * c), d_(a * d),
                  e_(b * b), f_(b * c), g_(b * d),
                  h_(c * c), i_(c * d),
                  j_(d * d) {}

        //! construct from point and normal specifying a plane
        Quadric(const vec3 &n, const vec3 &p) {
            *this = Quadric(n[0], n[1], n[2], -dot(n, p));
        }

        //! set all matrix entries to zero
        void clear() { a_ = b_ = c_ = d_ = e_ = f_ = g_ = h_ = i_ = j_ = 0.0; }

        //! add given quadric to this quadric
        Quadric &operator+=(const Quadric &q) {
            a_ += q.a_;
            b_ += q.b_;
            c_ += q.c_;
            d_ += q.d_;
            e_ += q.e_;
            f_ += q.f_;
            g_ += q.g_;
            h_ += q.h_;
            i_ += q.i_;
            j_ += q.j_;
            return *this;
        }

        //! multiply quadric by a scalar
        Quadric &operator*=(double s) {
            a_ *= s;
            b_ *= s;
            c_ *= s;
            d_ *= s;
            e_ *= s;
            f_ *= s;
            g_ *= s;
            h_ *= s;
            i_ *= s;
            j_ *= s;
            return *this;
        }

        //! evaluate quadric Q at position p by computing (p^T * Q * p)
        double operator()(const vec3 &p) const {
            const double x(p[0]), y(p[1]), z(p[2]);
            return a_ * x * x + 2.0 * b_ * x * y + 2.0 * c_ * x * z + 2.0 * d_ * x
                   + e_ * y * y + 2.0 * f_ * y * z + 2.0 * g_ * y
                   + h_ * z * z + 2.0 * i_ * z
                   + j_;
        }

    private:

        double a_, b_, c_, d_,
                e_, f_, g_,
                h_, i_,
                j_;
    }; // clang-format on

//=============================================================================
//!@}
//=============================================================================
} // namespace easy3d
//=============================================================================
