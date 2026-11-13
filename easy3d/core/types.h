/** \cond LICENSE
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 \endcond */

#ifndef EASY3D_CORE_TYPES_H
#define EASY3D_CORE_TYPES_H


#include <cmath>
#include <vector>
#include <cstdint>

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
#include <easy3d/core/constant.h>

/**
 * Gathers different basic types for geometric operations.
 * Types defined here are points/vectors, lines, segments,
 * planes, boxes and matrices in 2D and 3D.
 */

namespace easy3d {

    //____________________ default types___________________

    /// \brief A 2D point/vector of \p float type.
    typedef Vec<2, float> vec2;
    /// \brief A 3D point/vector of \p float type.
    typedef Vec<3, float> vec3;
    /// \brief A 4D point/vector of \p float type.
    typedef Vec<4, float> vec4;

    /// \brief A 2D point/vector of \p double type.
    typedef Vec<2, double> dvec2;
    /// \brief A 3D point/vector of \p double type.
    typedef Vec<3, double> dvec3;
    /// \brief A 4D point/vector of \p double type.
    typedef Vec<4, double> dvec4;

    /// \brief A 2D point/vector of \p int32_t type.
    typedef Vec<2, int32_t> ivec2;
    /// \brief A 3D point/vector of \p int32_t type.
    typedef Vec<3, int32_t> ivec3;
    /// \brief A 4D point/vector of \p int32_t type.
    typedef Vec<4, int32_t> ivec4;

    /// \brief A 2 by 2 matrix of \p float type.
    typedef Mat2<float> mat2;
    /// \brief A 3 by 3 matrix of \p float type.
    typedef Mat3<float> mat3;
    /// \brief A 4 by 4 matrix of \p float type.
    typedef Mat4<float> mat4;
    /// \brief A 3 by 4 matrix of \p float type.
    typedef Mat<3, 4, float> mat34;
    /// \brief A 4 by 3 matrix of \p float type.
    typedef Mat<4, 3, float> mat43;

    /// \brief A 2 by 2 matrix of \p double type.
    typedef Mat2<double> dmat2;
    /// \brief A 3 by 3 matrix of \p double type.
    typedef Mat3<double> dmat3;
    /// \brief A 4 by 4 matrix of \p double type.
    typedef Mat4<double> dmat4;
    /// \brief A 3 by 4 matrix of \p double type.
    typedef Mat<3, 4, double> dmat34;
    /// \brief A 4 by 3 matrix of \p double type.
    typedef Mat<4, 3, double> dmat43;

    /// \brief A quaternion of \p float type.
    typedef Quat<float> quat;
    /// \brief A quaternion of \p double type.
    typedef Quat<double> dquat;

    /// \brief A 2D line of \p float type.
    typedef GenericLine<2, float> Line2;
    /// \brief A 3D line of \p float type.
    typedef GenericLine<3, float> Line3;

    /// \brief A 3D oriented line of \p float type.
    typedef GenericOrientedLine<float> OrientedLine3;

    /// \brief A 2D line segment of \p float type.
    typedef GenericSegment<2, float> Segment2;
    /// \brief A 3D line segment of \p float type.
    typedef GenericSegment<3, float> Segment3;

    /// \brief A 3D plane of \p float type.
    typedef GenericPlane<float> Plane3;

    /// \brief A 2D axis-aligned bounding box of \p float type.
    typedef GenericBox<2, float> Box2;
    /// \brief A 3D axis-aligned bounding box of \p float type.
    typedef GenericBox<3, float> Box3;

    /// \brief A 2D axis-aligned rectangle of \p float type.
    typedef GenericRect<float> Rect;
    /// \brief A 2D axis-aligned rectangle of \p int32_t type.
    typedef GenericRect<int32_t> iRect;

    /// \brief A 2D polygon of \p float type.
    typedef GenericPolygon<float> Polygon2;


    /// Test whether a floating-point value is non-finite.
    /// \param v Value to test.
    /// \return True for NaN or infinity.
    template <typename FT>
    inline bool is_nan(FT v) {
        return (std::isnan(v) || std::isinf(v));
    }

    /**
     * \brief Clamp a value to an inclusive interval.
     * \param x Value to clamp.
     * \param lower Lower bound.
     * \param upper Upper bound.
     * \return x restricted to [lower, upper].
     */
    template<class T>
    inline T clamp(T x, T lower, T upper) {
        return std::min(upper, std::max(x, lower));
    }

    /**
     * \brief Calculates the next larger power of 2. If the input is already a power of 2, it will return itself.
     * \param a The starting point for finding the next power of 2.
     * \return value^2.
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

    /**
     * \brief Round a floating-point value to a specified number of decimal digits.
     * \param v Value to round.
     * \param num Number of digits after the decimal point; negative values round
     *        to tens, hundreds, and so on.
     * \return Rounded value.
     */
    template<class FT>
    inline FT truncate_digits(const FT &v, int num) {
        FT tmp = std::pow(10.0, num);
        auto des = static_cast<long long>((v < 0) ? (v * tmp - 0.5) : (v * tmp + 0.5));
        return FT(des) / tmp;
    }


    /// \brief Functions for basic geometric computations.
    /// \namespace easy3d::geom
    namespace geom {

        /**
         * \brief Return a vector orthogonal to a 3D vector.
         * \param v Input vector.
         * \return Orthogonal vector; it is zero only when v is zero.
         */
        inline vec3 orthogonal(const vec3 &v) {
            const float absx = std::fabs(v.x);
            const float absy = std::fabs(v.y);
            const float absz = std::fabs(v.z);
            // Find smallest component. Keep equal case for null values.
            if ((absy >= absx) && (absz >= absx))
                return vec3(0.0f, -v.z, v.y);
            else {
                if ((absx >= absy) && (absz >= absy))
                    return vec3(-v.z, 0.0f, v.x);
                else
                    return vec3(-v.y, v.x, 0.0f);
            }
        }

        /**
         * \brief Compute the axis-aligned bounding box of a point range.
         * \tparam Box Bounding-box type compatible with the points.
         * \tparam Container Point container type.
         * \param points Input points.
         * \return Smallest box containing the points; default/empty for no points.
         */
        template<typename Box, typename Container>
        inline Box bounding_box(const Container& points) {
            Box result;
            for (const auto& p : points) {
                result.grow(p);
            }
            return result;
        }

        /**
         * \brief Compute the arithmetic mean of a point range.
         * \tparam Vec Result vector type.
         * \tparam Container Point container type.
         * \param points Input points; must not be empty.
         * \return Arithmetic centroid.
         */
        template<typename Vec, typename Container>
        inline Vec centroid(const Container& points) {
            Vec v(0);
            for (const auto& p : points)
                v += p;
            return v / points.size();
        }

        /// \brief Compute the midpoint of two points.
        /// \param p1 First point.
        /// \param p2 Second point.
        /// \return Arithmetic midpoint.
        template<typename Vec>
        inline Vec barycenter(const Vec &p1, const Vec &p2) {
            return (p1 + p2) * 0.5f;
        }

        /// \brief Compute the arithmetic barycenter of three points.
        /// \param p1 First point.
        /// \param p2 Second point.
        /// \param p3 Third point.
        /// \return Arithmetic barycenter.
        template<typename Vec>
        inline Vec barycenter(const Vec &p1, const Vec &p2, const Vec &p3) {
            return (p1 + p2 + p3) / 3.0f;
        }

        /// \brief Compute the arithmetic barycenter of four points.
        /// \param p1 First point.
        /// \param p2 Second point.
        /// \param p3 Third point.
        /// \param p4 Fourth point.
        /// \return Arithmetic barycenter.
        template<typename Vec>
        inline Vec barycenter(const Vec &p1, const Vec &p2, const Vec &p3, const Vec &p4) {
            return (p1 + p2 + p3 + p4) * 0.25f;
        }

        /** \brief Computes the barycentric coordinates of a point \p p with respect to
         * three points \p u, \p v, and \p w.
         * \param p Point to express.
         * \param u First triangle vertex.
         * \param v Second triangle vertex.
         * \param w Third triangle vertex.
         * \return Three barycentric weights.
         */
        template<typename FT>
        inline Vec<3, FT> barycentric_coordinates(const Vec<3, FT> &p,
                                                  const Vec<3, FT> &u,
                                                  const Vec<3, FT> &v,
                                                  const Vec<3, FT> &w);

        /**
         * \brief Tests if a point \p p lies inside or outside of a \p polygon.
         * This function is robust to handle general polygons (no matter convex or concave).
         * \param p Point to test.
         * \param polygon Ordered polygon vertices.
         * \return True if the point is inside the polygon.
         */
        inline bool point_in_polygon(const vec2 &p, const std::vector<vec2> &polygon);

        /// Clamp a cotangent to the range corresponding approximately to angles [3,177] degrees.
        /// \param v Cotangent value.
        /// \return Clamped cotangent.
        inline double clamp_cot(const double v) {
            const double bound = 19.1; // 3 degrees
            return (v < -bound ? -bound : (v > bound ? bound : v));
        }

        /// Clamp a cosine to avoid angles within approximately 3 degrees of 0 or pi.
        /// \param v Cosine value.
        /// \return Clamped cosine.
        inline double clamp_cos(const double v) {
            const double bound = 0.9986; // 3 degrees
            return (v < -bound ? -bound : (v > bound ? bound : v));
        }

        /// Compute the cosine of the angle between two unnormalized vectors.
        /// \param a First vector.
        /// \param b Second vector.
        /// \return Cosine of their angle.
        template<typename Vec>
        inline double cos_angle(const Vec &a, const Vec &b) {
            return dot(a, b) / std::sqrt(length2(a) * length2(b));
        }

        /// Compute the sine of the angle between two unnormalized vectors.
        /// \param a First vector.
        /// \param b Second vector.
        /// \return Non-negative sine magnitude.
        template<typename Vec>
        inline double sin_angle(const Vec &a, const Vec &b) {
            return norm(cross(a, b)) / (norm(a) * norm(b));
        }

        /// Compute a clamped cotangent of the angle between two vectors.
        /// \param a First vector.
        /// \param b Second vector.
        /// \return Clamped cotangent.
        template<typename Vec>
        inline typename Vec::FT cotan_angle(const Vec &a, const Vec &b) {
            return clamp_cot(dot(a, b) / norm(cross(a, b)));
        }

        /// Compute the angle between two unnormalized vectors.
        /// \param a First vector.
        /// \param b Second vector.
        /// \return Angle in radians in [0,pi].
        template<typename Vec>
        inline double angle(const Vec &a, const Vec &b) {
            return std::atan2(norm(cross(a, b)), dot(a, b));
        }

        /// Convert degrees to radians.
        /// \param degrees Angle in degrees.
        /// \return Angle in radians.
        template<typename FT>
        inline FT to_radians(FT degrees) {
            return degrees * static_cast<FT>(0.01745329251994329576923690768489);
        }

        /// Convert radians to degrees.
        /// \param radians Angle in radians.
        /// \return Angle in degrees.
        template<typename FT>
        inline FT to_degrees(FT radians) {
            return radians * static_cast<FT>(57.295779513082320876798154814105);
        }

        /// Compute the unsigned area of a 3D triangle.
        /// \param p1 First vertex.
        /// \param p2 Second vertex.
        /// \param p3 Third vertex.
        /// \return Triangle area.
        inline float triangle_area(const vec3 &p1, const vec3 &p2, const vec3 &p3) {
            return 0.5f * length(cross(p2 - p1, p3 - p1));
        }

        /// Compute the signed area of a 2D triangle.
        /// \param p1 First vertex.
        /// \param p2 Second vertex.
        /// \param p3 Third vertex.
        /// \return Signed area; sign follows the vertex orientation.
        inline float triangle_signed_area(const vec2 &p1, const vec2 &p2, const vec2 &p3) {
            return 0.5f * det(p2 - p1, p3 - p1);
        }

        /// Compute the normalized normal of a 3D triangle.
        /// \param p1 First vertex.
        /// \param p2 Second vertex.
        /// \param p3 Third vertex.
        /// \return Unit normal following the vertex winding, or zero for degeneracy.
        inline vec3 triangle_normal(const vec3 &p1, const vec3 &p2, const vec3 &p3) {
            vec3 n = cross(p2 - p1, p3 - p2);
            return normalize(n);
        }

        /**
         * \brief Compute the distance from a point to a 3D line segment.
         * \param p Point to test.
         * \param v0 First segment endpoint.
         * \param v1 Second segment endpoint.
         * \param nearest_point Receives the closest point on the segment.
         * \return Euclidean distance.
         */
        inline float dist_point_line_segment(const vec3 &p, const vec3 &v0, const vec3 &v1, vec3 &nearest_point);

        /**
         * \brief Compute the distance from a point to a 3D triangle.
         * \param p Point to test.
         * \param v0 First triangle vertex.
         * \param v1 Second triangle vertex.
         * \param v2 Third triangle vertex.
         * \param nearest_point Receives the closest point on the triangle.
         * \return Euclidean distance.
         */
        inline float
        dist_point_triangle(const vec3 &p, const vec3 &v0, const vec3 &v1, const vec3 &v2, vec3 &nearest_point);

        /**
         * \brief Compute the circumcenter of a tetrahedron.
         * \param p First vertex.
         * \param q Second vertex.
         * \param r Third vertex.
         * \param s Fourth vertex.
         * \return Center of the sphere through all four vertices.
         */
        inline vec3 tetra_circum_center(const vec3& p, const vec3& q, const vec3& r, const vec3& s);

    } // namespace geom


    namespace color {

        // Encode a color into an integer

        /// \brief Encodes an RGB (each component in the range [0, 255]) color in an integer value.
        inline int encode(int r, int g, int b) {
            return (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
        }

        /// \brief Encodes an RGBA (each component in the range [0, 255]) color in an integer value.
        inline int encode(int r, int g, int b, int a) {
            return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
        }

        // Encode an integer into color components

        /// \brief Decodes an integer value as RGB color (each component in the range [0, 255]).
        inline void decode(int value, int &r, int &g, int &b) {
            r = ((value >> 16) & 0xff);
            g = ((value >> 8) & 0xff);
            b = (value & 0xff);
        }

        /// \brief Decodes an integer value as RGBA color (each component in the range [0, 255]).
        inline void decode(int value, int &r, int &g, int &b, int &a) {
            r = ((value >> 16) & 0xff);
            g = ((value >> 8) & 0xff);
            b = (value & 0xff);
            a = (value >> 24);
        }


        // Given a color encoded as an integer, get the color component

        /// \brief Gets the red component of RGB. [0, 255]
        inline int red(int color) { return ((color >> 16) & 0xff); }

        /// \brief Gets the green component of RGB. [0, 255]
        inline int green(int color) { return ((color >> 8) & 0xff); }

        /// \brief Gets the blue component of RGB. [0, 255]
        inline int blue(int color) { return (color & 0xff); }

        /// \brief Gets the alpha component of RGBA. [0, 255]
        inline int alpha(int color) { return color >> 24; }

    }   // namespace color

}   // namespace easy3d


//=============================================================================


// \cond
namespace easy3d {

    namespace geom {

        template<typename FT>
        inline Vec<3, FT> barycentric_coordinates(const Vec<3, FT> &p,
                                                  const Vec<3, FT> &u,
                                                  const Vec<3, FT> &v,
                                                  const Vec<3, FT> &w) {
            Vec<3, FT> result(1.0 / 3.0); // default: barycenter

            Vec<3, FT> vu = v - u, wu = w - u, pu = p - u;

            // find the largest absolute coordinate of normal
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
                default:
                    break;
            }

            return result;
        }

        //-----------------------------------------------------------------------------

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

        //-----------------------------------------------------------------------------

        inline float dist_point_line_segment(const vec3 &p, const vec3 &v0, const vec3 &v1, vec3 &nearest_point) {
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

        inline float
        dist_point_triangle(const vec3 &p, const vec3 &v0, const vec3 &v1, const vec3 &v2, vec3 &nearest_point) {
            vec3 v0v1 = v1 - v0;
            vec3 v0v2 = v2 - v0;
            vec3 n = cross(v0v1, v0v2); // not normalized !
            float len2 = length2(n);

            // Check if the triangle is degenerated -> measure dist to line segments
            if (fabs(len2) < FLT_MIN) {
                vec3 q, qq;
                float dd = dist_point_line_segment(p, v0, v1, qq);
                float d = dist_point_line_segment(p, v1, v2, q);
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

            float inv_d = 1.0f / len2;
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


        inline vec3 tetra_circum_center(const vec3 &p, const vec3 &q, const vec3 &r, const vec3 &s) {
            vec3 qp = q - p;
            float qp2 = length2(qp);
            vec3 rp = r - p;
            float rp2 = length2(rp);
            vec3 sp = s - p;
            float sp2 = length2(sp);

            double num_x = determinant(mat3(
                    qp.y, qp.z, qp2,
                    rp.y, rp.z, rp2,
                    sp.y, sp.z, sp2)
            );
            double num_y = determinant(mat3(
                    qp.x, qp.z, qp2,
                    rp.x, rp.z, rp2,
                    sp.x, sp.z, sp2)
            );
            double num_z = determinant(mat3(
                    qp.x, qp.y, qp2,
                    rp.x, rp.y, rp2,
                    sp.x, sp.y, sp2)
            );
            double den = determinant(mat3(
                    qp.x, qp.y, qp.z,
                    rp.x, rp.y, rp.z,
                    sp.x, sp.y, sp.z)
            );

            assert(std::abs(den) > 1e-30f);

            den *= 2.0f;

            return vec3(
                    p.x + float(num_x / den),
                    p.y - float(num_y / den),
                    p.z + float(num_z / den)
            );

        }

    }   // namespace geom

}   // namespace easy3d
// \endcond

#endif  // EASY3D_CORE_TYPES_H

