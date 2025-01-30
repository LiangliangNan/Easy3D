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
     * \brief Base class for curve fitting/interpolation
     * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
     *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
     *      `T` is the data type.
     * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
     * \tparam T The scalar type (e.g., `float` or `double`).
     * \class Curve easy3d/core/curve.h
     * \sa Bezier, BSpline, and CatmullRom
     */
    template <template <size_t, class> class Point, size_t N, typename T>
    class Curve {
    public:
        using Point_t = Point<N, T>;

        /// Default constructor.
        Curve() : steps_(10) {}

        /// Set the number of steps.
        void set_steps(int steps) { steps_ = steps; }

        /// Add a way point.
        void add_way_point(const Point_t &point) {
            way_points_.push_back(point);
            on_way_point_added();
        }

        /// Return the number of nodes.
        int node_count() const { return static_cast<int>(nodes_.size()); }

        /// Return the coordinates of the \p i_th node.
        const Point_t &node(int i) const { return nodes_[i]; }

        /// Return the total curve length from the start point.
        T length_from_start_point(int i) const { return distances_[i]; }

        /// Return the total length of the curve.
        T total_length() const {
            assert(!distances_.empty());
            return distances_.back();
        }

        /// Clear all cached values.
        void clear() {
            nodes_.clear();
            way_points_.clear();
            distances_.clear();
        }

    protected:
        void add_node(const Point_t &node) {
            nodes_.push_back(node);
            if (nodes_.size() == 1)
                distances_.push_back(0);
            else {
                int new_node_index = nodes_.size() - 1;
                T segment_distance = distance(nodes_[new_node_index], nodes_[new_node_index - 1]);
                distances_.push_back(segment_distance + distances_[new_node_index - 1]);
            }
        }

        virtual void on_way_point_added() = 0;

        virtual Point_t interpolate(T u, const Point_t &P0, const Point_t &P1, const Point_t &P2, const Point_t &P3) const = 0;

    protected:
        int steps_;
        std::vector<Point_t> way_points_;
        std::vector<Point_t> nodes_;
        std::vector<T> distances_;
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
     *      auto cv = new Bezier<Vec, 3, float>;
     *      cv->set_steps(num);
     *      for (const auto& p : points)
     *          cv->add_way_point(p);
     *      for (int i = 0; i < cv->node_count(); ++i)
     *          std::cout << cv->node(i) << std::endl;
     *  \endcode
     * \class Bezier easy3d/core/curve.h
     */
    template <template <size_t, class> class Point, size_t N, typename T>
    class Bezier : public Curve<Point, N, T> {
    public:
        /// Default constructor
        Bezier() = default;

    protected:
        using Point_t = Point<N, T>;
        using Curve<Point, N, T>::way_points_;
        using Curve<Point, N, T>::steps_;
        using Curve<Point, N, T>::add_node;

        void on_way_point_added() override {
            if (way_points_.size() < 4)
                return;
            int new_control_point_index = way_points_.size() - 1;
            if (new_control_point_index == 3) {
                for (int i = 0; i <= steps_; i++) {
                    T u = (T) i / (T) steps_;
                    add_node(interpolate(u, way_points_[0], way_points_[1], way_points_[2], way_points_[3]));
                }
            } else {
                if (new_control_point_index % 2 == 0)
                    return;
                int pt = new_control_point_index - 2;
                for (int i = 0; i <= steps_; i++) {
                    T u = (T) i / (T) steps_;
                    Point_t point4 = 2 * way_points_[pt] - way_points_[pt - 1];
                    add_node(interpolate(u, way_points_[pt], point4, way_points_[pt + 1], way_points_[pt + 2]));
                }
            }
        }

        Point_t interpolate(T u, const Point_t &P0, const Point_t &P1, const Point_t &P2,
                            const Point_t &P3) const override {
            Point_t point;
            point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3);
            point += u * u * (3 * P0 - 6 * P1 + 3 * P2);
            point += u * ((-3) * P0 + 3 * P1);
            point += P0;
            return point;
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
     *      auto cv = new BSpline<Vec, 3, float>;
     *      cv->set_steps(num);
     *      for (const auto& p : points)
     *          cv->add_way_point(p);
     *      for (int i = 0; i < cv->node_count(); ++i)
     *          std::cout << cv->node(i) << std::endl;
     *  \endcode
     * \class BSpline easy3d/core/curve.h
     */
    template <template <size_t, class> class Point, size_t N, typename T>
    class BSpline : public Curve<Point, N, T> {
    public:
        /// Default constructor.
        BSpline() = default;

    protected:
        using Point_t = Point<N, T>;
        using Curve<Point, N, T>::way_points_;
        using Curve<Point, N, T>::steps_;
        using Curve<Point, N, T>::add_node;

        void on_way_point_added() override {
            if (way_points_.size() < 4)
                return;
            int new_control_point_index = static_cast<int>(way_points_.size()) - 1;
            int pt = new_control_point_index - 3;
            for (int i = 0; i <= steps_; i++) {
                T u = (T) i / (T) steps_;
                add_node(interpolate(u, way_points_[pt], way_points_[pt + 1], way_points_[pt + 2],
                                     way_points_[pt + 3]));
            }
        }

        Point_t interpolate(T u, const Point_t &P0, const Point_t &P1, const Point_t &P2,
                            const Point_t &P3) const override {
            Point_t point;
            point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 6;
            point += u * u * (3 * P0 - 6 * P1 + 3 * P2) / 6;
            point += u * ((-3) * P0 + 3 * P2) / 6;
            point += (P0 + 4 * P1 + P2) / 6;

            return point;
        }

    };


    /**
     * \brief Class for CatmullRom curve interpolation.
     *
     * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
     *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
     *      `T` is the data type.
     * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
     * \tparam T The scalar type (e.g., `float` or `double`).
     *
     * Example:
     *  \code
     *      auto cv = new CatmullRom<Vec, 3, float>;
     *      cv->set_steps(num);
     *      for (const auto& p : points)
     *          cv->add_way_point(p);
     *      for (int i = 0; i < cv->node_count(); ++i)
     *          std::cout << cv->node(i) << std::endl;
     *  \endcode
     * \class CatmullRom easy3d/core/curve.h
     */
    template <template <size_t, class> class Point, size_t N, typename T>
    class CatmullRom : public Curve<Point, N, T> {
    public:
        /// Default constructor.
        CatmullRom() = default;

    protected:
        using Point_t = Point<N, T>;
        using Curve<Point, N, T>::way_points_;
        using Curve<Point, N, T>::steps_;
        using Curve<Point, N, T>::add_node;

        void on_way_point_added() override {
            if (way_points_.size() < 4)
                return;
            int new_control_point_index = way_points_.size() - 1;
            int pt = new_control_point_index - 2;
            for (int i = 0; i <= steps_; i++) {
                T u = (T) i / (T) steps_;
                add_node(interpolate(u, way_points_[pt - 1], way_points_[pt], way_points_[pt + 1],
                                     way_points_[pt + 2]));
            }
        }

        Point_t interpolate(T u, const Point_t &P0, const Point_t &P1, const Point_t &P2,
                            const Point_t &P3) const override {
            Point_t point;
            point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 2;
            point += u * u * (2 * P0 - 5 * P1 + 4 * P2 - P3) / 2;
            point += u * ((-1) * P0 + P2) / 2;
            point += P1;
            return point;
        }

    };

}   // namespace easy3d


#endif  // EASY3D_CORE_CURVE_H
