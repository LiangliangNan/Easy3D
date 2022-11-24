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
     * @brief Spline curve fitting for arbitrary dimensions.
     * \details This class uses the efficient blossom algorithm to compute a position on the curve.
     *      It allow to set the order of the spline and the end points behavior (passing or not).
     *      The class can be instantiated with any point type (1D (float), 2D, 3D etc.) as long as the
     *      necessary operator overloads are implemented.
     * @tparam Point_t: type of a point operators such as '+' '*' must be correctly overloaded. The default
     *      constructor must be defined to return the null vector (0, 0 ,0 ...)
     * @tparam FT: floating point representation of the points (float, double etc.)
     * Example usage:
     *      \code
     *          const int resolution = 1000;    // Number of line subdivisions to display the spline
     *          const int order = 3;            // Smoothness of the spline (min 2)
     *          SplineCurveFitting<vec3> fitter(order, SplineCurveFitting<vec3>::eOPEN_UNIFORM);
     *          fitter.set_ctrl_points(points);
     *          for(int i = 0; i < resolution; ++i) {
     *               const vec3 p = fitter.eval_f( float(i) / float(resolution-1) );
     *               std::cout << p << std::endl;
     *          }
     *      \endcode
     * @class SplineCurveFitting easy3d/core/spline_curve_fitting.h
     */
    template<typename Point_t>
    class SplineCurveFitting {
    public:
        typedef typename Point_t::FT FT;
        enum Node_e {
            eUNIFORM,
            eOPEN_UNIFORM ///< Connected to the first and last control points
        };

    public:
        /// \brief Constructor
        /// @param k Order of the spline (minimum is two)
        /// @param node_type Nodal vector type (uniform, open_uniform). This will define the behavior of the spline
        ///     with its control points as well as its speed according to its parameter.
        explicit SplineCurveFitting(int k = 2, Node_e node_type = eOPEN_UNIFORM);

        /// \brief Sets the position of the spline control points.
        void set_ctrl_points(const std::vector<Point_t> &point);

        /// \brief Gets the control points of the spline
        void get_ctrl_points(std::vector<Point_t> &points) const;

        /// \brief Sets the the nodal vector type
        void set_node_type(Node_e type);

        /// \brief Evaluates position of the spline
        /// @param u : curve parameter ranging from [0; 1].
        /// \note Calling this with equally distributed \p u will result in non-uniformly distributed points on the
        ///     curves (because some of input points are closely spaced but others may not). To get points at fixed
        ///     distances along the curve, use the parameter generated from get_equally_spaced_parameters().
        Point_t eval_f(FT u) const;

        /// \brief Evaluates speed of the spline
        Point_t eval_df(FT u) const;

        int get_order() const { return _k; }

        /// \brief Gets parameters such that evaluation of the curve positions using these parameters results in
        ///     equally spaced points along the curve.
        /// \details Calling eval_f() with equal intervals will result in non-uniformly distributed points on the
        ///     curves. This method first evaluates the spline curve at equally spaced values in the parameter
        ///     domain, and then does linear interpolation to compute the parameters that result in points uniformly
        ///     spaced along the curve.
        std::vector<FT> get_equally_spaced_parameters(std::size_t steps) const;

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
        Point_t eval(FT u,
                     const std::vector<Point_t> &point,
                     int k,
                     const std::vector<FT> &node,
                     int off = 0) const;

        Point_t eval_rec(FT u,
                         std::vector<Point_t> p_in,
                         int k,
                         std::vector<FT> node_in) const;

        // -------------------------------------------------------------------------
        /// @name attributes
        // -------------------------------------------------------------------------

        Node_e _node_type;            ///< Nodal vector type
        int _k;                       ///< spline order
        std::vector<Point_t> _point;  ///< Control points
        std::vector<Point_t> _vec;    ///< Control points differences
        std::vector<FT> _node;    ///< Nodal vector
    };


    template<typename Point_t>
    SplineCurveFitting<Point_t>::SplineCurveFitting(int k, Node_e node_type):
            _node_type(node_type),
            _k(k),
            _point(_k),
            _vec(_k - 1),
            _node(_k + _point.size()) {
        assert_splines();
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    void SplineCurveFitting<Point_t>::set_ctrl_points(const std::vector<Point_t> &point) {
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
    void SplineCurveFitting<Point_t>::get_ctrl_points(std::vector<Point_t> &points) const {
        points = _point;
    }

    // -----------------------------------------------------------------------------

    /// The the nodal vector type
    template<typename Point_t>
    void SplineCurveFitting<Point_t>::set_node_type(Node_e type) {
        _node_type = type;
        set_nodal_vector();
        assert_splines();
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    Point_t SplineCurveFitting<Point_t>::eval_f(FT u) const {
        u = std::max(std::min(u, (FT) 1), (FT) 0); // clamp between [0 1]
        return eval(u, _point, _k, _node);
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    Point_t SplineCurveFitting<Point_t>::eval_df(FT u) const {
        u = std::max(std::min(u, (FT) 1), (FT) 0); // clamp between [0 1]
        return eval(u, _vec, (_k - 1), _node, 1) * (FT) (_k - 1);
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    std::vector<typename Point_t::FT> SplineCurveFitting<Point_t>::get_equally_spaced_parameters(std::size_t steps) const {
        assert_splines();

        std::vector<FT> lengths(steps, 0);
        std::vector<FT> parameters(steps, 0);
        Point_t prev_point;
        for (std::size_t i = 0; i < steps; ++i) {
            const FT u = static_cast<FT>(i) / static_cast<FT>(steps - 1);
            parameters[i] = u;
            const Point_t pos = eval_f(u);
            if (i > 0)
                lengths[i] = lengths[i - 1] + distance(pos, prev_point);
            prev_point = pos;
        }
        FT total_length = lengths.back();

        // given a \c value and a sorted sequence of values, find the two indices such that
        // sorted_values[left_index] <= value and sorted_values[right_index] >= value.
        auto find_closest_less_or_equal = [](const std::vector<FT> &sorted_values, FT value,
                                             std::size_t start) -> std::size_t {
            assert(value >= sorted_values.front() && value <= sorted_values.back());
            std::size_t index = start;
            while (sorted_values[index] < value)
                ++index;
            while (sorted_values[index] > value)
                --index;
            return index;
        };

        std::vector<FT> U(steps);
        for (std::size_t i = 0; i < steps; ++i) {
            FT u = static_cast<FT>(i) / static_cast<FT>(steps - 1);
            FT length = total_length * u;
            std::size_t left_index = find_closest_less_or_equal(lengths, length, i);
            std::size_t right_index = (lengths[left_index] == length) ? left_index : left_index + 1;
            if (lengths[left_index] == lengths[right_index])
                u = parameters[left_index];
            else { // linear interpolation
                FT left_u = parameters[left_index];
                FT right_u = parameters[right_index];
                FT w = (length - lengths[left_index]) / (lengths[right_index] - lengths[left_index]);
                u = (1.0f - w) * left_u + w * right_u;
            }
            U[i] = u;
        }
        return U;
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    void SplineCurveFitting<Point_t>::assert_splines() const {
        assert(_k > 1);
        assert((int) _point.size() >= _k);
        assert(_node.size() == (_k + _point.size()));
        assert(_point.size() == (_vec.size() + 1));
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    void SplineCurveFitting<Point_t>::set_nodal_vector() {
        if (_node_type == eOPEN_UNIFORM)
            set_node_to_open_uniform();
        else if (_node_type == eUNIFORM)
            set_node_to_uniform();
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    void SplineCurveFitting<Point_t>::set_node_to_uniform() {
        const std::size_t n = _point.size() - 1;
        _node.resize(_k + n + 1);

        FT step = (FT) 1 / (FT) (n - _k + 2);
        for (std::size_t i = 0; i < _node.size(); ++i) {
            _node[i] = ((FT) i) * step - step * (FT) (_k - 1);
        }
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    void SplineCurveFitting<Point_t>::set_node_to_open_uniform() {
        _node.resize(_k + _point.size());

        int acc = 1;
        for (int i = 0; i < (int) _node.size(); ++i) {
            if (i < _k)
                _node[i] = 0.;
            else if (i >= ((int) _point.size() + 1))
                _node[i] = 1.;
            else {
                _node[i] = (FT) acc / (FT) (_point.size() + 1 - _k);
                acc++;
            }
        }
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    Point_t SplineCurveFitting<Point_t>::eval(FT u,
                                  const std::vector<Point_t> &point,
                                  int k,
                                  const std::vector<FT> &node,
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

        std::vector<FT> node_rec(k + k - 2, (FT) 0);
        for (int i = (dec + 1), j = 0; i < (dec + k + k - 1); ++i, ++j)
            node_rec[j] = node[i + off];

        return eval_rec(u, p_rec, k, node_rec);
    }

    // -----------------------------------------------------------------------------

    template<typename Point_t>
    Point_t SplineCurveFitting<Point_t>::eval_rec(FT u,
                                      std::vector<Point_t> p_in,
                                      int k,
                                      std::vector<FT> node_in) const {
        if (p_in.size() == 1)
            return p_in[0];

        // TODO: use buffers in attributes for better performances ?
        std::vector<Point_t> p_out(k - 1, Point_t());
        for (int i = 0; i < (k - 1); ++i) {
            const FT n0 = node_in[i + k - 1];
            const FT n1 = node_in[i];
            const FT f0 = (n0 - u) / (n0 - n1);
            const FT f1 = (u - n1) / (n0 - n1);

            p_out[i] = p_in[i] * f0 + p_in[i + 1] * f1;
        }

        std::vector<FT> node_out(node_in.size() - 2);
        for (int i = 1, j = 0; i < ((int) node_in.size() - 1); ++i, ++j)
            node_out[j] = node_in[i];

        return eval_rec(u, p_out, (k - 1), node_out);
    }

}   // namespace easy3d


#endif  // EASY3D_CORE_SPLINE_CURVE_FITTING_H
