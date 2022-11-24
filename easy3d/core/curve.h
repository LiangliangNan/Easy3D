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
         * \brief De Casteljau algorithm evaluating a quadratic or conic (second degree) curve from the given control
         * points \p A, \p B, and \p C. Works for both 2D and 3D.
         * @param curve Returns the sequence of points on the curve.
         * @param bezier_steps Controls the smoothness of the curved corners. A greater value results in a smoother
         *      transitions but more vertices. Suggested value is 4.
         * @param include_end Ture to extend the curve to the end point.
         *
         * The following code shows how to visualize a quadratic curve (as a polyline):
         * \code
         * {
         *     vec3 a(0, 0, 0);
         *     vec3 b(800, 0, 0);
         *     vec3 c(800, 800, 0);
         *     unsigned int steps = 20;
         *
         *     std::vector<vec3> points;
         *     curve::quadratic(a, b, c, steps, points);
         *     std::cout << "first point: " << points.front() << ", last point: " << points.back() << std::endl;
         *
         *     std::vector<unsigned int> indices;
         *     for (unsigned int i=0; i<points.size() - 1; ++i) {
         *         indices.push_back(i);
         *         indices.push_back(i+1);
         *     }
         *     LinesDrawable *curve = new LinesDrawable;
         *     curve->update_vertex_buffer(points);
         *     curve->update_element_buffer(indices);
         *     curve->set_impostor_type(easy3d::LinesDrawable::CYLINDER);
         *     curve->set_line_width(5);
         *     curve->set_uniform_coloring(vec3(0, 0, 1, 1));
         *     viewer.add_drawable(curve);
         * }
         * \endcode
         */
        template<typename Point_t>
        inline
        void quadratic(const Point_t &A, const Point_t &B, const Point_t &C, std::vector<Point_t> &curve,
                       unsigned int bezier_steps = 4, bool include_end = false) {
            typedef typename Point_t::FT FT;
            for (unsigned int i = 0; i < bezier_steps; i++) {
                const FT t = static_cast<FT>(i) / bezier_steps;
                const Point_t U = (1.0 - t) * A + t * B;
                const Point_t V = (1.0 - t) * B + t * C;
                curve.push_back((1.0 - t) * U + t * V);
            }

            if (include_end)
                curve.push_back(C);
        }

        /**
         * \brief De Casteljau algorithm evaluating a cubic (third degree) curve from the given control points \p A,
         * \p B, and \p C. Works for both 2D and 3D.
         * @param curve Returns the sequence of points on the curve.
         * @param bezier_steps Controls the smoothness of the curved corners. A greater value results in a smoother
         *      transitions but more vertices. Suggested value is 4.
         * @param include_end Ture to extend the curve to the end point.
         *
         * The following code shows how to visualize a quadratic curve (as a polyline):
         * \code
         * {
         *     vec3 a(0, 0, 0);
         *     vec3 b(400, 0, 0);
         *     vec3 c(400, 800, 0);
         *     vec3 d(800, 800, 0);
         *     unsigned int steps = 20;
         *
         *     std::vector<vec3> points;
         *     curve::cubic(a, b, c, d, steps, points);
         *     std::cout << "first point: " << points.front() << ", last point: " << points.back() << std::endl;
         *
         *     std::vector<unsigned int> indices;
         *     for (unsigned int i = 0; i < points.size() - 1; ++i) {
         *         indices.push_back(i);
         *         indices.push_back(i + 1);
         *     }
         *     LinesDrawable *curve = new LinesDrawable;
         *     curve->update_vertex_buffer(points);
         *     curve->update_element_buffer(indices);
         *     curve->set_impostor_type(easy3d::LinesDrawable::CYLINDER);
         *     curve->set_line_width(5);
         *     curve->set_uniform_coloring(vec3(0, 1, 0, 1));
         *     viewer.add_drawable(curve);
         * }
         * \endcode
         */
        template<typename Point_t>
        inline
        void cubic(const Point_t &A, const Point_t &B, const Point_t &C, const Point_t &D, std::vector<Point_t> &curve,
                   unsigned int bezier_steps = 4, bool include_end = false) {
            typedef typename Point_t::FT FT;
            for (unsigned int i = 0; i < bezier_steps; i++) {
                const FT t = static_cast<FT>(i) / bezier_steps;
                const Point_t U = (1.0 - t) * A + t * B;
                const Point_t V = (1.0 - t) * B + t * C;
                const Point_t W = (1.0 - t) * C + t * D;
                const Point_t M = (1.0 - t) * U + t * V;
                const Point_t N = (1.0 - t) * V + t * W;
                curve.push_back((1.0 - t) * M + t * N);
            }
            if (include_end)
                curve.push_back(D);
        }

    }


    // =============================================================================


    /**
     * \brief Base class for curve fitting/interpolation
     * \class Curve easy3d/core/curve.h
     * \sa Bezier, BSpline, and CatmullRom
     */
    template<typename Point_t>
    class Curve {
    public:
        /// The floating-point number type.
        typedef typename Point_t::FT FT;
    public:
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
        FT length_from_start_point(int i) const { return distances_[i]; }

        /// Return the total length of the curve.
        FT total_length() const {
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
                FT segment_distance = distance(nodes_[new_node_index], nodes_[new_node_index - 1]);
                distances_.push_back(segment_distance + distances_[new_node_index - 1]);
            }
        }

        virtual void on_way_point_added() = 0;

        virtual Point_t
        interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2, const Point_t &P3) const = 0;

    protected:
        int steps_;
        std::vector<Point_t> way_points_;
        std::vector<Point_t> nodes_;
        std::vector<FT> distances_;
    };

    /**
     * \brief Class for Bezier curve fitting.
     * Works for both 2D and 3D.
     * Example:
     *  \code
     *      auto cv = new easy3d::curve::Bezier<vec3>;
     *      curve->set_steps(num);
     *      for (const auto& p : points)
     *          cv->add_way_point(p);
     *      for (int i = 0; i < cv->node_count(); ++i)
     *          std::cout << cv->node(i) << std::endl;
     *  \endcode
     * \class Bezier easy3d/core/curve.h
     */
    template<typename Point_t>
    class Bezier : public Curve<Point_t> {
    public:
        /// The floating-point number type.
        typedef typename Point_t::FT FT;
    public:
        /// Default constructor
        Bezier() = default;

    protected:
        using Curve<Point_t>::way_points_;
        using Curve<Point_t>::steps_;
        using Curve<Point_t>::add_node;

        void on_way_point_added() override {
            if (way_points_.size() < 4)
                return;
            int new_control_point_index = way_points_.size() - 1;
            if (new_control_point_index == 3) {
                for (int i = 0; i <= steps_; i++) {
                    FT u = (FT) i / (FT) steps_;
                    add_node(interpolate(u, way_points_[0], way_points_[1], way_points_[2], way_points_[3]));
                }
            } else {
                if (new_control_point_index % 2 == 0)
                    return;
                int pt = new_control_point_index - 2;
                for (int i = 0; i <= steps_; i++) {
                    FT u = (FT) i / (FT) steps_;
                    Point_t point4 = 2 * way_points_[pt] - way_points_[pt - 1];
                    add_node(interpolate(u, way_points_[pt], point4, way_points_[pt + 1], way_points_[pt + 2]));
                }
            }
        }

        Point_t interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2,
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
     * Works for both 2D and 3D.
     * Example:
     *  \code
     *      auto cv = new easy3d::curve::BSpline<vec3>;
     *      curve->set_steps(num);
     *      for (const auto& p : points)
     *      cv->add_way_point(p);
     *      for (int i = 0; i < cv->node_count(); ++i)
     *      std::cout << cv->node(i) << std::endl;
     *  \endcode
     * \class BSpline easy3d/core/curve.h
     */
    template<typename Point_t>
    class BSpline : public Curve<Point_t> {
    public:
        /// The floating-point number type.
        typedef typename Point_t::FT FT;
    public:
        /// Default constructor.
        BSpline() = default;

    protected:
        using Curve<Point_t>::way_points_;
        using Curve<Point_t>::steps_;
        using Curve<Point_t>::add_node;

        void on_way_point_added() override {
            if (way_points_.size() < 4)
                return;
            int new_control_point_index = static_cast<int>(way_points_.size()) - 1;
            int pt = new_control_point_index - 3;
            for (int i = 0; i <= steps_; i++) {
                FT u = (FT) i / (FT) steps_;
                add_node(interpolate(u, way_points_[pt], way_points_[pt + 1], way_points_[pt + 2],
                                     way_points_[pt + 3]));
            }
        }

        Point_t interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2,
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
     * Works for both 2D and 3D.
     * Example:
     *  \code
     *      auto cv = new easy3d::curve::CatmullRom<vec3>;
     *      curve->set_steps(num);
     *      for (const auto& p : points)
     *      cv->add_way_point(p);
     *      for (int i = 0; i < cv->node_count(); ++i)
     *      std::cout << cv->node(i) << std::endl;
     *  \endcode
     * \class CatmullRom easy3d/core/curve.h
     */
    template<typename Point_t>
    class CatmullRom : public Curve<Point_t> {
    public:
        /// The floating-point number type.
        typedef typename Point_t::FT FT;
    public:
        /// Default constructor.
        CatmullRom() = default;

    protected:
        using Curve<Point_t>::way_points_;
        using Curve<Point_t>::steps_;
        using Curve<Point_t>::add_node;

        void on_way_point_added() override {
            if (way_points_.size() < 4)
                return;
            int new_control_point_index = way_points_.size() - 1;
            int pt = new_control_point_index - 2;
            for (int i = 0; i <= steps_; i++) {
                FT u = (FT) i / (FT) steps_;
                add_node(interpolate(u, way_points_[pt - 1], way_points_[pt], way_points_[pt + 1],
                                     way_points_[pt + 2]));
            }
        }

        Point_t interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2,
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
