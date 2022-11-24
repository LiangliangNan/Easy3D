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

#ifndef EASY3D_CORE_SPLINE_INTERPOLATION_H
#define EASY3D_CORE_SPLINE_INTERPOLATION_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include <easy3d/util/logging.h>


namespace easy3d {

    /**
     * \brief Cubic spline interpolation.
     * \details SplineInterpolation generates a piecewise polynomial function of degree 3 and is twice continuously
     *      differentiable everywhere. Boundary conditions default to zero-curvature at the end points. It extrapolates
     *      linearly, if default boundary conditions are used, or otherwise extrapolation is a quadratic function.
     *      The math behind this implementation is described here: https://kluge.in-chemnitz.de/opensource/spline/
     *
     * Spline interpolation have many applications, e.g., curve interpolation (for any dimensions).
     * The following code shows how to use SplineInterpolation for 3D curve interpolation.
     *      \code
     *          // a 3D curve is represented in the parametric form: x(t), y(t), and z(t).
     *          std::vector<double> T(points.size()), X(points.size()), Y(points.size()), Z(points.size());
     *          double t = 0.0;
     *          for (std::size_t i = 0; i < points.size(); ++i) {
     *              const auto &p = points[i];
     *              if (i > 0)
     *                  t += distance(points[i-1], p);
     *              T[i] = t;
     *              X[i] = p.x;
     *              Y[i] = p.y;
     *              Z[i] = p.z;
     *          }
     *
     *          // class instantiation
     *          typedef SplineInterpolation<double> Interpolator;
     *          Interpolator x_spline, y_spline, z_spline;
     *          // [optional] set the boundary conditions.
     *          x_spline.set_boundary(Interpolator::second_deriv, 0.0, Interpolator::first_deriv, -2.0, false);
     *          // set data
     *          x_spline.set_data(T, X);
     *          y_spline.set_data(T, Y);
     *          z_spline.set_data(T, Z);
     *          // evaluate the curve at equal intervals (this results in a sequence of points on the curve).
     *          double total_length = t;
     *          int steps = 1000;
     *          double interval = total_length / steps;
     *          for (int i=0; i<=steps; ++i) {
     *              double d = i * interval;
     *              vec3 p(x_spline(d), y_spline(d), z_spline(d));
     *              std::cout << p << std::endl;
     *          }
     *      \endcode
     * \class SplineInterpolation easy3d/core/spline_interpolation.h
     */
    template<typename FT>
    class SplineInterpolation {
    public:
        enum BoundaryType {
            first_deriv = 1,
            second_deriv = 2
        };

    public:
        /// Constructor.
        /// Sets default boundary condition to be zero curvature at both ends
        SplineInterpolation() : left_(second_deriv), right_(second_deriv),
                        left_value_(0.0), right_value_(0.0),
                        linear_extrapolation_(false) {
        }

        /// Sets the boundary condition (optional).
        /// \attention If called, it has to come before set_points().
        void set_boundary(BoundaryType left, FT left_value,
                          BoundaryType right, FT right_value,
                          bool linear_extrapolation = false);

        /// Sets the data can carry out the interpolation.
        /// \c true for cubic spline interpolation; \c false for linear interpolation.
        /// \attention The \c x has to be monotonously increasing.
        void set_data(const std::vector <FT> &x, const std::vector <FT> &y, bool cubic_spline = true);

        /// Evaluates the spline at \p x.
        FT operator()(FT x) const;

        /// Returns the \p order -th derivative of the spline at x.
        FT derivative(int order, FT x) const;

    private:
        std::vector <FT> x_, y_;          // x,y coordinates of points
        // interpolation parameters
        // f(x) = a*(x-x_i)^3 + b*(x-x_i)^2 + c*(x-x_i) + y_i
        std::vector <FT> a_, b_, c_;      // spline coefficients
        FT b0_, c0_;                     // for left extrapolation
        BoundaryType left_, right_;
        FT left_value_, right_value_;
        bool linear_extrapolation_;
    };



    // \cond
    /**
     * \brief A band matrix representation that is capable of solving linear systems.
     * \details A band matrix is a sparse matrix whose non-zero entries are confined to a diagonal band, comprising
     *      the main diagonal and zero or more diagonals on either side.
     * \class BandMatrix easy3d/core/matrix.h
     */
    template<typename FT>
    class BandMatrix {
    public:
        /// Constructor
        BandMatrix() = default;
        /// Constructor
        BandMatrix(int dim, int n_u, int n_l);
        /// Destructor
        ~BandMatrix() = default;

        void resize(int dim, int n_u, int n_l);      // init with dim,n_u,n_l
        int dim() const;                             // matrix dimension

        int num_upper() const { return m_upper.size() - 1; }
        int num_lower() const { return m_lower.size() - 1; }

        /// access operator
        FT &operator()(int i, int j);            // write
        /// access operator
        const FT &operator()(int i, int j) const;      // read

        /// second diag (used in LU decomposition), saved in m_lower
        FT &saved_diag(int i);
        const FT &saved_diag(int i) const;

        /// LR-Decomposition of the band matrix
        void lu_decompose();

        /// solves Rx=y
        std::vector <FT> r_solve(const std::vector <FT> &b) const;
        /// solves Ly=b
        std::vector <FT> l_solve(const std::vector <FT> &b) const;
        std::vector <FT> lu_solve(const std::vector <FT> &b, bool is_lu_decomposed = false);

    private:
        std::vector <std::vector<FT>> m_upper;  // upper band
        std::vector <std::vector<FT>> m_lower;  // lower band

    };  // BandMatrix
    // \endcond


    // Cubic spline implementation
    // -----------------------

    template<typename FT>
    void SplineInterpolation<FT>::set_boundary(SplineInterpolation<FT>::BoundaryType left, FT left_value,
                                       SplineInterpolation<FT>::BoundaryType right, FT right_value,
                                       bool linear_extrapolation) {
        assert(x_.size() == 0);  // set_points() must not have happened yet
        left_ = left;
        right_ = right;
        left_value_ = left_value;
        right_value_ = right_value;
        linear_extrapolation_ = linear_extrapolation;
    }

    template<typename FT>
    void SplineInterpolation<FT>::set_data(const std::vector <FT> &x, const std::vector <FT> &y, bool cubic_spline) {
        if (x.size() != y.size()) {
            LOG(ERROR) << "sizes of x (" << x.size() << ") and y (" << y.size() << ") do not match";
            return;
        }
        if (x.size() < 3) {
            LOG(ERROR) << "too few data (size of x: " << x.size() << ")";
            return;
        }
        x_ = x;
        y_ = y;
        int n = x.size();
        // TODO: maybe sort x and y, rather than returning an error
        for (int i = 0; i < n - 1; i++) {
            if (x_[i] >= x_[i + 1]) {
                LOG_N_TIMES(3, ERROR) << "x has to be monotonously increasing (x[" << i << "]=" << x_[i] << ", x[" << i + 1 << "]=" << x_[i + 1] << "). " << COUNTER;
                return;
            }
        }

        if (cubic_spline) { // cubic spline interpolation
            // setting up the matrix and right-hand side of the equation system
            // for the parameters b[]
            BandMatrix<FT> A(n, 1, 1);
            std::vector <FT> rhs(n);
            for (int i = 1; i < n - 1; i++) {
                A(i, i - 1) = 1.0 / 3.0 * (x[i] - x[i - 1]);
                A(i, i) = 2.0 / 3.0 * (x[i + 1] - x[i - 1]);
                A(i, i + 1) = 1.0 / 3.0 * (x[i + 1] - x[i]);
                rhs[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
            }
            // boundary conditions
            if (left_ == SplineInterpolation<FT>::second_deriv) {
                // 2*b[0] = f''
                A(0, 0) = 2.0;
                A(0, 1) = 0.0;
                rhs[0] = left_value_;
            } else if (left_ == SplineInterpolation<FT>::first_deriv) {
                // c[0] = f', needs to be re-expressed in terms of b:
                // (2b[0]+b[1])(x[1]-x[0]) = 3 ((y[1]-y[0])/(x[1]-x[0]) - f')
                A(0, 0) = 2.0 * (x[1] - x[0]);
                A(0, 1) = 1.0 * (x[1] - x[0]);
                rhs[0] = 3.0 * ((y[1] - y[0]) / (x[1] - x[0]) - left_value_);
            } else {
                assert(false);
            }
            if (right_ == SplineInterpolation<FT>::second_deriv) {
                // 2*b[n-1] = f''
                A(n - 1, n - 1) = 2.0;
                A(n - 1, n - 2) = 0.0;
                rhs[n - 1] = right_value_;
            } else if (right_ == SplineInterpolation<FT>::first_deriv) {
                // c[n-1] = f', needs to be re-expressed in terms of b:
                // (b[n-2]+2b[n-1])(x[n-1]-x[n-2])
                // = 3 (f' - (y[n-1]-y[n-2])/(x[n-1]-x[n-2]))
                A(n - 1, n - 1) = 2.0 * (x[n - 1] - x[n - 2]);
                A(n - 1, n - 2) = 1.0 * (x[n - 1] - x[n - 2]);
                rhs[n - 1] = 3.0 * (right_value_ - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]));
            } else {
                assert(false);
            }

            // solve the equation system to obtain the parameters b[]
            b_ = A.lu_solve(rhs);

            // calculate parameters a[] and c[] based on b[]
            a_.resize(n);
            c_.resize(n);
            for (int i = 0; i < n - 1; i++) {
                a_[i] = 1.0 / 3.0 * (b_[i + 1] - b_[i]) / (x[i + 1] - x[i]);
                c_[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i])
                        - 1.0 / 3.0 * (2.0 * b_[i] + b_[i + 1]) * (x[i + 1] - x[i]);
            }
        } else { // linear interpolation
            a_.resize(n);
            b_.resize(n);
            c_.resize(n);
            for (int i = 0; i < n - 1; i++) {
                a_[i] = 0.0;
                b_[i] = 0.0;
                c_[i] = (y_[i + 1] - y_[i]) / (x_[i + 1] - x_[i]);
            }
        }

        // for left extrapolation coefficients
        b0_ = linear_extrapolation_ ? 0.0 : b_[0];
        c0_ = c_[0];

        // for the right extrapolation coefficients
        // f_{n-1}(x) = b*(x-x_{n-1})^2 + c*(x-x_{n-1}) + y_{n-1}
        FT h = x[n - 1] - x[n - 2];
        // b_[n-1] is determined by the boundary condition
        a_[n - 1] = 0.0;
        c_[n - 1] = 3.0 * a_[n - 2] * h * h + 2.0 * b_[n - 2] * h + c_[n - 2];   // = f'_{n-2}(x_{n-1})
        if (linear_extrapolation_)
            b_[n - 1] = 0.0;
    }

    template<typename FT>
    FT SplineInterpolation<FT>::operator()(FT x) const {
        size_t n = x_.size();
        // find the closest point x_[idx] < x, idx=0 even if x<x_[0]
        typename std::vector<FT>::const_iterator it;
        it = std::lower_bound(x_.begin(), x_.end(), x);
        int idx = std::max(int(it - x_.begin()) - 1, 0);

        FT h = x - x_[idx];
        FT interpol;
        if (x < x_[0]) {
            // extrapolation to the left
            interpol = (b0_ * h + c0_) * h + y_[0];
        } else if (x > x_[n - 1]) {
            // extrapolation to the right
            interpol = (b_[n - 1] * h + c_[n - 1]) * h + y_[n - 1];
        } else {
            // interpolation
            interpol = ((a_[idx] * h + b_[idx]) * h + c_[idx]) * h + y_[idx];
        }
        return interpol;
    }

    template<typename FT>
    FT SplineInterpolation<FT>::derivative(int order, FT x) const {
        assert(order > 0);

        size_t n = x_.size();
        // find the closest point x_[idx] < x, idx=0 even if x<x_[0]
        typename std::vector<FT>::const_iterator it;
        it = std::lower_bound(x_.begin(), x_.end(), x);
        int idx = std::max(int(it - x_.begin()) - 1, 0);

        FT h = x - x_[idx];
        FT interpol;
        if (x < x_[0]) {
            // extrapolation to the left
            switch (order) {
                case 1:
                    interpol = 2.0 * b0_ * h + c0_;
                    break;
                case 2:
                    interpol = 2.0 * b0_ * h;
                    break;
                default:
                    interpol = 0.0;
                    break;
            }
        } else if (x > x_[n - 1]) {
            // extrapolation to the right
            switch (order) {
                case 1:
                    interpol = 2.0 * b_[n - 1] * h + c_[n - 1];
                    break;
                case 2:
                    interpol = 2.0 * b_[n - 1];
                    break;
                default:
                    interpol = 0.0;
                    break;
            }
        } else {
            // interpolation
            switch (order) {
                case 1:
                    interpol = (3.0 * a_[idx] * h + 2.0 * b_[idx]) * h + c_[idx];
                    break;
                case 2:
                    interpol = 6.0 * a_[idx] * h + 2.0 * b_[idx];
                    break;
                case 3:
                    interpol = 6.0 * a_[idx];
                    break;
                default:
                    interpol = 0.0;
                    break;
            }
        }
        return interpol;
    }


    // \cond
    // BandMatrix implementation
    // -------------------------

    template<typename FT>
    BandMatrix<FT>::BandMatrix(int dim, int n_u, int n_l) {
        resize(dim, n_u, n_l);
    }

    template<typename FT>
    void BandMatrix<FT>::resize(int dim, int n_u, int n_l) {
        assert(dim > 0);
        assert(n_u >= 0);
        assert(n_l >= 0);
        m_upper.resize(n_u + 1);
        m_lower.resize(n_l + 1);
        for (size_t i = 0; i < m_upper.size(); i++) {
            m_upper[i].resize(dim);
        }
        for (size_t i = 0; i < m_lower.size(); i++) {
            m_lower[i].resize(dim);
        }
    }

    template<typename FT>
    int BandMatrix<FT>::dim() const {
        if (m_upper.size() > 0) {
            return m_upper[0].size();
        } else {
            return 0;
        }
    }


    // defines the new operator (), so that we can access the elements
    // by A(i,j), index going from i=0,...,dim()-1
    template<typename FT>
    FT &BandMatrix<FT>::operator()(int i, int j) {
        int k = j - i;       // what band is the entry
        assert((i >= 0) && (i < dim()) && (j >= 0) && (j < dim()));
        assert((-num_lower() <= k) && (k <= num_upper()));
        // k=0 -> diagonal, k<0 lower left part, k>0 upper right part
        if (k >= 0) return m_upper[k][i];
        else return m_lower[-k][i];
    }

    template<typename FT>
    const FT &BandMatrix<FT>::operator()(int i, int j) const {
        int k = j - i;       // what band is the entry
        assert((i >= 0) && (i < dim()) && (j >= 0) && (j < dim()));
        assert((-num_lower() <= k) && (k <= num_upper()));
        // k=0 -> diagonal, k<0 lower left part, k>0 upper right part
        if (k >= 0) return m_upper[k][i];
        else return m_lower[-k][i];
    }

    // second diag (used in LU decomposition), saved in m_lower
    template<typename FT>
    const FT &BandMatrix<FT>::saved_diag(int i) const {
        assert((i >= 0) && (i < dim()));
        return m_lower[0][i];
    }

    template<typename FT>
    FT &BandMatrix<FT>::saved_diag(int i) {
        assert((i >= 0) && (i < dim()));
        return m_lower[0][i];
    }

    // LR-Decomposition of a band matrix
    template<typename FT>
    void BandMatrix<FT>::lu_decompose() {
        int i_max, j_max;
        int j_min;
        FT x;

        // preconditioning
        // normalize column i so that a_ii=1
        for (int i = 0; i < this->dim(); i++) {
            assert(this->operator()(i, i) != 0.0);
            this->saved_diag(i) = 1.0 / this->operator()(i, i);
            j_min = std::max(0, i - this->num_lower());
            j_max = std::min(this->dim() - 1, i + this->num_upper());
            for (int j = j_min; j <= j_max; j++) {
                this->operator()(i, j) *= this->saved_diag(i);
            }
            this->operator()(i, i) = 1.0;          // prevents rounding errors
        }

        // Gauss LR-Decomposition
        for (int k = 0; k < this->dim(); k++) {
            i_max = std::min(this->dim() - 1, k + this->num_lower());  // num_lower not a mistake!
            for (int i = k + 1; i <= i_max; i++) {
                assert(this->operator()(k, k) != 0.0);
                x = -this->operator()(i, k) / this->operator()(k, k);
                this->operator()(i, k) = -x;                         // assembly part of L
                j_max = std::min(this->dim() - 1, k + this->num_upper());
                for (int j = k + 1; j <= j_max; j++) {
                    // assembly part of R
                    this->operator()(i, j) = this->operator()(i, j) + x * this->operator()(k, j);
                }
            }
        }
    }

    // solves Ly=b
    template<typename FT>
    std::vector <FT> BandMatrix<FT>::l_solve(const std::vector <FT> &b) const {
        assert(this->dim() == (int) b.size());
        std::vector <FT> x(this->dim());
        int j_start;
        FT sum;
        for (int i = 0; i < this->dim(); i++) {
            sum = 0;
            j_start = std::max(0, i - this->num_lower());
            for (int j = j_start; j < i; j++) sum += this->operator()(i, j) * x[j];
            x[i] = (b[i] * this->saved_diag(i)) - sum;
        }
        return x;
    }

    // solves Rx=y
    template<typename FT>
    std::vector <FT> BandMatrix<FT>::r_solve(const std::vector <FT> &b) const {
        assert(this->dim() == (int) b.size());
        std::vector <FT> x(this->dim());
        int j_stop;
        FT sum;
        for (int i = this->dim() - 1; i >= 0; i--) {
            sum = 0;
            j_stop = std::min(this->dim() - 1, i + this->num_upper());
            for (int j = i + 1; j <= j_stop; j++) sum += this->operator()(i, j) * x[j];
            x[i] = (b[i] - sum) / this->operator()(i, i);
        }
        return x;
    }

    template<typename FT>
    std::vector <FT> BandMatrix<FT>::lu_solve(const std::vector <FT> &b, bool is_lu_decomposed) {
        assert(this->dim() == (int) b.size());
        std::vector <FT> x, y;
        if (!is_lu_decomposed) {
            this->lu_decompose();
        }
        y = this->l_solve(b);
        x = this->r_solve(y);
        return x;
    }
    // \endcond

}


#endif //EASY3D_CORE_SPLINE_INTERPOLATION_H
