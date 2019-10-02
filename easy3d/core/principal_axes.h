/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_PRINCIPAL_AXIS_H
#define EASY3D_PRINCIPAL_AXIS_H

#include <easy3d/core/types.h>


namespace easy3d {

    /**
    * PrincipalAxes computes the center and inertia axes of
    * a set of 2D or 3D points.
    * @tparam DIM dimension (must be 2 or 3)
    */

    //NOTE: 2D cases has not been tested!!!

    template <int DIM, typename FT>
    class PrincipalAxes {
    public:
        PrincipalAxes();
        ~PrincipalAxes();

        // add point one by one
        void begin();
        void add_point(const Vec<DIM, FT>& p, FT weight = FT(1.0));
        void end();

        // add a set of point (it internally calls add_point())
        template <typename InputIterator>
        void add_points(InputIterator first, InputIterator last);

        Vec<DIM, FT> center() const;

        // eigen values are sorted in descending order,
        // eigen vectors are sorted in accordance.
        Vec<DIM, FT> axis(int i) const;
        FT eigen_value(int i) const ;

    private:
        FT	center_[DIM];
        FT	axis_[DIM][DIM];
        FT	eigen_value_[DIM];

        FT**    M_;
        int		nb_points_;
        FT		sum_weights_;
    } ;

} // namespace easy3d


#include <cassert>
#include <easy3d/core/eigen_solver.h>


namespace easy3d {


    template <int DIM, typename FT>
    PrincipalAxes<DIM, FT>::PrincipalAxes() {
        M_ = new FT*[DIM];
        for (unsigned short i = 0; i < DIM; ++i)
            M_[i] = new FT[DIM];
    }


    template <int DIM, typename FT>
    PrincipalAxes<DIM, FT>::~PrincipalAxes() {
        for (unsigned short i = 0; i < DIM; ++i)
            delete[] M_[i];
        delete[] M_;
    }


    template <int DIM, typename FT>
    inline Vec<DIM, FT> PrincipalAxes<DIM, FT>::center() const {
        return Vec<DIM, FT>(center_) ;
    }


    template <int DIM, typename FT>
    inline Vec<DIM, FT> PrincipalAxes<DIM, FT>::axis(int i) const {
        assert(i >= 0 && i < DIM) ;
        return Vec<DIM, FT>(axis_[i]) ;
    }


    template <int DIM, typename FT>
    inline FT PrincipalAxes<DIM, FT>::eigen_value(int i) const {
        assert(i >= 0 && i < DIM) ;
        return eigen_value_[i] ;
    }


    template <int DIM, typename FT>
    inline void PrincipalAxes<DIM, FT>::begin() {
        nb_points_ = 0 ;
        sum_weights_ = 0 ;
        for (unsigned short i = 0; i < DIM; ++i) {
            center_[i] = 0;
            for (unsigned short j = 0; j < DIM; ++j)
                M_[i][j] = 0;
        }
    }


    template <int DIM, typename FT>
    inline void PrincipalAxes<DIM, FT>::end() {
        for (unsigned short i = 0; i < DIM; ++i)
            center_[i] /= sum_weights_;

        // If the system is under-determined, return the trivial basis.
        if(nb_points_ < DIM + 1) {
            for (unsigned short i = 0; i < DIM; ++i) {
                eigen_value_[i] = FT(1.0) ;
                for (unsigned short j = 0; j < DIM; ++j)
                    axis_[i][j] = (i == j ? FT(1) : FT(0));
            }
        }
        else {
            for (unsigned short i = 0; i < DIM; ++i) {
                for (unsigned short j = i; j < DIM; ++j) {
                    M_[i][j] = M_[i][j]/sum_weights_ - center_[i]*center_[j];
                    if (i != j)
                        M_[j][i] = M_[i][j];
                }

                if( M_[i][i] <= FT(0) )
                    M_[i][i] = std::numeric_limits<FT>::min();
            }

            EigenSolver<FT> solver(DIM);
            solver.solve(M_, EigenSolver<FT>::DECREASING);

            for (unsigned short i=0; i<DIM; ++i) {
                eigen_value_[i] = solver.eigen_value(i);
                for (unsigned short j=0; j<DIM; ++j)
                    axis_[i][j] = solver.eigen_vector(j, i); // eigenvectors are stored in columns
            }

            // Normalize the eigen vectors
            for(unsigned short i=0; i<DIM; i++) {
                FT sqr_len(0);
                for(unsigned short j=0; j<DIM; j++)
                    sqr_len += (axis_[i][j] * axis_[i][j]);
                FT s = std::sqrt(sqr_len);
                s = (s > std::numeric_limits<FT>::min()) ? FT(1.0) / s : FT(0.0);
                for (unsigned short j = 0; j < DIM; ++j)
                    axis_[i][j] *= s;
            }
        }
    }


    // The covariance matrix:
    // If A and B have components a_i and b_j respectively, then
    // the covariance matrix C has a_i*b_j as its ij-th entry.
    template <int DIM, typename FT>
    inline void PrincipalAxes<DIM, FT>::add_point(const Vec<DIM, FT>& p, FT weight) {
        for (unsigned short i = 0; i < DIM; ++i) {
            center_[i] += p[i] * weight ;
            for (unsigned short j = i; j < DIM; ++j)
                M_[i][j] += weight * p[i] * p[j];
        }
        nb_points_++ ;
        sum_weights_ += weight ;
    }


    // add a set of point (it internally calls add_point())
    template <int DIM, typename FT>
    template <typename InputIterator >
    inline void PrincipalAxes<DIM, FT>::add_points(InputIterator first, InputIterator last) {
        assert(first != last);
        begin();
        for (InputIterator it = first; it != last; ++it) {
            add_point(*it);
        }
        end();
    }

} // namespace easy3d



#endif

