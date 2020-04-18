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

#ifndef EASY3D_TYPES_H
#define EASY3D_TYPES_H

#include <cstdint>
#include <vector>

#include <easy3d/core/vec.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/rect.h>
#include <easy3d/core/line.h>
#include <easy3d/core/oriented_line.h>
#include <easy3d/core/segment.h>
#include <easy3d/core/plane.h>
#include <easy3d/core/box.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/polygon.h>


/**
* Gathers different basic types for geometric operations.
* Types defined here are points/vectors, lines, segments,
* planes, boxes and matrices in 2D and 3D.
*/

namespace easy3d {

    //____________________ default types___________________

    typedef Vec<2, float> vec2;
    typedef Vec<3, float> vec3;
    typedef Vec<4, float> vec4;

    typedef Vec<2, double> dvec2;
    typedef Vec<3, double> dvec3;
    typedef Vec<4, double> dvec4;

    typedef Vec<2, int32_t> ivec2;
    typedef Vec<3, int32_t> ivec3;
    typedef Vec<4, int32_t> ivec4;

    typedef Mat2<float> mat2;
    typedef Mat3<float> mat3;
    typedef Mat4<float> mat4;
    typedef Mat<3, 4, float> mat34;
    typedef Mat<4, 3, float> mat43;

    typedef Mat2<double> dmat2;
    typedef Mat3<double> dmat3;
    typedef Mat4<double> dmat4;
    typedef Mat<3, 4, double> dmat34;
    typedef Mat<4, 3, double> dmat43;

    typedef Quat<float> quat;
    typedef Quat<double> dquat;

    typedef GenericLine<2, float> Line2;
    typedef GenericLine<3, float> Line3;

    typedef GenericOrientedLine<float> OrientedLine3;

    typedef GenericSegment<2, float> Segment2;
    typedef GenericSegment<3, float> Segment3;

    typedef GenericPlane<float> Plane3;

    typedef GenericBox<2, float> Box2;
    typedef GenericBox<3, float> Box3;

    typedef GenericRect<float> Rect;
    typedef GenericRect<int32_t> iRect;

    typedef GenericPolygon<float> Polygon2;


    namespace geom {

        /* Returns a vector orthogonal to v. Its norm() depends on v, but is zero only for a null v.*/
        inline vec3 orthogonal(const vec3 &v) {
            const float absx = std::fabs(v.x);
            const float absy = std::fabs(v.y);
            const float absz = std::fabs(v.z);
            // Find smallest component. Keep equal case for null values.
            if ((absy >= absx) && (absz >= absx))
                return vec3(0.0, -v.z, v.y);
            else {
                if ((absx >= absy) && (absz >= absy))
                    return vec3(-v.z, 0.0f, v.x);
                else
                    return vec3(-v.y, v.x, 0.0f);
            }
        }

        // bounding box for 3D/2D point set
        template<typename Box, typename InputIterator>
        inline Box bounding_box(InputIterator first, InputIterator last) {
            assert(first != last);
            Box result;
            for (InputIterator it = first; it != last; ++it) {
                result.add_point(*it);
            }
            return result;
        }

        // centroid for 3D/2D point set
        template<typename Vec, typename InputIterator>
        inline Vec centroid(InputIterator begin, InputIterator end) {
            assert(begin != end);
            Vec v;
            unsigned int nb_pts = 0;
            for (InputIterator it = begin; it != end; ++it) {
                v += (*it);
                ++nb_pts;
            }
            return v / nb_pts;
        }

        template<typename Vec>
        inline Vec barycenter(const Vec &p1, const Vec &p2) {
            return (p1 + p2) * 0.5f;
        }

        template<typename Vec>
        inline Vec barycenter(const Vec &p1, const Vec &p2, const Vec &p3) {
            return (p1 + p2 + p3) / 3.0f;
        }

        template<typename Vec>
        inline Vec barycenter(const Vec &p1, const Vec &p2, const Vec &p3, const Vec &p4) {
            return (p1 + p2 + p3 + p4) * 0.25f;
        }

        template<typename FT>
        const Vec<3, FT> barycentric_coordinates(const Vec<3, FT> &p,
                                                 const Vec<3, FT> &u,
                                                 const Vec<3, FT> &v,
                                                 const Vec<3, FT> &w) {
            Vec<3, FT> result(1.0 / 3.0); // default: barycenter

            Vec<3, FT> vu = v - u, wu = w - u, pu = p - u;

            // find largest absolute coordinate of normal
            FT nx = vu[1] * wu[2] - vu[2] * wu[1],
                    ny = vu[2] * wu[0] - vu[0] * wu[2],
                    nz = vu[0] * wu[1] - vu[1] * wu[0], ax = fabs(nx), ay = fabs(ny),
                    az = fabs(nz);

            unsigned char maxCoord;

            if (ax > ay) {
                if (ax > az)
                    maxCoord = 0;
                else
                    maxCoord = 2;
            } else {
                if (ay > az)
                    maxCoord = 1;
                else
                    maxCoord = 2;
            }

            // solve 2D problem
            switch (maxCoord) {
                case 0: {
                    if (1.0 + ax != 1.0) {
                        result[1] = 1.0 + (pu[1] * wu[2] - pu[2] * wu[1]) / nx - 1.0;
                        result[2] = 1.0 + (vu[1] * pu[2] - vu[2] * pu[1]) / nx - 1.0;
                        result[0] = 1.0 - result[1] - result[2];
                    }
                    break;
                }

                case 1: {
                    if (1.0 + ay != 1.0) {
                        result[1] = 1.0 + (pu[2] * wu[0] - pu[0] * wu[2]) / ny - 1.0;
                        result[2] = 1.0 + (vu[2] * pu[0] - vu[0] * pu[2]) / ny - 1.0;
                        result[0] = 1.0 - result[1] - result[2];
                    }
                    break;
                }

                case 2: {
                    if (1.0 + az != 1.0) {
                        result[1] = 1.0 + (pu[0] * wu[1] - pu[1] * wu[0]) / nz - 1.0;
                        result[2] = 1.0 + (vu[0] * pu[1] - vu[1] * pu[0]) / nz - 1.0;
                        result[0] = 1.0 - result[1] - result[2];
                    }
                    break;
                }
            }

            return result;
        }

        // NOTE: works for both convex and non-convex polygons.
        inline bool point_in_polygon(const vec2 &p, const std::vector<vec2> &polygon) {
            bool inside = false;
            std::size_t n = polygon.size();
            for (std::size_t i = 0, j = n - 1; i < n; j = i, ++i) {
                const vec2 &u0 = polygon[i];
                const vec2 &u1 = polygon[j];  // current edge

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

        /** clamp cotangent values as if angles are in [1, 179]    */
        inline double clamp_cot(const double v) {
            const double bound = 19.1; // 3 degrees
            return (v < -bound ? -bound : (v > bound ? bound : v));
        }

        /** clamp cosine values as if angles are in [1, 179]    */
        inline double clamp_cos(const double v) {
            const double bound = 0.9986; // 3 degrees
            return (v < -bound ? -bound : (v > bound ? bound : v));
        }

        /** compute cosine of angle between two (un-normalized) vectors */
        template<typename Vec>
        inline double cos_angle(const Vec &a, const Vec &b) {
            return dot(a, b) / std::sqrt(length2(a) * length2(b));
        }

        /** compute sine of angle between two (un-normalized) vectors */
        template<typename Vec>
        inline double sin_angle(const Vec &a, const Vec &b) {
            return norm(cross(a, b)) / (norm(a) * norm(b));
        }

        /** compute cotangent of angle between two (un-normalized) vectors */
        template<typename Vec>
        inline double cot_angle(const Vec &a, const Vec &b) {
            return clamp_cot(dot(a, b) / norm(cross(a, b)));
        }

        /** compute angle between two (un-normalized) vectors */
        template<typename Vec>
        inline double angle(const Vec &a, const Vec &b) {
            return atan2(norm(cross(a, b)), dot(a, b));
        }

        // radians
        template<typename T>
        inline T to_radians(T degrees) {
            return degrees * static_cast<T>(0.01745329251994329576923690768489);
        }


        // degrees
        template<typename T>
        inline T to_degrees(T radians) {
            return radians * static_cast<T>(57.295779513082320876798154814105);
        }

        /**
         * @brief Calculates the next larger power of 2. If the input is already a power of 2, it will return itself.
         * @param a The starting point for finding the next power of 2.
         * @return value^2.
         * Example:
         *      next_pow2(50);  // returns 64
         *      next_pow2(64);  // returns 64
         *      next_pow2(401); // returns 512
         */
        inline int next_pow2(int a) {
            int rval = 1;
            while (rval < a) rval <<= 1;
            return rval;
        }

        /** compute area of a triangle given by three points */
        inline float triangle_area(const vec3 &p1, const vec3 &p2, const vec3 &p3) {
            return 0.5f * length(cross(p2 - p1, p3 - p1));
        }

        inline float triangle_signed_area(const vec2 &p1, const vec2 &p2, const vec2 &p3) {
            return 0.5f * det(p2 - p1, p3 - p1);
        }

        inline vec3 triangle_normal(const vec3 &p1, const vec3 &p2, const vec3 &p3) {
            vec3 n = cross(p2 - p1, p3 - p2);
            return normalize(n);
        }

        // round the given floating point number v to be num_digits.
        // TODO: this function should not be in this file.
        template<class T>
        T truncate_digits(const T &v, int num_digits) {
            T tmp = std::pow(10.0f, num_digits);
            long long des = static_cast<long long>((v < 0) ? (v * tmp - 0.5f) : (v * tmp + 0.5f));
            T result = T(des) / tmp;
            return result;
        }

    } // namespace Geom


    namespace rgb {

        //------- conversion between a RGB(A) color and an integer ----------

        // get red part of RGB. [0, 255]
        inline int red(int color) { return ((color >> 16) & 0xff); }

        // get green part of RGB. [0, 255]
        inline int green(int color) { return ((color >> 8) & 0xff); }

        // get blue part of RGB. [0, 255]
        inline int blue(int color) { return (color & 0xff); }

        // get alpha part of RGBA. [0, 255]
        inline int alpha(int color) { return color >> 24; }

        // set RGB value. [0, 255]
        inline int rgb(int r, int g, int b) {
            return (0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
        }

        // set RGBA value. [0, 255]
        inline int rgba(int r, int g, int b, int a) {
            return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
        }

        // convert R,G,B to gray [0, 255]
        inline int gray(int r, int g, int b) {
            return (r * 11 + g * 16 + b * 5) / 32;
        }

        // encode an integer value as R,G,B color. [0, 255]
        inline void encode(int value, int &r, int &g, int &b) {
            r = ((value >> 16) & 0xff);
            g = ((value >> 8) & 0xff);
            b = (value & 0xff);
        }

        // encode an integer value as R,G,B,A color. [0, 255]
        inline void encode(int value, int &r, int &g, int &b, int &a) {
            r = ((value >> 16) & 0xff);
            g = ((value >> 8) & 0xff);
            b = (value & 0xff);
            a = (value >> 24);
        }
    }

}

#endif  // EASY3D_TYPES_H


