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

#ifndef EASY3D_CORE_CURVE_H
#define EASY3D_CORE_CURVE_H


#include <vector>


namespace easy3d {

    /**
     * \brief Algorithms for evaluating curves.
     * \namespace easy3d::curve
     */
    namespace curve {

        /**
         * \brief Computes a quadratic Bézier curve using De Casteljau’s algorithm.
         *
         * This function evaluates a second-degree Bézier curve (also known as a conic curve) given three control
         * points: \p A, \p B, and \p C. The computed curve can be used for smooth interpolation between points in
         * both 2D and 3D spaces, depending on the dimensionality of the `Point` type.
         *
         * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
         *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
         *      `T` is the data type.
         * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
         * \tparam T The scalar type (e.g., `float` or `double`).
         *
         * \param[in] A The first control point (start of the curve).
         * \param[in] B The second control point (influences the curvature).
         * \param[in] C The third control point (end of the curve).
         * \param[out] curve A vector storing the computed points along the curve.
         * \param[in] bezier_steps The number of segments used to approximate the curve. A higher value results in a
         *      smoother curve at the cost of more points. Suggested default is 4.
         * \param[in] include_end If \c true, the endpoint \p C is included in the output (i.e., extend the curve to
         *      the end point).
         *
         * \details The Bézier curve is computed using De Casteljau’s algorithm, which recursively  interpolates
         *      between the control points. The interpolation formula is:
         *
         *      \f[
         *          P(t) = (1 - t)^2 A + 2(1 - t)t B + t^2 C, \quad t \in [0,1]
         *      \f]
         *
         *      The function iterates from \f$ t = 0 \f$ to \f$ t = 1 \f$ in steps of \f$ 1/\text{bezier_steps} \f$,
         *      generating points along the curve.
         *
         * The following example computes and visualizes a quadratic Bézier curve:
         * \code
         * {
         *     vec3 A(0, 0, 0);
         *     vec3 B(800, 0, 0);
         *     vec3 C(800, 800, 0);
         *     unsigned int steps = 20;
         *
         *     std::vector<vec3> curvePoints;
         *     curve::quadratic(A, B, C, curvePoints, steps);
         *
         *     std::cout << "First point: " << curvePoints.front()
         *               << ", Last point: " << curvePoints.back() << std::endl;
         *
         *     std::vector<unsigned int> indices;
         *     for (unsigned int i = 0; i < curvePoints.size() - 1; ++i) {
         *         indices.push_back(i);
         *         indices.push_back(i + 1);
         *     }
         *
         *     LinesDrawable *curveDrawable = new LinesDrawable;
         *     curveDrawable->update_vertex_buffer(curvePoints);
         *     curveDrawable->update_element_buffer(indices);
         *     curveDrawable->set_impostor_type(LinesDrawable::CYLINDER);
         *     curveDrawable->set_line_width(5);
         *     curveDrawable->set_uniform_coloring(vec3(0, 0, 1, 1));
         *     viewer.add_drawable(curveDrawable);
         * }
         * \endcode
         */
        template <template <size_t, class> class Point, size_t N, typename T>
        void quadratic(const Point<N, T>& A, const Point<N, T>& B, const Point<N, T>& C,
                       std::vector<Point<N, T>>& curve, unsigned int bezier_steps = 4,
                       bool include_end = false)
        {
            using Point_t = Point<N, T>;
            for (unsigned int i = 0; i <= bezier_steps; i++) {   // i <= bezier_steps: to fully sample the curve
                const T t = static_cast<T>(i) / bezier_steps;
                const Point_t U = (1.0 - t) * A + t * B;
                const Point_t V = (1.0 - t) * B + t * C;
                curve.push_back((1.0 - t) * U + t * V);
            }

            if (include_end)
                curve.push_back(C);
        }

        /**
         * \brief Evaluates a cubic Bézier curve using De Casteljau’s algorithm.
         *
         * Given four control points \p A, \p B, \p C, and \p D, this function computes a sequence of points that
         * approximate the cubic Bézier curve defined by these control points. The method works in both 2D and 3D
         * spaces, depending on the template instantiation of \p Point.
         *
         * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
         *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
         *      `T` is the data type.
         * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
         * \tparam T The scalar type (e.g., `float` or `double`).
         *
         * \param[in] A The first control point (curve start).
         * \param[in] B The second control point (controls curvature).
         * \param[in] C The third control point (controls curvature).
         * \param[in] D The fourth control point (curve end).
         * \param[out] curve  Output vector that will store the computed points along the Bézier curve.
         * \param[in] bezier_steps The number of subdivisions used to approximate the curve. A higher value results in a
         *      smoother curve but increases computation time. The default value is 4.
         * \param[in] include_end If true, the function ensures that the last point in the curve is exactly \p D.
         *
         * The following code demonstrates how to visualize a cubic Bézier curve as a polyline:
         * \code
         * {
         *     vec3 a(0, 0, 0);
         *     vec3 b(400, 0, 0);
         *     vec3 c(400, 800, 0);
         *     vec3 d(800, 800, 0);
         *     unsigned int steps = 20;
         *
         *     std::vector<vec3> points;
         *     curve::cubic(a, b, c, d, points, steps);
         *     std::cout << "First point: " << points.front() << ", Last point: " << points.back() << std::endl;
         *
         *     std::vector<unsigned int> indices;
         *     for (unsigned int i = 0; i < points.size() - 1; ++i) {
         *         indices.push_back(i);
         *         indices.push_back(i + 1);
         *     }
         *     LinesDrawable *curve = new LinesDrawable;
         *     curve->update_vertex_buffer(points);
         *     curve->update_element_buffer(indices);
         *     curve->set_impostor_type(LinesDrawable::CYLINDER);
         *     curve->set_line_width(5);
         *     curve->set_uniform_coloring(vec3(0, 1, 0, 1));
         *     viewer.add_drawable(curve);
         * }
         * \endcode
         */
        template <template <size_t, class> class Point, size_t N, typename T>
        void cubic(const Point<N, T> &A, const Point<N, T> &B, const Point<N, T> &C, const Point<N, T> &D,
                   std::vector<Point<N, T>> &curve,
                   unsigned int bezier_steps = 4,
                   bool include_end = false)
        {
            using Point_t = Point<N, T>;
            for (unsigned int i = 0; i <= bezier_steps; i++) {  // i <= bezier_steps: to fully sample the curve
                const T t = static_cast<T>(i) / bezier_steps;
                const Point_t U = (1.0 - t) * A + t * B;
                const Point_t V = (1.0 - t) * B + t * C;
                const Point_t W = (1.0 - t) * C + t * D;
                const Point_t P = (1.0 - t) * U + t * V;
                const Point_t Q = (1.0 - t) * V + t * W;
                curve.push_back((1.0 - t) * P + t * Q);
            }
            if (include_end)
                curve.push_back(D);
        }

    }

    // =============================================================================


    /**
   * \brief Abstract base class for curve fitting/interpolation.
   * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
   *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
   *      `T` is the data type.
   * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
   * \tparam T The scalar type (e.g., `float` or `double`).
   * \class Curve easy3d/core/curve.h
   * \sa Bezier, BSpline, and CatmullRom
   */
    template<template <size_t, class> class Point, size_t N, typename T>
    class Curve {
    public:
        /// The type of points.
        using Point_t = Point<N, T>;

        /**
         * \brief Generates a curve based on the given control points.
         * \param control_points A vector of control points that define the curve.
         * \param num_samples The number of samples to generate along the curve.
         * \return A vector of points representing the generated curve.
         */
        virtual std::vector<Point_t> generate(const std::vector<Point_t> &control_points, size_t num_samples) const = 0;
    };

    /**
     * \brief Class for Bezier curve fitting.
     *
     * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
     *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
     *      `T` is the data type.
     * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
     * \tparam T The scalar type (e.g., `float` or `double`).
     *
     * Example:
     *  \code
     *      Bezier<Vec, 3, float> curve;
     *      const auto points = curve.generate(control_points, 100);
     *  \endcode
     * \class Bezier easy3d/core/curve.h
     * \see Curve, BSpline, CatmullRom
     */
    template<template <size_t, class> class Point, size_t N, typename T>
    class Bezier : public Curve<Point, N, T> {
    public:
        /// The type of points.
        using Point_t = Point<N, T>;

        /// Default constructor
        Bezier() = default;

        /**
         * \brief Generates a curve based on the given control points.
         * \param control_points A vector of control points that define the curve.
         * \param num_samples The number of samples to generate along the curve.
         * \return A vector of points representing the generated curve.
         */
        std::vector<Point_t> generate(const std::vector<Point_t> &control_points, size_t num_samples) const override {
            std::vector<Point_t> curve;
            curve.reserve(num_samples + 1); // Reserve space to avoid reallocations

            for (size_t i = 0; i <= num_samples; ++i) {
                T t = static_cast<T>(i) / num_samples;
                curve.push_back(interpolate(control_points, t));
            }
            return curve;
        }

    private:
        Point_t interpolate(const std::vector<Point_t> &control_points, T t) const {
            std::vector<Point_t> points = control_points; // Copy control points
            size_t n = points.size();
            for (size_t k = 1; k < n; ++k) {
                for (size_t i = 0; i < n - k; ++i) {
                    points[i] = points[i] * (1 - t) + points[i + 1] * t;
                }
            }
            return points[0];
        }
    };

    /**
     * \brief Class for BSpline curve fitting.
     *
     * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
     *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
     *      `T` is the data type.
     * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
     * \tparam T The scalar type (e.g., `float` or `double`).
     *
     * Example:
     *  \code
     *      BSpline<Vec, 3, float> curve;
     *      const auto points = curve.generate(control_points, 100);
     *  \endcode
     * \class BSpline easy3d/core/curve.h
     * \see Curve, Bezier, CatmullRom
     */
    template<template <size_t, class> class Point, size_t N, typename T>
    class BSpline : public Curve<Point, N, T> {
    public:
        /// The type of points.
        using Point_t = Point<N, T>;

        /// Default constructor.
        BSpline() = default;

        /**
         * \brief Generates a curve based on the given control points.
         * \param control_points A vector of control points that define the curve.
         * \param num_samples The number of samples to generate along the curve.
         * \return A vector of points representing the generated curve.
         */
        std::vector<Point_t> generate(const std::vector<Point_t> &control_points, size_t num_samples) const override {
            std::vector<Point_t> curve;
            size_t n = control_points.size();
            if (n < 4)
                return curve;

            curve.reserve(num_samples + 1); // Reserve space to avoid reallocations

            // Sample the curve
            for (size_t i = 0; i <= num_samples; ++i) {
                T t = static_cast<T>(i) / num_samples * (n - 3);
                size_t k = static_cast<size_t>(t);
                t -= k;

                // Ensure k is within the valid range
                if (k >= n - 3) {
                    k = n - 4;
                    t = 1.0;
                }

                // Evaluate using a proper B-spline basis
                curve.push_back(interpolate(control_points, t, k));
            }
            return curve;
        }

    private:
        Point_t interpolate(const std::vector<Point_t> &control_points, T t, size_t k) const {
            const T one_sixth = static_cast<T>(1) / 6;
            Point_t P0 = control_points[k];
            Point_t P1 = control_points[k + 1];
            Point_t P2 = control_points[k + 2];
            Point_t P3 = control_points[k + 3];

            T t2 = t * t;
            T t3 = t2 * t;

            return (P0 * (-t3 + 3 * t2 - 3 * t + 1) * one_sixth) +
                   (P1 * (3 * t3 - 6 * t2 + 4) * one_sixth) +
                   (P2 * (-3 * t3 + 3 * t2 + 3 * t + 1) * one_sixth) +
                   (P3 * (t3) * one_sixth);
        }
    };

    /**
     * \brief Class for uniform Catmull-Rom curve interpolation (N-dimensional).
     *
     * Implements the **default (uniform)** Catmull-Rom spline with tension 0.5, i.e. the
     * classic cubic basis
     * \f[
     *   P(u) = \tfrac{1}{2}\bigl[\,
     *     2P_1 + (-P_0 + P_2)\,u + (2P_0 - 5P_1 + 4P_2 - P_3)\,u^2
     *            + (-P_0 + 3P_1 - 3P_2 + P_3)\,u^3 \,\bigr].
     * \f]
     *
     * The curve passes exactly through every keyframe, including the first and last.
     * Endpoints are handled by the standard Catmull-Rom **endpoint-duplication** trick
     * (`P_{-1} = P_0`, `P_n = P_{n-1}`); this is valid for the uniform variant because
     * its basis does not depend on inter-keyframe distances.
     *
     * \note The uniform variant does not adapt its parametrization to keyframe spacing.
     *       For strongly unevenly spaced keyframes it can exhibit cusps/overshoot; use
     *       the centripetal variant in that case.
     *
     * \tparam Point Point class parameterized as `Point<N, T>`, supporting `+`, `-`, and
     *      right-scalar `*`.
     * \tparam N Number of dimensions (2, 3, 4, ...).
     * \tparam T Scalar type (`float` or `double`).
     *
     * Example:
     *  \code
     *      CatmullRom<Vec, 3, float> curve;
     *      const auto points = curve.generate(control_points, 200);
     *  \endcode
     *
     * \class CatmullRom easy3d/core/curve.h
     * \see Curve, Bezier, BSpline
     */
    template<template <size_t, class> class Point, size_t N, typename T>
    class CatmullRom : public Curve<Point, N, T> {
    public:
        /// The type of points.
        using Point_t = Point<N, T>;

        /// Default constructor.
        CatmullRom() = default;

        /**
         * \brief Generates a uniform Catmull-Rom curve through the given keyframes.
         *
         * The returned polyline passes exactly through every element of \p control_points,
         * in order, starting at `control_points.front()` and ending at
         * `control_points.back()`. Sampling is per segment with a guaranteed minimum
         * resolution, so no segment collapses to a single straight step.
         *
         * \param control_points Keyframes to interpolate; requires at least 2.
         * \param num_samples Target total sample count; a floor of 16 samples per segment
         *        is enforced regardless.
         * \return Points along the curve, or an empty vector if fewer than 2 keyframes.
         */
        std::vector<Point_t> generate(const std::vector<Point_t> &control_points,
                                      size_t num_samples) const override {
            std::vector<Point_t> curve;
            if (control_points.size() < 2)
                return curve;

            // Two keyframes: straight segment, uniformly sampled (correct limit).
            if (control_points.size() == 2) {
                const size_t samples = std::max<size_t>(1, num_samples);
                curve.reserve(samples + 1);
                const Point_t &A = control_points.front();
                const Point_t &B = control_points.back();
                for (size_t s = 0; s <= samples; ++s) {
                    const T u = static_cast<T>(s) / static_cast<T>(samples);
                    curve.push_back(A * (T(1) - u) + B * u);
                }
                return curve;
            }

            const size_t m = control_points.size();

            // Standard uniform-CR endpoint handling: duplicate the first and last points.
            // (Valid only for the uniform variant; centripetal CR needs distinct points.)
            std::vector<Point_t> P;
            P.reserve(m + 2);
            P.push_back(control_points.front());                                 // P_{-1}
            P.insert(P.end(), control_points.begin(), control_points.end());     // P_0..P_{m-1}
            P.push_back(control_points.back());                                  // P_m

            const size_t segments = P.size() - 3;   // == m - 1
            const size_t per_seg  = std::max<size_t>(
                16, (num_samples + segments - 1) / segments);

            curve.reserve(segments * per_seg + 1);

            for (size_t i = 0; i < segments; ++i) {
                for (size_t s = 0; s < per_seg; ++s) {
                    const T u = static_cast<T>(s) / static_cast<T>(per_seg);
                    curve.push_back(interpolate_uniform(P, i, u));
                }
            }
            // Append the final real keyframe exactly.
            curve.push_back(control_points.back());

            return curve;
        }

    private:
        /**
         * \brief Uniform Catmull-Rom evaluation of one segment.
         *
         * Evaluates the segment whose middle two control points are `P[i+1]` and `P[i+2]`.
         * `u == 0` returns `P[i+1]`; `u == 1` returns `P[i+2]`.
         *
         * The basis has constant denominators (the implicit knot spacing is uniform), so
         * no distance-dependent machinery is required. All arithmetic is per-component and
         * uses only `+`, `-`, and right-scalar `*`, so it is valid for any dimension N.
         */
        static Point_t interpolate_uniform(const std::vector<Point_t> &P,
                                           size_t i, T u) {
            const Point_t &P0 = P[i];
            const Point_t &P1 = P[i + 1];
            const Point_t &P2 = P[i + 2];
            const Point_t &P3 = P[i + 3];

            const T u2 = u * u;
            const T u3 = u2 * u;
            const T half = static_cast<T>(0.5);

            // P(u) = 0.5 * [ 2P1 + (-P0+P2)u + (2P0-5P1+4P2-P3)u^2 + (-P0+3P1-3P2+P3)u^3 ]
            return ( P1 * T(2)
                   + (P2 - P0) * u
                   + (P0 * T(2) - P1 * T(5) + P2 * T(4) - P3) * u2
                   + (P1 * T(3) - P0 - P2 * T(3) + P3) * u3 ) * half;
        }
    };

}   // namespace easy3d


#endif  // EASY3D_CORE_CURVE_H
