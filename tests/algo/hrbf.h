/* This Source Code Form is subject to the terms of the Mozilla Public License, 
 * v. 2.0. If a copy of the MPL was not distributed with this file, 
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// @file hrbf_core.hpp
/// Original author: Gael Guennebaud - gael.guennebaud@inria.fr - http://www.labri.fr/perso/guenneba/
/// Rodolphe Vaillant - (Fixed the gradient evaluation) - http://www.irit.fr/~Rodolphe.Vaillant

#ifndef HRBF_H
#define HRBF_H

#include <3rd_party/eigen/Eigen/LU>
#include <3rd_party/eigen/Eigen/Cholesky>

#include <vector>


/**
 * @brief An example of radial basis functions definitions (function phi)
 * Here you can add more radial basis function definitions.
 *
 * Radial basis function phi(x) = x^3 first and second derivative
 */
template<typename FT>
struct Rbf_pow3 {
    // phi(x) = x^3
    static inline FT f  (const FT& x) { return x*x*x;         }
    // first derivative phi'(x) = 3x^2
    static inline FT df (const FT& x) { return FT(3) * x * x; }
    // second derivative phi''(x) = 6x
    static inline FT ddf(const FT& x) { return FT(6) * x;     }
};


/// @brief Fitting surface on a cloud of points and evaluating the implicit surface.
/// @tparam Real A base scalar type, e.g., float, double.
/// @tparam DIM. Integer of the dimension of the ambient space (for an implicit surface DIM == 3)
/// @tparam Rbf The class of a radial basis. Must implement float Rbf::f(float), float Rbf::df(float),
///         and float Rbf::ddf(float) (see Rbf_pow3 for an example).
/// @note Please see http://eigen.tuxfamily.org to use matrix and vectors types of Eigen.
template<typename Real, int DIM, typename Rbf>
class HRBF
{
public:
    typedef Real FT;
    enum { Dim = DIM };

    typedef Eigen::Matrix<FT, Dim, 1>                           Vector;
    typedef Eigen::Matrix<FT, Dim, Eigen::Dynamic>              MatrixDX;
    typedef Eigen::Matrix<FT, Eigen::Dynamic, Eigen::Dynamic>   MatrixXX;
    typedef Eigen::Matrix<FT, Eigen::Dynamic, 1>                VectorX;

    HRBF() {}

    // --------------------------------------------------------------------------

    /// Compute surface interpolation given a set of points and normals.
    /// This solve the linear system of equation to find alpha scalars and beta
    /// vectors stored in '_alphas' and '_betas' attributes.
    void fit(const std::vector<Vector>& points, const std::vector<Vector>& normals);

    // -------------------------------------------------------------------------

    /// Evaluate potential f() at position 'x'
    FT eval(const Vector& x) const;

    // -------------------------------------------------------------------------

    /// Evaluate gradient nabla f() at position 'x'
    Vector grad(const Vector& x) const;

    // --------------------------------------------------------------------------

    const VectorX& alphas() const { return _alphas; }
    const MatrixDX& betas() const { return _betas;  }

private:
    /// Each column represents p_i:  VectorX pi = _node_centers.col(i);
    MatrixDX  _node_centers;
    /// Vector of scalar values alpha
    VectorX   _alphas;
    /// Each column represents beta_i:  VectorX bi = _betas.col(i);
    MatrixDX  _betas;

}; // END HermiteRbfReconstruction Class =======================================




template<typename FT, int DIM, typename Rbf>
void HRBF<FT, DIM, Rbf>::fit(const std::vector<Vector>& points, const std::vector<Vector>& normals)
{
    assert( points.size() == normals.size() );

    int nb_points           = points.size();
    int nb_hrbf_constraints = (Dim+1)*nb_points;
    int nb_constraints      = nb_hrbf_constraints;
    int nb_coeffs           = (Dim+1)*nb_points;

    _node_centers.resize(Dim, nb_points);
    _betas.       resize(Dim, nb_points);
    _alphas.      resize(nb_points);

    // Assemble the "design" and "value" matrix and vector
    MatrixXX  D(nb_constraints, nb_coeffs);
    VectorX   f(nb_constraints);
    VectorX   x(nb_coeffs);

    // copy the node centers
    for(int i = 0; i < nb_points; ++i)
        _node_centers.col(i) = points[i];

    for(int i = 0; i < nb_points; ++i)
    {
        Vector p = points[i];
        Vector n = normals[i];

        int io = (Dim+1) * i;
        f(io) = 0;
        f.template segment<Dim>(io + 1) = n;

        for(int j = 0; j < nb_points; ++j)
        {
            int jo = (Dim + 1) * j;
            Vector diff = p - _node_centers.col(j);
            FT l = diff.norm();
            if( l == 0 ) {
                D.template block<Dim+1,Dim+1>(io,jo).setZero();
            } else {
                FT w    = Rbf::f(l);
                FT dw_l = Rbf::df(l)/l;
                FT ddw  = Rbf::ddf(l);
                Vector g    = diff * dw_l;
                D(io,jo) = w;
                D.row(io).template segment<Dim>(jo+1) = g.transpose();
                D.col(jo).template segment<Dim>(io+1) = g;
                D.template block<Dim,Dim>(io+1,jo+1)  = (ddw - dw_l)/(l*l) * (diff * diff.transpose());
                D.template block<Dim,Dim>(io+1,jo+1).diagonal().array() += dw_l;
            }
        }
    }

    x = D.lu().solve(f);
    Eigen::Map< Eigen::Matrix<FT,Dim+1,Eigen::Dynamic> > mx( x.data(), Dim + 1, nb_points);

    _alphas = mx.row(0);
    _betas  = mx.template bottomRows<Dim>();
}

// -------------------------------------------------------------------------

template<typename FT, int DIM, typename Rbf>
FT HRBF<FT, DIM, Rbf>::eval(const Vector& x) const
{
    FT ret = 0;
    int nb_nodes = _node_centers.cols();

    for(int i = 0; i < nb_nodes; ++i)
    {
        Vector diff = x - _node_centers.col(i);
        FT l    = diff.norm();

        if( l > 0 )
        {
            ret += _alphas(i) * Rbf::f(l);
            ret += _betas.col(i).dot( diff ) * Rbf::df(l) / l;
        }
    }
    return ret;
}

// -------------------------------------------------------------------------

template<typename FT, int DIM, typename Rbf>
typename HRBF<FT, DIM, Rbf>::Vector HRBF<FT, DIM, Rbf>::grad(const Vector& x) const
{
    Vector grad = Vector::Zero();
    int nb_nodes = _node_centers.cols();
    for(int i = 0; i < nb_nodes; i++)
    {
        Vector node  = _node_centers.col(i);
        Vector beta  = _betas.col(i);
        float  alpha = _alphas(i);
        Vector diff  = x - node;

        Vector diffNormalized = diff;
        float l =  diff.norm();

        if( l > 0.00001f)
        {
            diffNormalized.normalize();
            float dphi  = Rbf::df(l);
            float ddphi = Rbf::ddf(l);

            float alpha_dphi = alpha * dphi;

            float bDotd_l = beta.dot(diff)/l;
            float squared_l = diff.squaredNorm();

            grad += alpha_dphi * diffNormalized;
            grad += bDotd_l * (ddphi * diffNormalized - diff * dphi / squared_l) + beta * dphi / l ;
        }
    }
    return grad;
}



#endif // HRBF_H

