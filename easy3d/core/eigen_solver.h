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

/** ----------------------------------------------------------
 * The code is adapted from Magic Software with significant
 * modifications and enhancement.
 *      Magic Software, Inc.
 *      http://www.magic-software.com
 *      Copyright(c) 2000, 2001.  All Rights Reserved
 *      Source code from Magic Software is supplied under the
 *      terms of a license agreement and may not be copied or
 *      disclosed except in accordance with the terms of that
 *      agreement. The various license agreements may be found
 *      at the Magic Software web site. This file is subject
 *      to the license
 *              FREE SOURCE CODE
 *              http://www.magic-software.com/License/free.pdf
 *----------------------------------------------------------*/

#ifndef EASY3D_EIGEN_SOLVER_H
#define EASY3D_EIGEN_SOLVER_H

#include <cmath>
#include <cassert>


namespace easy3d {

    template <typename FT>
    class EigenSolver
    {
    public:
        enum SortingMethod { NO_SORTING, INCREASING, DECREASING };

    public:
        /// @param n: the size of the input matrix
        EigenSolver(int n);
        ~EigenSolver();

        /// solve
        /// @param mat: the input matrix (row major 2D array)
        void solve(FT** mat, SortingMethod sm = NO_SORTING);

        /// the i_th eigenvalue
        FT eigen_value(int i) const { return diag_[i]; }
        /// the comp_th component of the i_th eigenvector
        FT eigen_vector(int comp, int i) const { return matrix_[comp][i]; }

        /// the eigenvalues
        FT* eigen_values() { return diag_; }
        /// the eigenvectors (stored as the columns of the returned matrix)
        FT** eigen_vectors() { return matrix_; }

    protected:
        int     size_;
        FT**    matrix_;
        FT*     diag_;
        FT*     subd_;

    private:
        // Householder reduction to tridiagonal form
        static void tridiagonal_2(FT** mat, FT* diag, FT* subd);
        static void tridiagonal_3(FT** mat, FT* diag, FT* subd);
        static void tridiagonal_4(FT** mat, FT* diag, FT* subd);
        static void tridiagonal_n(int n, FT** mat, FT* diag, FT* subd);

        // QL algorithm with implicit shifting, applies to tridiagonal matrices
        static bool ql_algorithm(int n, FT* diag, FT* subd, FT** mat);

        // sort eigenvalues and eigenvectors in the descending order
        static void decreasing_sort(int n, FT* eigval, FT** eigvec);

        // sort eigenvalues and eigenvectors in the ascending order
        static void increasing_sort(int n, FT* eigval, FT** eigvec);
    };


    //----------------------------------------------------------------------------


    template <typename FT>
    EigenSolver<FT>::EigenSolver(int n)
    {
        assert( n >= 2 );
        size_ = n;

        matrix_ = nullptr;

        diag_ = new FT[size_];
        subd_ = new FT[size_];
    }

    template <typename FT>
    EigenSolver<FT>::~EigenSolver()
    {
        delete[] subd_;
        delete[] diag_;
    }


    template <typename FT>
    inline void EigenSolver<FT>::tridiagonal_2(FT** matrix_, FT* diag_, FT* subd_)
    {
        // matrix is already tridiagonal
        diag_[0] = matrix_[0][0];
        diag_[1] = matrix_[1][1];
        subd_[0] = matrix_[0][1];
        subd_[1] = 0.0f;
        matrix_[0][0] = 1.0f;
        matrix_[0][1] = 0.0f;
        matrix_[1][0] = 0.0f;
        matrix_[1][1] = 1.0f;
    }


    template <typename FT>
    inline void EigenSolver<FT>::tridiagonal_3(FT** matrix_, FT* diag_, FT* subd_)
    {
        FT fM00 = matrix_[0][0];
        FT fM01 = matrix_[0][1];
        FT fM02 = matrix_[0][2];
        FT fM11 = matrix_[1][1];
        FT fM12 = matrix_[1][2];
        FT fM22 = matrix_[2][2];

        diag_[0] = fM00;
        subd_[2] = 0.0f;
        if( fM02 != 0.0f )
        {
            FT fLength = std::sqrt(fM01*fM01+fM02*fM02);
            FT fInvLength = 1.0f/fLength;
            fM01 *= fInvLength;
            fM02 *= fInvLength;
            FT fQ = 2.0f*fM01*fM12+fM02*(fM22-fM11);
            diag_[1] = fM11+fM02*fQ;
            diag_[2] = fM22-fM02*fQ;
            subd_[0] = fLength;
            subd_[1] = fM12-fM01*fQ;
            matrix_[0][0] = 1.0f; matrix_[0][1] = 0.0f;  matrix_[0][2] = 0.0f;
            matrix_[1][0] = 0.0f; matrix_[1][1] = fM01; matrix_[1][2] = fM02;
            matrix_[2][0] = 0.0f; matrix_[2][1] = fM02; matrix_[2][2] = -fM01;
        }
        else
        {
            diag_[1] = fM11;
            diag_[2] = fM22;
            subd_[0] = fM01;
            subd_[1] = fM12;
            matrix_[0][0] = 1.0f; matrix_[0][1] = 0.0f; matrix_[0][2] = 0.0f;
            matrix_[1][0] = 0.0f; matrix_[1][1] = 1.0f; matrix_[1][2] = 0.0f;
            matrix_[2][0] = 0.0f; matrix_[2][1] = 0.0f; matrix_[2][2] = 1.0f;
        }
    }


    template <typename FT>
    inline void EigenSolver<FT>::tridiagonal_4(FT** matrix_, FT* diag_, FT* subd_)
    {
        // save matrix M
        FT fM00 = matrix_[0][0];
        FT fM01 = matrix_[0][1];
        FT fM02 = matrix_[0][2];
        FT fM03 = matrix_[0][3];
        FT fM11 = matrix_[1][1];
        FT fM12 = matrix_[1][2];
        FT fM13 = matrix_[1][3];
        FT fM22 = matrix_[2][2];
        FT fM23 = matrix_[2][3];
        FT fM33 = matrix_[3][3];

        diag_[0] = fM00;
        subd_[3] = 0.0f;

        matrix_[0][0] = 1.0f;
        matrix_[0][1] = 0.0f;
        matrix_[0][2] = 0.0f;
        matrix_[0][3] = 0.0f;
        matrix_[1][0] = 0.0f;
        matrix_[2][0] = 0.0f;
        matrix_[3][0] = 0.0f;

        FT fLength, fInvLength;

        if( fM02 != 0.0f || fM03 != 0.0f )
        {
            FT fQ11, fQ12, fQ13;
            FT fQ21, fQ22, fQ23;
            FT fQ31, fQ32, fQ33;

            // build column Q1
            fLength = std::sqrt(fM01*fM01+fM02*fM02+fM03*fM03);
            fInvLength = 1.0f/fLength;
            fQ11 = fM01*fInvLength;
            fQ21 = fM02*fInvLength;
            fQ31 = fM03*fInvLength;

            subd_[0] = fLength;

            // compute S*Q1
            FT fV0 = fM11*fQ11+fM12*fQ21+fM13*fQ31;
            FT fV1 = fM12*fQ11+fM22*fQ21+fM23*fQ31;
            FT fV2 = fM13*fQ11+fM23*fQ21+fM33*fQ31;

            diag_[1] = fQ11*fV0+fQ21*fV1+fQ31*fV2;

            // build column Q3 = Q1x(S*Q1)
            fQ13 = fQ21*fV2-fQ31*fV1;
            fQ23 = fQ31*fV0-fQ11*fV2;
            fQ33 = fQ11*fV1-fQ21*fV0;
            fLength = std::sqrt(fQ13*fQ13+fQ23*fQ23+fQ33*fQ33);
            if( fLength > 0.0f )
            {
                fInvLength = 1.0f/fLength;
                fQ13 *= fInvLength;
                fQ23 *= fInvLength;
                fQ33 *= fInvLength;

                // build column Q2 = Q3xQ1
                fQ12 = fQ23*fQ31-fQ33*fQ21;
                fQ22 = fQ33*fQ11-fQ13*fQ31;
                fQ32 = fQ13*fQ21-fQ23*fQ11;

                fV0 = fQ12*fM11+fQ22*fM12+fQ32*fM13;
                fV1 = fQ12*fM12+fQ22*fM22+fQ32*fM23;
                fV2 = fQ12*fM13+fQ22*fM23+fQ32*fM33;
                subd_[1] = fQ11*fV0+fQ21*fV1+fQ31*fV2;
                diag_[2] = fQ12*fV0+fQ22*fV1+fQ32*fV2;
                subd_[2] = fQ13*fV0+fQ23*fV1+fQ33*fV2;

                fV0 = fQ13*fM11+fQ23*fM12+fQ33*fM13;
                fV1 = fQ13*fM12+fQ23*fM22+fQ33*fM23;
                fV2 = fQ13*fM13+fQ23*fM23+fQ33*fM33;
                diag_[3] = fQ13*fV0+fQ23*fV1+fQ33*fV2;
            }
            else
            {
                // S*Q1 parallel to Q1, choose any valid Q2 and Q3
                subd_[1] = 0.0f;

                fLength = fQ21*fQ21+fQ31*fQ31;
                if( fLength > 0.0f )
                {
                    fInvLength = 1.0f/fLength;
                    FT fTmp = fQ11-1.0f;
                    fQ12 = -fQ21;
                    fQ22 = 1.0f+fTmp*fQ21*fQ21*fInvLength;
                    fQ32 = fTmp*fQ21*fQ31*fInvLength;

                    fQ13 = -fQ31;
                    fQ23 = fQ32;
                    fQ33 = 1.0f+fTmp*fQ31*fQ31*fInvLength;

                    fV0 = fQ12*fM11+fQ22*fM12+fQ32*fM13;
                    fV1 = fQ12*fM12+fQ22*fM22+fQ32*fM23;
                    fV2 = fQ12*fM13+fQ22*fM23+fQ32*fM33;
                    diag_[2] = fQ12*fV0+fQ22*fV1+fQ32*fV2;
                    subd_[2] = fQ13*fV0+fQ23*fV1+fQ33*fV2;

                    fV0 = fQ13*fM11+fQ23*fM12+fQ33*fM13;
                    fV1 = fQ13*fM12+fQ23*fM22+fQ33*fM23;
                    fV2 = fQ13*fM13+fQ23*fM23+fQ33*fM33;
                    diag_[3] = fQ13*fV0+fQ23*fV1+fQ33*fV2;
                }
                else
                {
                    // Q1 =(+-1,0,0)
                    fQ12 = 0.0f; fQ22 = 1.0f; fQ32 = 0.0f;
                    fQ13 = 0.0f; fQ23 = 0.0f; fQ33 = 1.0f;

                    diag_[2] = fM22;
                    diag_[3] = fM33;
                    subd_[2] = fM23;
                }
            }

            matrix_[1][1] = fQ11; matrix_[1][2] = fQ12; matrix_[1][3] = fQ13;
            matrix_[2][1] = fQ21; matrix_[2][2] = fQ22; matrix_[2][3] = fQ23;
            matrix_[3][1] = fQ31; matrix_[3][2] = fQ32; matrix_[3][3] = fQ33;
        }
        else
        {
            diag_[1] = fM11;
            subd_[0] = fM01;
            matrix_[1][1] = 1.0f;
            matrix_[2][1] = 0.0f;
            matrix_[3][1] = 0.0f;

            if( fM13 != 0.0f )
            {
                fLength = std::sqrt(fM12*fM12+fM13*fM13);
                fInvLength = 1.0f/fLength;
                fM12 *= fInvLength;
                fM13 *= fInvLength;
                FT fQ = 2.0f*fM12*fM23+fM13*(fM33-fM22);

                diag_[2] = fM22+fM13*fQ;
                diag_[3] = fM33-fM13*fQ;
                subd_[1] = fLength;
                subd_[2] = fM23-fM12*fQ;
                matrix_[1][2] = 0.0f;
                matrix_[1][3] = 0.0f;
                matrix_[2][2] = fM12;
                matrix_[2][3] = fM13;
                matrix_[3][2] = fM13;
                matrix_[3][3] = -fM12;
            }
            else
            {
                diag_[2] = fM22;
                diag_[3] = fM33;
                subd_[1] = fM12;
                subd_[2] = fM23;
                matrix_[1][2] = 0.0f;
                matrix_[1][3] = 0.0f;
                matrix_[2][2] = 1.0f;
                matrix_[2][3] = 0.0f;
                matrix_[3][2] = 0.0f;
                matrix_[3][3] = 1.0f;
            }
        }
    }


    template <typename FT>
    inline void EigenSolver<FT>::tridiagonal_n(int n, FT** matrix_, FT* diag_, FT* subd_)
    {
        int i0, i1, i2, i3;

        for(i0 = n-1, i3 = n-2; i0 >= 1; i0--, i3--)
        {
            FT fH = 0.0f, fScale = 0.0f;

            if( i3 > 0 )
            {
                for(i2 = 0; i2 <= i3; i2++)
                    fScale += std::abs(matrix_[i0][i2]);
                if( fScale == 0 )
                {
                    subd_[i0] = matrix_[i0][i3];
                }
                else
                {
                    FT fInvScale = 1.0f/fScale;
                    for(i2 = 0; i2 <= i3; i2++)
                    {
                        matrix_[i0][i2] *= fInvScale;
                        fH += matrix_[i0][i2]*matrix_[i0][i2];
                    }
                    FT fF = matrix_[i0][i3];
                    FT fG = std::sqrt(fH);
                    if( fF > 0.0f )
                        fG = -fG;
                    subd_[i0] = fScale*fG;
                    fH -= fF*fG;
                    matrix_[i0][i3] = fF-fG;
                    fF = 0.0f;
                    FT fInvH = 1.0f/fH;
                    for(i1 = 0; i1 <= i3; i1++)
                    {
                        matrix_[i1][i0] = matrix_[i0][i1]*fInvH;
                        fG = 0.0f;
                        for(i2 = 0; i2 <= i1; i2++)
                            fG += matrix_[i1][i2]*matrix_[i0][i2];
                        for(i2 = i1+1; i2 <= i3; i2++)
                            fG += matrix_[i2][i1]*matrix_[i0][i2];
                        subd_[i1] = fG*fInvH;
                        fF += subd_[i1]*matrix_[i0][i1];
                    }
                    FT fHalfFdivH = 0.5f*fF*fInvH;
                    for(i1 = 0; i1 <= i3; i1++)
                    {
                        fF = matrix_[i0][i1];
                        fG = subd_[i1] - fHalfFdivH*fF;
                        subd_[i1] = fG;
                        for(i2 = 0; i2 <= i1; i2++)
                        {
                            matrix_[i1][i2] -= fF*subd_[i2] +
                                fG*matrix_[i0][i2];
                        }
                    }
                }
            }
            else
            {
                subd_[i0] = matrix_[i0][i3];
            }

            diag_[i0] = fH;
        }

        diag_[0] = 0.0f;
        subd_[0] = 0.0f;
        for(i0 = 0, i3 = -1; i0 <= n-1; i0++, i3++)
        {
            if( diag_[i0] )
            {
                for(i1 = 0; i1 <= i3; i1++)
                {
                    FT fSum = 0.0f;
                    for(i2 = 0; i2 <= i3; i2++)
                        fSum += matrix_[i0][i2]*matrix_[i2][i1];
                    for(i2 = 0; i2 <= i3; i2++)
                        matrix_[i2][i1] -= fSum*matrix_[i2][i0];
                }
            }
            diag_[i0] = matrix_[i0][i0];
            matrix_[i0][i0] = 1.0f;
            for(i1 = 0; i1 <= i3; i1++)
            {
                matrix_[i1][i0] = 0.0f;
                matrix_[i0][i1] = 0.0f;
            }
        }

        // re-ordering if Eigen<FT>::ql_algorithm is used subsequently
        for(i0 = 1, i3 = 0; i0 < n; i0++, i3++)
            subd_[i3] = subd_[i0];
        subd_[n-1] = 0.0f;
    }


    template <typename FT>
    inline bool EigenSolver<FT>::ql_algorithm(int n, FT* diag_, FT* subd_, FT** matrix_)
    {
        const int iMaxIter = 32;

        for(int i0 = 0; i0 < n; i0++)
        {
            int i1;
            for(i1 = 0; i1 < iMaxIter; i1++)
            {
                int i2;
                for(i2 = i0; i2 <= n-2; i2++)
                {
                    FT fTmp = std::abs(diag_[i2]) +
                        std::abs(diag_[i2+1]);
                    if( std::abs(subd_[i2]) + fTmp == fTmp )
                        break;
                }
                if( i2 == i0 )
                    break;

                FT fG =(diag_[i0+1]-diag_[i0])/(2.0f*subd_[i0]);
                FT fR = std::sqrt(fG*fG+1.0f);
                if( fG < 0.0f )
                    fG = diag_[i2]-diag_[i0]+subd_[i0]/(fG-fR);
                else
                    fG = diag_[i2]-diag_[i0]+subd_[i0]/(fG+fR);
                FT fSin = 1.0f, fCos = 1.0, fP = 0.0f;
                for(int i3 = i2-1; i3 >= i0; i3--)
                {
                    FT fF = fSin*subd_[i3];
                    FT fB = fCos*subd_[i3];
                    if( std::abs(fF) >= std::abs(fG) )
                    {
                        fCos = fG/fF;
                        fR = std::sqrt(fCos*fCos+1.0f);
                        subd_[i3+1] = fF*fR;
                        fSin = 1.0f/fR;
                        fCos *= fSin;
                    }
                    else
                    {
                        fSin = fF/fG;
                        fR = std::sqrt(fSin*fSin+1.0f);
                        subd_[i3+1] = fG*fR;
                        fCos = 1.0f/fR;
                        fSin *= fCos;
                    }
                    fG = diag_[i3+1]-fP;
                    fR =(diag_[i3]-fG)*fSin+2.0f*fB*fCos;
                    fP = fSin*fR;
                    diag_[i3+1] = fG+fP;
                    fG = fCos*fR-fB;

                    for(int i4 = 0; i4 < n; i4++)
                    {
                        fF = matrix_[i4][i3+1];
                        matrix_[i4][i3+1] = fSin*matrix_[i4][i3]+fCos*fF;
                        matrix_[i4][i3] = fCos*matrix_[i4][i3]-fSin*fF;
                    }
                }
                diag_[i0] -= fP;
                subd_[i0] = fG;
                subd_[i2] = 0.0f;
            }
            if( i1 == iMaxIter )
                return false;
        }

        return true;
    }


    template <typename FT>
    inline void EigenSolver<FT>::decreasing_sort(int n, FT* eigval, FT** eigvec)
    {
        // sort eigenvalues in decreasing order, e[0] >= ... >= e[n-1]
        for(int i0 = 0, i1; i0 <= n-2; i0++)
        {
            // locate maximum eigenvalue
            i1 = i0;
            FT fMax = eigval[i1];
            int i2;
            for(i2 = i0+1; i2 < n; i2++)
            {
                if( eigval[i2] > fMax )
                {
                    i1 = i2;
                    fMax = eigval[i1];
                }
            }

            if( i1 != i0 )
            {
                // swap eigenvalues
                eigval[i1] = eigval[i0];
                eigval[i0] = fMax;

                // swap eigenvectors
                for(i2 = 0; i2 < n; i2++)
                {
                    FT fTmp = eigvec[i2][i0];
                    eigvec[i2][i0] = eigvec[i2][i1];
                    eigvec[i2][i1] = fTmp;
                }
            }
        }
    }


    template <typename FT>
    inline void EigenSolver<FT>::increasing_sort(int n, FT* eigval, FT** eigvec)
    {
        // sort eigenvalues in increasing order, e[0] <= ... <= e[n-1]
        for(int i0 = 0, i1; i0 <= n-2; i0++)
        {
            // locate minimum eigenvalue
            i1 = i0;
            FT fMin = eigval[i1];
            int i2;
            for(i2 = i0+1; i2 < n; i2++)
            {
                if( eigval[i2] < fMin )
                {
                    i1 = i2;
                    fMin = eigval[i1];
                }
            }

            if( i1 != i0 )
            {
                // swap eigenvalues
                eigval[i1] = eigval[i0];
                eigval[i0] = fMin;

                // swap eigenvectors
                for(i2 = 0; i2 < n; i2++)
                {
                    FT fTmp = eigvec[i2][i0];
                    eigvec[i2][i0] = eigvec[i2][i1];
                    eigvec[i2][i1] = fTmp;
                }
            }
        }
    }


    template <typename FT>
    inline void EigenSolver<FT>::solve(FT** mat, SortingMethod sm /* = NO_SORTING*/)
    {
        matrix_ = mat;

        switch( size_ )
        {
            case 2:
                tridiagonal_2(matrix_,diag_,subd_);
                break;
            case 3:
                tridiagonal_3(matrix_,diag_,subd_);
                break;
            case 4:
                tridiagonal_4(matrix_,diag_,subd_);
                break;
            default:
                tridiagonal_n(size_,matrix_,diag_,subd_);
                break;
        }

        ql_algorithm(size_,diag_,subd_,matrix_);

        switch( sm )
        {
            case INCREASING:
                increasing_sort(size_,diag_,matrix_);
                break;
            case DECREASING:
                decreasing_sort(size_,diag_,matrix_);
                break;
            default:
                break;
        }
    }

}

#endif  // EASY3D_EIGEN_SOLVER_H
