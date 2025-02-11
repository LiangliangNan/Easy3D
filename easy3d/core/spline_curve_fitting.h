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

#ifndef EASY3D_CORE_SPLINE_CURVE_FITTING_H
#define EASY3D_CORE_SPLINE_CURVE_FITTING_H


#include <vector>
#include <cassert>


namespace easy3d {


    /**
     * \brief Spline curve fitting for arbitrary dimensions.
     * \details This class uses the efficient blossom algorithm to compute a position on the curve.
     *      It allows to set the order of the spline and the end points behavior (passing or not).
     *      The class can be instantiated with any point type (1D (float), 2D, 3D etc.) as long as the
     *      necessary operator overloads are implemented.
     *
     * \tparam Point A templated point class that supports basic arithmetic operations (addition and scalar
     *      multiplication). It must be parameterized as `Point<N, T>`, where `N` is the number of dimensions, and
     *      `T` is the data type.
     * \tparam N The number of dimensions (e.g., 2 for 2D, 3 for 3D).
     * \tparam T The scalar type (e.g., `float` or `double`).
     *
     * Example usage:
     *      \code
     *          const int resolution = 1000;    // Number of line subdivisions to display the spline
     *          const int order = 3;            // Smoothness of the spline (min 2)
     *          SplineCurveFitting<Vec, 3, float> fitter(order, SplineCurveFitting<Vec, 3, float>::eOPEN_UNIFORM);
     *          fitter.set_ctrl_points(points);
     *          for(int i = 0; i < resolution; ++i) {
     *               const vec3 p = fitter.eval_f( float(i) / float(resolution-1) );
     *               std::cout << p << std::endl;
     *          }
     *      \endcode
     * \class SplineCurveFitting easy3d/core/spline_curve_fitting.h
     */
    template <template <size_t, class> class Point, size_t N, typename T>
    class SplineCurveFitting {
    public:
        /**
         * \brief The nodal vector (or knot vector) type.
         * \details In spline curve fitting, the nodal vector (or knot vector) plays a crucial role in defining how the
         *      spline curve is constructed. The type of nodal vector determines how the control points influence the
         *      shape of the curve.
         *      - Uniform Knot Vector: A uniform knot vector means that the knot values are evenly spaced. This results
         *        in a B-spline where every segment between knots contributes equally to the overall curve. However,
         *        this type of knot vector does not guarantee that the curve passes through the first and last control
         *        points. Example of a uniform knot vector for a cubic B-spline (degree 3, 5 control points):
         *        t=[0,1,2,3,4,5,6].
         *      - Open Uniform Knot Vector: An open uniform knot vector is a special type of uniform knot vector where
         *        the first and last knots are repeated degree + 1 times. This ensures that the curve starts at the
         *        first control point and ends at the last control point, making it more intuitive for curve fitting
         *        applications. Example of an open uniform knot vector for a cubic B-spline (degree 3, 5 control points):
         *        t=[0,0,0,0,1,2,3,3,3,3].
         */
        enum Node_e {
            eUNIFORM,     ///< Uniform nodal vector
            eOPEN_UNIFORM ///< Open uniform nodal vector.
        };

    public:
        /**
         * \brief Constructor
         * \param k Order of the spline (minimum is two)
         * \param node_type Nodal vector type (uniform, open_uniform). This will define the behavior of the spline
         *     with its control points as well as its speed according to its parameter.
         */
        explicit SplineCurveFitting(int k = 2, Node_e node_type = eOPEN_UNIFORM);

        /**
         * \brief Sets the positions of the spline control points.
         * \param points The control points of the spline.
         */
        void set_ctrl_points(const std::vector<Point<N, T>> &points);

        /**
         * \brief Gets the control points of the spline.
         * \param points The control points of the spline.
         */
        void get_ctrl_points(std::vector<Point<N, T>> &points) const;

        /**
         * \brief Sets the nodal vector type.
         * \param type The nodal vector type.
         */
        void set_node_type(Node_e type);

        /**
         * \brief Evaluates position of the spline.
         * \param u Curve parameter ranging from [0, 1].
         * \note Calling this with equally distributed \p u will result in non-uniformly distributed points on the
         *     curves (because some input points are closely spaced but others may not). To get points at fixed
         *     distances along the curve, use the parameter generated from get_equally_spaced_parameters().
         * \return The evaluated position on the spline.
         */
        Point<N, T> eval_f(T u) const;

        /**
         * \brief Evaluates speed of the spline.
         * \param u Curve parameter ranging from [0, 1].
         * \return The evaluated speed on the spline.
         */
        Point<N, T> eval_df(T u) const;

        /**
         * \brief Gets the order of the spline.
         * \return The order of the spline.
         */
        int get_order() const { return _k; }

        /**
         * \brief Gets parameters such that evaluation of the curve positions using these parameters results in
         *     equally spaced points along the curve.
         * \param steps The number of steps.
         * \details Calling eval_f() with equal intervals will result in non-uniformly distributed points on the
         *     curves. This method first evaluates the spline curve at equally spaced values in the parameter
         *     domain, and then does linear interpolation to compute the parameters that result in points uniformly
         *     spaced along the curve.
         * \return The parameters for equally spaced points along the curve.
         */
        std::vector<T> get_equally_spaced_parameters(std::size_t steps) const;

    private:
        // -------------------------------------------------------------------------
        /// @name Class tools
        // -------------------------------------------------------------------------

        /**
         * \brief Asserts the validity of the spline parameters.
         */
        void assert_splines() const;

        /// set value and size of the nodal vector depending on the current number of control points
        void set_nodal_vector();

        /// Set values of the nodal vector to be uniform
        void set_node_to_uniform();

        /// Set values of the nodal vector to be open uniform
        void set_node_to_open_uniform();

        /**
         * \brief Evaluates the equation of a spline using the blossom algorithm.
         * \param u The curve parameter which ranges from the values [node[k-1]; node[point.size()]].
         * \param point The control points which size must be at least equal to the order of the spline (point.size() >= k).
         * \param k The spline order (degree == k-1).
         * \param node The nodal vector which defines the speed of the spline parameter u. The nodal vector size must be equal to (k + point.size()).
         * \param off Offset to apply to the nodal vector 'node' before reading from it. This is useful to compute derivatives.
         * \return The evaluated position on the spline.
         */
        Point<N, T> eval(T u,
                     const std::vector<Point<N, T>> &point,
                     int k,
                     const std::vector<T> &node,
                     int off = 0) const;
        /**
         * \brief Recursive evaluation of the spline using the blossom algorithm.
         * \param u The curve parameter.
         * \param p_in The input control points.
         * \param k The spline order.
         * \param node_in The nodal vector.
         * \return The evaluated position on the spline.
         */
        Point<N, T> eval_rec(T u,
                         std::vector<Point<N, T>> p_in,
                         int k,
                         std::vector<T> node_in) const;

        // -------------------------------------------------------------------------
        /// @name attributes
        // -------------------------------------------------------------------------

        Node_e _node_type;                  ///< Nodal vector type
        int _k;                             ///< spline order
        std::vector<Point<N, T>> _point;    ///< Control points
        std::vector<Point<N, T>> _vec;      ///< Control points differences
        std::vector<T> _node;               ///< Nodal vector
    };


    template <template <size_t, class> class Point, size_t N, typename T>
    SplineCurveFitting<Point, N, T>::SplineCurveFitting(int k, Node_e node_type):
            _node_type(node_type),
            _k(k),
            _point(_k),
            _vec(_k - 1),
            _node(_k + _point.size()) {
        assert_splines();
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::set_ctrl_points(const std::vector<Point<N, T>> &point) {
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

    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::get_ctrl_points(std::vector<Point<N, T>> &points) const {
        points = _point;
    }

    // -----------------------------------------------------------------------------

    /// The nodal vector type
    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::set_node_type(Node_e type) {
        _node_type = type;
        set_nodal_vector();
        assert_splines();
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    Point<N, T> SplineCurveFitting<Point, N, T>::eval_f(T u) const {
        u = std::max(std::min(u, (T) 1), (T) 0); // clamp between [0 1]
        return eval(u, _point, _k, _node);
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    Point<N, T> SplineCurveFitting<Point, N, T>::eval_df(T u) const {
        u = std::max(std::min(u, (T) 1), (T) 0); // clamp between [0 1]
        return eval(u, _vec, (_k - 1), _node, 1) * (T) (_k - 1);
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    std::vector<T> SplineCurveFitting<Point, N, T>::get_equally_spaced_parameters(std::size_t steps) const {
        assert_splines();

        std::vector<T> lengths(steps, 0);
        std::vector<T> parameters(steps, 0);
        Point<N, T> prev_point;
        for (std::size_t i = 0; i < steps; ++i) {
            const T u = static_cast<T>(i) / static_cast<T>(steps - 1);
            parameters[i] = u;
            const Point<N, T> pos = eval_f(u);
            if (i > 0)
                lengths[i] = lengths[i - 1] + distance(pos, prev_point);
            prev_point = pos;
        }
        T total_length = lengths.back();

        // given a \c value and a sorted sequence of values, find the two indices such that
        // sorted_values[left_index] <= value and sorted_values[right_index] >= value.
        auto find_closest_less_or_equal = [](const std::vector<T> &sorted_values, T value,
                                             std::size_t start) -> std::size_t {
            assert(value >= sorted_values.front() && value <= sorted_values.back());
            std::size_t index = start;
            while (sorted_values[index] < value)
                ++index;
            while (sorted_values[index] > value)
                --index;
            return index;
        };

        std::vector<T> U(steps);
        for (std::size_t i = 0; i < steps; ++i) {
            T u = static_cast<T>(i) / static_cast<T>(steps - 1);
            T length = total_length * u;
            std::size_t left_index = find_closest_less_or_equal(lengths, length, i);
            std::size_t right_index = (lengths[left_index] == length) ? left_index : left_index + 1;
            if (lengths[left_index] == lengths[right_index])
                u = parameters[left_index];
            else { // linear interpolation
                T left_u = parameters[left_index];
                T right_u = parameters[right_index];
                T w = (length - lengths[left_index]) / (lengths[right_index] - lengths[left_index]);
                u = (1.0f - w) * left_u + w * right_u;
            }
            U[i] = u;
        }
        return U;
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::assert_splines() const {
        assert(_k > 1);
        assert((int) _point.size() >= _k);
        assert(_node.size() == (_k + _point.size()));
        assert(_point.size() == (_vec.size() + 1));
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::set_nodal_vector() {
        if (_node_type == eOPEN_UNIFORM)
            set_node_to_open_uniform();
        else if (_node_type == eUNIFORM)
            set_node_to_uniform();
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::set_node_to_uniform() {
        const std::size_t n = _point.size() - 1;
        _node.resize(_k + n + 1);

        T step = (T) 1 / (T) (n - _k + 2);
        for (std::size_t i = 0; i < _node.size(); ++i) {
            _node[i] = ((T) i) * step - step * (T) (_k - 1);
        }
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    void SplineCurveFitting<Point, N, T>::set_node_to_open_uniform() {
        _node.resize(_k + _point.size());

        int acc = 1;
        for (int i = 0; i < (int) _node.size(); ++i) {
            if (i < _k)
                _node[i] = 0.;
            else if (i >= ((int) _point.size() + 1))
                _node[i] = 1.;
            else {
                _node[i] = (T) acc / (T) (_point.size() + 1 - _k);
                acc++;
            }
        }
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    Point<N, T> SplineCurveFitting<Point, N, T>::eval(T u,
                                  const std::vector<Point<N, T>> &point,
                                  int k,
                                  const std::vector<T> &node,
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
        std::vector<Point<N, T>> p_rec(k, Point<N, T>());
        for (int i = dec, j = 0; i < (dec + k); ++i, ++j)
            p_rec[j] = point[i];

        std::vector<T> node_rec(k + k - 2, (T) 0);
        for (int i = (dec + 1), j = 0; i < (dec + k + k - 1); ++i, ++j)
            node_rec[j] = node[i + off];

        return eval_rec(u, p_rec, k, node_rec);
    }

    // -----------------------------------------------------------------------------

    template <template <size_t, class> class Point, size_t N, typename T>
    Point<N, T> SplineCurveFitting<Point, N, T>::eval_rec(T u,
                                      std::vector<Point<N, T>> p_in,
                                      int k,
                                      std::vector<T> node_in) const {
        if (p_in.size() == 1)
            return p_in[0];

        // TODO: use buffers in attributes for better performances ?
        std::vector<Point<N, T>> p_out(k - 1, Point<N, T>());
        for (int i = 0; i < (k - 1); ++i) {
            const T n0 = node_in[i + k - 1];
            const T n1 = node_in[i];
            const T f0 = (n0 - u) / (n0 - n1);
            const T f1 = (u - n1) / (n0 - n1);

            p_out[i] = p_in[i] * f0 + p_in[i + 1] * f1;
        }

        std::vector<T> node_out(node_in.size() - 2);
        for (int i = 1, j = 0; i < ((int) node_in.size() - 1); ++i, ++j)
            node_out[j] = node_in[i];

        return eval_rec(u, p_out, (k - 1), node_out);
    }

}   // namespace easy3d


#endif  // EASY3D_CORE_SPLINE_CURVE_FITTING_H
