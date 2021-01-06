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

#ifndef EASY3D_CORE_CURVE_H
#define EASY3D_CORE_CURVE_H


#include <vector>


/**
 * \brief The \c easy3D namespace.
 * \namespace easy3d
 */
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
         *     curve->set_uniform_coloring(vec4(0, 0, 1, 1));
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
         *     curve->set_uniform_coloring(vec4(0, 1, 0, 1));
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


        // =============================================================================


        /**
         * Base class for curve fitting/interpolation
         * \sa Bezier, BSpline, and CatmullRom
         */
        template<typename Point_t>
        class Curve {
        public:
            typedef typename Point_t::FT FT;
        public:
            Curve() : steps_(10) {}

            virtual ~Curve() = default;

            void set_steps(int steps) { steps_ = steps; }

            void add_way_point(const Point_t &point) {
                way_points_.push_back(point);
                on_way_point_added();
            }

            int node_count() const { return static_cast<int>(nodes_.size()); }

            const Point_t &node(int i) const { return nodes_[i]; }

            FT length_from_starting_point(int i) const { return distances_[i]; }

            FT total_length() const {
                assert(!distances_.empty());
                return distances_.back();
            }

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
         * Class for Bezier curve fitting.
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
         */
        template<typename Point_t>
        class Bezier : public Curve<Point_t> {
        public:
            typedef typename Point_t::FT FT;
        public:
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

            Point_t interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2, const Point_t &P3) const override {
                Point_t point;
                point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3);
                point += u * u * (3 * P0 - 6 * P1 + 3 * P2);
                point += u * ((-3) * P0 + 3 * P1);
                point += P0;
                return point;
            }
        };


        /**
         * Class for BSpline curve fitting.
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
         */
        template<typename Point_t>
        class BSpline : public Curve<Point_t> {
        public:
            typedef typename Point_t::FT FT;
        public:
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

            Point_t interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2, const Point_t &P3) const override {
                Point_t point;
                point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 6;
                point += u * u * (3 * P0 - 6 * P1 + 3 * P2) / 6;
                point += u * ((-3) * P0 + 3 * P2) / 6;
                point += (P0 + 4 * P1 + P2) / 6;

                return point;
            }

        };


        /**
         * Class for CatmullRom curve interpolation.
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
         */
        template<typename Point_t>
        class CatmullRom : public Curve<Point_t> {
        public:
            typedef typename Point_t::FT FT;
        public:
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

            Point_t interpolate(FT u, const Point_t &P0, const Point_t &P1, const Point_t &P2, const Point_t &P3) const override {
                Point_t point;
                point = u * u * u * ((-1) * P0 + 3 * P1 - 3 * P2 + P3) / 2;
                point += u * u * (2 * P0 - 5 * P1 + 4 * P2 - P3) / 2;
                point += u * ((-1) * P0 + P2) / 2;
                point += P1;
                return point;
            }

        };



        // =============================================================================


        // another powerful spline fitting implementation which allow to set the order of the spline and the end points
        // behavior (passing or not)
        namespace spline {

            /**
             * @class Spline. It supports arbitrary dimensions (1D, 2D, 3D, ...).
             * The class can be instantiated with any point type (1D (float), 2D, 3D etc.) as long as the appropriate
             * operator overload are implemented.
             *
             * @brief Handling spline curves of arbitrary dimensions
             * @note This class use the efficient blossom algorithm to compute a position on
             * the curve.
             *
             * @tparam Point_t : type of a point operators such as '+' '*' must be correctly
             * overloaded. The default constructor must be defined to return the
             * null vector (0, 0 ,0 ...)
             * @tparam Real_t ; floating point representation of the points
             * (float, double etc.)
             *
             * Example usage:
             *      \code
             *          const int resolution = 1000; // Number of line subdivisions to display the spline
             *          const int order = 3; // Smoothness of the spline (min 2)
             *          Spline<vec3, float> spline_curve(order, Spline<vec3, float>::eOPEN_UNIFORM);
             *          spline_curve.set_ctrl_points(points);
             *          for(int i = 0; i < resolution; ++i) {
             *               const vec3 p = spline_curve.eval_f( float(i) / float(resolution-1) );
             *               std::cout << p << std::endl;
             *          }
             *      \endcode
             */
            template<typename Point_t>
            class Spline {
            public:
                typedef typename Point_t::FT Real_t;
                enum Node_e {
                    eUNIFORM,
                    eOPEN_UNIFORM ///< Connected to the first and last control points
                };

            public:

                /// Type of the nodal vector
                /// @param k : order of the spline (minimum is two)
                /// @param node_type : nodal vector type (uniform, open_uniform)
                /// This will define the behavior of the spline with its control points
                /// as well as its speed according to its parameter.
                Spline(int k = 2, Node_e node_type = eOPEN_UNIFORM);

                /// Set the position of the spline control points.
                void set_ctrl_points(const std::vector<Point_t> &point);

                /// Get the control points of the spline
                void get_ctrl_points(std::vector<Point_t> &points) const;

                /// The the nodal vector type
                void set_node_type(Node_e type);

                /// Evaluate position of the spline
                /// @param u : curve parameter ranging from [0; 1]
                Point_t eval_f(Real_t u) const;

                /// Evaluate speed of the spline
                Point_t eval_df(Real_t u) const;

                int get_order() const { return _k; }

            private:
                // -------------------------------------------------------------------------
                /// @name Class tools
                // -------------------------------------------------------------------------

                void assert_splines() const;

                /// set value and size of the nodal vector depending on the current number
                /// of control points
                void set_nodal_vector();

                /// Set values of the nodal vector to be uniform
                void set_node_to_uniform();

                /// Set values of the nodal vector to be open uniform
                void set_node_to_open_uniform();

                /// Evaluate the equation of a splines using the blossom algorithm
                /// @param u : the curve parameter which range from the values
                /// [node[k-1]; node[point.size()]]
                /// @param point : the control points which size must be at least equal to
                /// the order of the spline (point.size() >= k)
                /// @param k : the spline order (degree == k-1)
                /// @param node : the nodal vector which defines the speed of the spline
                /// parameter u. The nodal vector size must be equal to (k + point.size())
                /// @param off : offset to apply to the nodal vector 'node' before reading
                /// from it. this is useful to compute derivatives.
                Point_t eval(Real_t u,
                             const std::vector<Point_t> &point,
                             int k,
                             const std::vector<Real_t> &node,
                             int off = 0) const;

                Point_t eval_rec(Real_t u,
                                 std::vector<Point_t> p_in,
                                 int k,
                                 std::vector<Real_t> node_in) const;

                // -------------------------------------------------------------------------
                /// @name attributes
                // -------------------------------------------------------------------------

                Node_e _node_type;            ///< Nodal vector type
                int _k;                       ///< spline order
                std::vector<Point_t> _point;  ///< Control points
                std::vector<Point_t> _vec;    ///< Control points differences
                std::vector<Real_t> _node;    ///< Nodal vector
            };


            template<typename Point_t>
            Spline<Point_t>::Spline(int k, Node_e node_type):
                    _node_type(node_type),
                    _k(k),
                    _point(_k),
                    _vec(_k - 1),
                    _node(_k + _point.size()) {
                assert_splines();
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            void Spline<Point_t>::set_ctrl_points(const std::vector<Point_t> &point) {
                _point = point;
                _vec.resize(_point.size() - 1);
                for (int i = 0; i < (int) _vec.size(); ++i)
                    _vec[i] = _point[i + 1] - _point[i];
                set_nodal_vector();
                assert_splines();

                for (int i = 0; i < (int) _vec.size(); ++i)
                    _vec[i] /= _node[_k + i] - _node[i + 1];
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            void Spline<Point_t>::get_ctrl_points(std::vector<Point_t> &points) const {
                points = _point;
            }

            // -----------------------------------------------------------------------------

            /// The the nodal vector type
            template<typename Point_t>
            void Spline<Point_t>::set_node_type(Node_e type) {
                _node_type = type;
                set_nodal_vector();
                assert_splines();
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            Point_t Spline<Point_t>::eval_f(Real_t u) const {
                u = std::max(std::min(u, (Real_t) 1), (Real_t) 0); // clamp between [0 1]
                return eval(u, _point, _k, _node);
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            Point_t Spline<Point_t>::eval_df(Real_t u) const {
                u = std::max(std::min(u, (Real_t) 1), (Real_t) 0); // clamp between [0 1]
                return eval(u, _vec, (_k - 1), _node, 1) * (Real_t) (_k - 1);
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            void Spline<Point_t>::assert_splines() const {
                assert(_k > 1);
                assert((int) _point.size() >= _k);
                assert(_node.size() == (_k + _point.size()));
                assert(_point.size() == (_vec.size() + 1));
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            void Spline<Point_t>::set_nodal_vector() {
                if (_node_type == eOPEN_UNIFORM)
                    set_node_to_open_uniform();
                else if (_node_type == eUNIFORM)
                    set_node_to_uniform();
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            void Spline<Point_t>::set_node_to_uniform() {
                const int n = _point.size() - 1;
                _node.resize(_k + n + 1);

                Real_t step = (Real_t) 1 / (Real_t) (n - _k + 2);
                for (int i = 0; i < (int) _node.size(); ++i) {
                    _node[i] = ((Real_t) i) * step - step * (Real_t) (_k - 1);
                }
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            void Spline<Point_t>::set_node_to_open_uniform() {
                _node.resize(_k + _point.size());

                int acc = 1;
                for (int i = 0; i < (int) _node.size(); ++i) {
                    if (i < _k)
                        _node[i] = 0.;
                    else if (i >= ((int) _point.size() + 1))
                        _node[i] = 1.;
                    else {
                        _node[i] = (Real_t) acc / (Real_t) (_point.size() + 1 - _k);
                        acc++;
                    }
                }
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            Point_t Spline<Point_t>::eval(Real_t u,
                                          const std::vector<Point_t> &point,
                                          int k,
                                          const std::vector<Real_t> &node,
                                          int off) const {
                assert(k > 1);
                assert((int) point.size() >= k);
                assert_splines();
                int dec = 0;
                // TODO: better search with dychotomi ?
                // TODO: check for overflow
                while (u > node[dec + k + off])
                    dec++;

                // TODO: use buffers in attributes for better performances ?
                std::vector<Point_t> p_rec(k, Point_t());
                for (int i = dec, j = 0; i < (dec + k); ++i, ++j)
                    p_rec[j] = point[i];

                std::vector<Real_t> node_rec(k + k - 2, (Real_t) 0);
                for (int i = (dec + 1), j = 0; i < (dec + k + k - 1); ++i, ++j)
                    node_rec[j] = node[i + off];

                return eval_rec(u, p_rec, k, node_rec);
            }

            // -----------------------------------------------------------------------------

            template<typename Point_t>
            Point_t Spline<Point_t>::eval_rec(Real_t u,
                                              std::vector<Point_t> p_in,
                                              int k,
                                              std::vector<Real_t> node_in) const {
                if (p_in.size() == 1)
                    return p_in[0];

                // TODO: use buffers in attributes for better performances ?
                std::vector<Point_t> p_out(k - 1, Point_t());
                for (int i = 0; i < (k - 1); ++i) {
                    const Real_t n0 = node_in[i + k - 1];
                    const Real_t n1 = node_in[i];
                    const Real_t f0 = (n0 - u) / (n0 - n1);
                    const Real_t f1 = (u - n1) / (n0 - n1);

                    p_out[i] = p_in[i] * f0 + p_in[i + 1] * f1;
                }

                std::vector<Real_t> node_out(node_in.size() - 2);
                for (int i = 1, j = 0; i < ((int) node_in.size() - 1); ++i, ++j)
                    node_out[j] = node_in[i];

                return eval_rec(u, p_out, (k - 1), node_out);
            }


        }   // spline

    }   // namespace curve

}   // namespace easy3d


#endif  // EASY3D_CORE_CURVE_H
