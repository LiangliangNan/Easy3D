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

#include <easy3d/algo/distance_point_triangle.h>

#include <cfloat>
#include <cmath>


namespace easy3d {

//=============================================================================

    float dist_point_line_segment(const vec3 &p, const vec3 &v0, const vec3 &v1,
                                  vec3 &nearest_point) {
        vec3 d1(p - v0);
        vec3 d2(v1 - v0);
        vec3 min_v(v0);
        float t = dot(d2, d2);

        if (t > FLT_MIN) {
            t = dot(d1, d2) / t;
            if (t > 1.0)
                d1 = p - (min_v = v1);
            else if (t > 0.0)
                d1 = p - (min_v = v0 + d2 * t);
        }

        nearest_point = min_v;
        return norm(d1);
    }

//-----------------------------------------------------------------------------

    float dist_point_triangle(const vec3 &p, const vec3 &v0, const vec3 &v1,
                              const vec3 &v2, vec3 &nearest_point) {
        vec3 v0v1 = v1 - v0;
        vec3 v0v2 = v2 - v0;
        vec3 n = cross(v0v1, v0v2); // not normalized !
        float d = length2(n);

        // Check if the triangle is degenerated -> measure dist to line segments
        if (fabs(d) < FLT_MIN) {
            vec3 q, qq;
            float d, dd(FLT_MAX);

            dd = dist_point_line_segment(p, v0, v1, qq);

            d = dist_point_line_segment(p, v1, v2, q);
            if (d < dd) {
                dd = d;
                qq = q;
            }

            d = dist_point_line_segment(p, v2, v0, q);
            if (d < dd) {
                dd = d;
                qq = q;
            }

            nearest_point = qq;
            return dd;
        }

        float inv_d = 1.0 / d;
        vec3 v1v2 = v2;
        v1v2 -= v1;
        vec3 v0p = p;
        v0p -= v0;
        vec3 t = cross(v0p, n);
        float a = dot(t, v0v2) * -inv_d;
        float b = dot(t, v0v1) * inv_d;
        float s01, s02, s12;

        // Calculate the distance to an edge or a corner vertex
        if (a < 0) {
            s02 = dot(v0v2, v0p) / length2(v0v2);
            if (s02 < 0.0) {
                s01 = dot(v0v1, v0p) / length2(v0v1);
                if (s01 <= 0.0) {
                    v0p = v0;
                } else if (s01 >= 1.0) {
                    v0p = v1;
                } else {
                    (v0p = v0) += (v0v1 *= s01);
                }
            } else if (s02 > 1.0) {
                s12 = dot(v1v2, (p - v1)) / length2(v1v2);
                if (s12 >= 1.0) {
                    v0p = v2;
                } else if (s12 <= 0.0) {
                    v0p = v1;
                } else {
                    (v0p = v1) += (v1v2 *= s12);
                }
            } else {
                (v0p = v0) += (v0v2 *= s02);
            }
        }

            // Calculate the distance to an edge or a corner vertex
        else if (b < 0.0) {
            s01 = dot(v0v1, v0p) / length2(v0v1);
            if (s01 < 0.0) {
                s02 = dot(v0v2, v0p) / length2(v0v2);
                if (s02 <= 0.0) {
                    v0p = v0;
                } else if (s02 >= 1.0) {
                    v0p = v2;
                } else {
                    (v0p = v0) += (v0v2 *= s02);
                }
            } else if (s01 > 1.0) {
                s12 = dot(v1v2, (p - v1)) / length2(v1v2);
                if (s12 >= 1.0) {
                    v0p = v2;
                } else if (s12 <= 0.0) {
                    v0p = v1;
                } else {
                    (v0p = v1) += (v1v2 *= s12);
                }
            } else {
                (v0p = v0) += (v0v1 *= s01);
            }
        }

            // Calculate the distance to an edge or a corner vertex
        else if (a + b > 1.0) {
            s12 = dot(v1v2, (p - v1)) / length2(v1v2);
            if (s12 >= 1.0) {
                s02 = dot(v0v2, v0p) / length2(v0v2);
                if (s02 <= 0.0) {
                    v0p = v0;
                } else if (s02 >= 1.0) {
                    v0p = v2;
                } else {
                    (v0p = v0) += (v0v2 *= s02);
                }
            } else if (s12 <= 0.0) {
                s01 = dot(v0v1, v0p) / length2(v0v1);
                if (s01 <= 0.0) {
                    v0p = v0;
                } else if (s01 >= 1.0) {
                    v0p = v1;
                } else {
                    (v0p = v0) += (v0v1 *= s01);
                }
            } else {
                (v0p = v1) += (v1v2 *= s12);
            }
        }

            // Calculate the distance to an interior point of the triangle
        else {
            n *= (dot(n, v0p) * inv_d);
            (v0p = p) -= n;
        }

        nearest_point = v0p;
        v0p -= p;
        return norm(v0p);
    }

//=============================================================================
} // namespace easy3d
//=============================================================================
