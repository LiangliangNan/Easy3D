/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) Copyright(c) 2000, 2001. Magic Software, Inc.
 *
 * The code in this file is partly from Magic Software with
 * modifications and enhancement:
 *      http://www.magic-software.com
 * The original code was under the "FREE SOURCE CODE" licence
 *      http://www.magic-software.com/License/free.pdf
 ********************************************************************/

#ifndef EASY3D_CORE_EIGEN_SOLVER_H
#define EASY3D_CORE_EIGEN_SOLVER_H

#include <cmath>
#include <cassert>


namespace easy3d
{
    /**
     * \brief An easy-to-use eigen solver.
     * \details This class provides methods to compute the eigenvalues and eigenvectors of a given matrix.
     * \tparam MAT The type of the matrix.
     * \tparam T The type of the floating-point number (default is double).
     * \class EigenSolver easy3d/core/eigen_solver.h
     */
    template <class MAT, typename T = double>
    class EigenSolver
    {
    public:
        /**
         * \brief Enumeration of sorting methods for eigenvalues and their corresponding eigenvectors.
         */
        enum SortingMethod
        {
            NO_SORTING, ///< No sorting
            INCREASING, ///< Sort in increasing order
            DECREASING  ///< Sort in decreasing order
        };

    public:
        /**
         * \brief Constructs an EigenSolver with a specified matrix size.
         * \param n The size of the input matrix.
         */
        explicit EigenSolver(int n);
        /**
         * \brief Destructor.
         */
        ~EigenSolver();

        /**
         * \brief Computes the eigenvalues and eigenvectors of the specified input matrix.
         * \param mat The input matrix.
         * \param sm The sorting method for the eigenvalues and eigenvectors (default is NO_SORTING).
         */
        void solve(const MAT& mat, SortingMethod sm = NO_SORTING);

        /**
         * \brief Retrieves the eigenvalue at the specified index.
         * \param i The index of the eigenvalue.
         * \return The eigenvalue at the specified index.
         */
        T eigen_value(int i) const { return diag_[i]; }
        /**
         * \brief Retrieves the specified component of the eigenvector at the given index.
         * \param comp The component index of the eigenvector.
         * \param i The index of the eigenvector.
         * \return The specified component of the eigenvector at the given index.
         */
        T eigen_vector(int comp, int i) const { return matrix_(comp, i); }

        /**
         * \brief Retrieves the array of eigenvalues.
         * \return A pointer to the array of eigenvalues.
         */
        T* eigen_values() { return diag_; }

        /**
         * \brief Retrieves the matrix of eigenvectors, stored as columns.
         * \return A reference to the matrix of eigenvectors.
         */
        const MAT& eigen_vectors() { return matrix_; }

    protected:
        int size_; ///< The size of the matrix.
        MAT matrix_; ///< The matrix of eigenvectors.
        T* diag_; ///< The array of eigenvalues.
        T* subd_; ///< The array of subdiagonal elements.

    private:
        // Householder reduction to tridiagonal form
        static void tridiagonal_2(MAT& mat, T* diag, T* subd);
        static void tridiagonal_3(MAT& mat, T* diag, T* subd);
        static void tridiagonal_4(MAT& mat, T* diag, T* subd);
        static void tridiagonal_n(int n, MAT& mat, T* diag, T* subd);

        // QL algorithm with implicit shifting, applies to tridiagonal matrices
        static bool ql_algorithm(int n, T* diag, T* subd, MAT& mat);

        // sort eigenvalues and eigenvectors in the descending order
        static void decreasing_sort(int n, T* eigval, MAT& eigvec);

        // sort eigenvalues and eigenvectors in the ascending order
        static void increasing_sort(int n, T* eigval, MAT& eigvec);
    };


    //----------------------------------------------------------------------------


    template <class MAT, typename T>
    EigenSolver<MAT, T>::EigenSolver(int n)
    {
        assert(n >= 2);
        size_ = n;
        diag_ = new T[size_];
        subd_ = new T[size_];
    }

    template <class MAT, typename T>
    EigenSolver<MAT, T>::~EigenSolver()
    {
        delete[] subd_;
        delete[] diag_;
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::tridiagonal_2(MAT& matrix, T* diag, T* subd)
    {
        // matrix is already tridiagonal
        diag[0] = matrix(0, 0);
        diag[1] = matrix(1, 1);
        subd[0] = matrix(0, 1);
        subd[1] = 0.0f;
        matrix(0, 0) = 1.0f;
        matrix(0, 1) = 0.0f;
        matrix(1, 0) = 0.0f;
        matrix(1, 1) = 1.0f;
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::tridiagonal_3(MAT& matrix, T* diag, T* subd)
    {
        T fM00 = matrix(0, 0);
        T fM01 = matrix(0, 1);
        T fM02 = matrix(0, 2);
        T fM11 = matrix(1, 1);
        T fM12 = matrix(1, 2);
        T fM22 = matrix(2, 2);

        diag[0] = fM00;
        subd[2] = 0.0f;
        if (fM02 != 0.0f)
        {
            T fLength = std::sqrt(fM01 * fM01 + fM02 * fM02);
            T fInvLength = 1.0f / fLength;
            fM01 *= fInvLength;
            fM02 *= fInvLength;
            T fQ = 2.0f * fM01 * fM12 + fM02 * (fM22 - fM11);
            diag[1] = fM11 + fM02 * fQ;
            diag[2] = fM22 - fM02 * fQ;
            subd[0] = fLength;
            subd[1] = fM12 - fM01 * fQ;
            matrix(0, 0) = 1.0f;
            matrix(0, 1) = 0.0f;
            matrix(0, 2) = 0.0f;
            matrix(1, 0) = 0.0f;
            matrix(1, 1) = fM01;
            matrix(1, 2) = fM02;
            matrix(2, 0) = 0.0f;
            matrix(2, 1) = fM02;
            matrix(2, 2) = -fM01;
        }
        else
        {
            diag[1] = fM11;
            diag[2] = fM22;
            subd[0] = fM01;
            subd[1] = fM12;
            matrix(0, 0) = 1.0f;
            matrix(0, 1) = 0.0f;
            matrix(0, 2) = 0.0f;
            matrix(1, 0) = 0.0f;
            matrix(1, 1) = 1.0f;
            matrix(1, 2) = 0.0f;
            matrix(2, 0) = 0.0f;
            matrix(2, 1) = 0.0f;
            matrix(2, 2) = 1.0f;
        }
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::tridiagonal_4(MAT& matrix, T* diag, T* subd)
    {
        // save matrix M
        T fM00 = matrix(0, 0);
        T fM01 = matrix(0, 1);
        T fM02 = matrix(0, 2);
        T fM03 = matrix(0, 3);
        T fM11 = matrix(1, 1);
        T fM12 = matrix(1, 2);
        T fM13 = matrix(1, 3);
        T fM22 = matrix(2, 2);
        T fM23 = matrix(2, 3);
        T fM33 = matrix(3, 3);

        diag[0] = fM00;
        subd[3] = 0.0f;

        matrix(0, 0) = 1.0f;
        matrix(0, 1) = 0.0f;
        matrix(0, 2) = 0.0f;
        matrix(0, 3) = 0.0f;
        matrix(1, 0) = 0.0f;
        matrix(2, 0) = 0.0f;
        matrix(3, 0) = 0.0f;

        T fLength, fInvLength;

        if (fM02 != 0.0f || fM03 != 0.0f)
        {
            T fQ11, fQ12, fQ13;
            T fQ21, fQ22, fQ23;
            T fQ31, fQ32, fQ33;

            // build column Q1
            fLength = std::sqrt(fM01 * fM01 + fM02 * fM02 + fM03 * fM03);
            fInvLength = 1.0f / fLength;
            fQ11 = fM01 * fInvLength;
            fQ21 = fM02 * fInvLength;
            fQ31 = fM03 * fInvLength;

            subd[0] = fLength;

            // compute S*Q1
            T fV0 = fM11 * fQ11 + fM12 * fQ21 + fM13 * fQ31;
            T fV1 = fM12 * fQ11 + fM22 * fQ21 + fM23 * fQ31;
            T fV2 = fM13 * fQ11 + fM23 * fQ21 + fM33 * fQ31;

            diag[1] = fQ11 * fV0 + fQ21 * fV1 + fQ31 * fV2;

            // build column Q3 = Q1x(S*Q1)
            fQ13 = fQ21 * fV2 - fQ31 * fV1;
            fQ23 = fQ31 * fV0 - fQ11 * fV2;
            fQ33 = fQ11 * fV1 - fQ21 * fV0;
            fLength = std::sqrt(fQ13 * fQ13 + fQ23 * fQ23 + fQ33 * fQ33);
            if (fLength > 0.0f)
            {
                fInvLength = 1.0f / fLength;
                fQ13 *= fInvLength;
                fQ23 *= fInvLength;
                fQ33 *= fInvLength;

                // build column Q2 = Q3xQ1
                fQ12 = fQ23 * fQ31 - fQ33 * fQ21;
                fQ22 = fQ33 * fQ11 - fQ13 * fQ31;
                fQ32 = fQ13 * fQ21 - fQ23 * fQ11;

                fV0 = fQ12 * fM11 + fQ22 * fM12 + fQ32 * fM13;
                fV1 = fQ12 * fM12 + fQ22 * fM22 + fQ32 * fM23;
                fV2 = fQ12 * fM13 + fQ22 * fM23 + fQ32 * fM33;
                subd[1] = fQ11 * fV0 + fQ21 * fV1 + fQ31 * fV2;
                diag[2] = fQ12 * fV0 + fQ22 * fV1 + fQ32 * fV2;
                subd[2] = fQ13 * fV0 + fQ23 * fV1 + fQ33 * fV2;

                fV0 = fQ13 * fM11 + fQ23 * fM12 + fQ33 * fM13;
                fV1 = fQ13 * fM12 + fQ23 * fM22 + fQ33 * fM23;
                fV2 = fQ13 * fM13 + fQ23 * fM23 + fQ33 * fM33;
                diag[3] = fQ13 * fV0 + fQ23 * fV1 + fQ33 * fV2;
            }
            else
            {
                // S*Q1 parallel to Q1, choose any valid Q2 and Q3
                subd[1] = 0.0f;

                fLength = fQ21 * fQ21 + fQ31 * fQ31;
                if (fLength > 0.0f)
                {
                    fInvLength = 1.0f / fLength;
                    T fTmp = fQ11 - 1.0f;
                    fQ12 = -fQ21;
                    fQ22 = 1.0f + fTmp * fQ21 * fQ21 * fInvLength;
                    fQ32 = fTmp * fQ21 * fQ31 * fInvLength;

                    fQ13 = -fQ31;
                    fQ23 = fQ32;
                    fQ33 = 1.0f + fTmp * fQ31 * fQ31 * fInvLength;

                    fV0 = fQ12 * fM11 + fQ22 * fM12 + fQ32 * fM13;
                    fV1 = fQ12 * fM12 + fQ22 * fM22 + fQ32 * fM23;
                    fV2 = fQ12 * fM13 + fQ22 * fM23 + fQ32 * fM33;
                    diag[2] = fQ12 * fV0 + fQ22 * fV1 + fQ32 * fV2;
                    subd[2] = fQ13 * fV0 + fQ23 * fV1 + fQ33 * fV2;

                    fV0 = fQ13 * fM11 + fQ23 * fM12 + fQ33 * fM13;
                    fV1 = fQ13 * fM12 + fQ23 * fM22 + fQ33 * fM23;
                    fV2 = fQ13 * fM13 + fQ23 * fM23 + fQ33 * fM33;
                    diag[3] = fQ13 * fV0 + fQ23 * fV1 + fQ33 * fV2;
                }
                else
                {
                    // Q1 =(+-1,0,0)
                    fQ12 = 0.0f;
                    fQ22 = 1.0f;
                    fQ32 = 0.0f;
                    fQ13 = 0.0f;
                    fQ23 = 0.0f;
                    fQ33 = 1.0f;

                    diag[2] = fM22;
                    diag[3] = fM33;
                    subd[2] = fM23;
                }
            }

            matrix(1, 1) = fQ11;
            matrix(1, 2) = fQ12;
            matrix(1, 3) = fQ13;
            matrix(2, 1) = fQ21;
            matrix(2, 2) = fQ22;
            matrix(2, 3) = fQ23;
            matrix(3, 1) = fQ31;
            matrix(3, 2) = fQ32;
            matrix(3, 3) = fQ33;
        }
        else
        {
            diag[1] = fM11;
            subd[0] = fM01;
            matrix(1, 1) = 1.0f;
            matrix(2, 1) = 0.0f;
            matrix(3, 1) = 0.0f;

            if (fM13 != 0.0f)
            {
                fLength = std::sqrt(fM12 * fM12 + fM13 * fM13);
                fInvLength = 1.0f / fLength;
                fM12 *= fInvLength;
                fM13 *= fInvLength;
                T fQ = 2.0f * fM12 * fM23 + fM13 * (fM33 - fM22);

                diag[2] = fM22 + fM13 * fQ;
                diag[3] = fM33 - fM13 * fQ;
                subd[1] = fLength;
                subd[2] = fM23 - fM12 * fQ;
                matrix(1, 2) = 0.0f;
                matrix(1, 3) = 0.0f;
                matrix(2, 2) = fM12;
                matrix(2, 3) = fM13;
                matrix(3, 2) = fM13;
                matrix(3, 3) = -fM12;
            }
            else
            {
                diag[2] = fM22;
                diag[3] = fM33;
                subd[1] = fM12;
                subd[2] = fM23;
                matrix(1, 2) = 0.0f;
                matrix(1, 3) = 0.0f;
                matrix(2, 2) = 1.0f;
                matrix(2, 3) = 0.0f;
                matrix(3, 2) = 0.0f;
                matrix(3, 3) = 1.0f;
            }
        }
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::tridiagonal_n(int n, MAT& matrix, T* diag, T* subd)
    {
        int i0, i1, i2, i3;

        for (i0 = n - 1, i3 = n - 2; i0 >= 1; i0--, i3--)
        {
            T fH = 0.0f, fScale = 0.0f;

            if (i3 > 0)
            {
                for (i2 = 0; i2 <= i3; i2++)
                    fScale += std::abs(matrix(i0, i2));
                if (fScale == 0)
                {
                    subd[i0] = matrix(i0, i3);
                }
                else
                {
                    T fInvScale = 1.0f / fScale;
                    for (i2 = 0; i2 <= i3; i2++)
                    {
                        matrix(i0, i2) *= fInvScale;
                        fH += matrix(i0, i2) * matrix(i0, i2);
                    }
                    T fF = matrix(i0, i3);
                    T fG = std::sqrt(fH);
                    if (fF > 0.0f)
                        fG = -fG;
                    subd[i0] = fScale * fG;
                    fH -= fF * fG;
                    matrix(i0, i3) = fF - fG;
                    fF = 0.0f;
                    T fInvH = 1.0f / fH;
                    for (i1 = 0; i1 <= i3; i1++)
                    {
                        matrix(i1, i0) = matrix(i0, i1) * fInvH;
                        fG = 0.0f;
                        for (i2 = 0; i2 <= i1; i2++)
                            fG += matrix(i1, i2) * matrix(i0, i2);
                        for (i2 = i1 + 1; i2 <= i3; i2++)
                            fG += matrix(i2, i1) * matrix(i0, i2);
                        subd[i1] = fG * fInvH;
                        fF += subd[i1] * matrix(i0, i1);
                    }
                    T fHalfFdivH = 0.5f * fF * fInvH;
                    for (i1 = 0; i1 <= i3; i1++)
                    {
                        fF = matrix(i0, i1);
                        fG = subd[i1] - fHalfFdivH * fF;
                        subd[i1] = fG;
                        for (i2 = 0; i2 <= i1; i2++)
                        {
                            matrix(i1, i2) -= fF * subd[i2] + fG * matrix(i0, i2);
                        }
                    }
                }
            }
            else
            {
                subd[i0] = matrix(i0, i3);
            }

            diag[i0] = fH;
        }

        diag[0] = 0.0f;
        subd[0] = 0.0f;
        for (i0 = 0, i3 = -1; i0 <= n - 1; i0++, i3++)
        {
            if (diag[i0])
            {
                for (i1 = 0; i1 <= i3; i1++)
                {
                    T fSum = 0.0f;
                    for (i2 = 0; i2 <= i3; i2++)
                        fSum += matrix(i0, i2) * matrix(i2, i1);
                    for (i2 = 0; i2 <= i3; i2++)
                        matrix(i2, i1) -= fSum * matrix(i2, i0);
                }
            }
            diag[i0] = matrix(i0, i0);
            matrix(i0, i0) = 1.0f;
            for (i1 = 0; i1 <= i3; i1++)
            {
                matrix(i1, i0) = 0.0f;
                matrix(i0, i1) = 0.0f;
            }
        }

        // re-ordering if Eigen<T>::ql_algorithm is used subsequently
        for (i0 = 1, i3 = 0; i0 < n; i0++, i3++)
            subd[i3] = subd[i0];
        subd[n - 1] = 0.0f;
    }


    template <class MAT, typename T>
    bool EigenSolver<MAT, T>::ql_algorithm(int n, T* diag, T* subd, MAT& matrix)
    {
        const int iMaxIter = 32;

        for (int i0 = 0; i0 < n; i0++)
        {
            int i1;
            for (i1 = 0; i1 < iMaxIter; i1++)
            {
                int i2;
                for (i2 = i0; i2 <= n - 2; i2++)
                {
                    T fTmp = std::abs(diag[i2]) +
                        std::abs(diag[i2 + 1]);
                    if (std::abs(subd[i2]) + fTmp == fTmp)
                        break;
                }
                if (i2 == i0)
                    break;

                T fG = (diag[i0 + 1] - diag[i0]) / (2.0f * subd[i0]);
                T fR = std::sqrt(fG * fG + 1.0f);
                if (fG < 0.0f)
                    fG = diag[i2] - diag[i0] + subd[i0] / (fG - fR);
                else
                    fG = diag[i2] - diag[i0] + subd[i0] / (fG + fR);
                T fSin = 1.0f, fCos = 1.0, fP = 0.0f;
                for (int i3 = i2 - 1; i3 >= i0; i3--)
                {
                    T fF = fSin * subd[i3];
                    T fB = fCos * subd[i3];
                    if (std::abs(fF) >= std::abs(fG))
                    {
                        fCos = fG / fF;
                        fR = std::sqrt(fCos * fCos + 1.0f);
                        subd[i3 + 1] = fF * fR;
                        fSin = 1.0f / fR;
                        fCos *= fSin;
                    }
                    else
                    {
                        fSin = fF / fG;
                        fR = std::sqrt(fSin * fSin + 1.0f);
                        subd[i3 + 1] = fG * fR;
                        fCos = 1.0f / fR;
                        fSin *= fCos;
                    }
                    fG = diag[i3 + 1] - fP;
                    fR = (diag[i3] - fG) * fSin + 2.0f * fB * fCos;
                    fP = fSin * fR;
                    diag[i3 + 1] = fG + fP;
                    fG = fCos * fR - fB;

                    for (int i4 = 0; i4 < n; i4++)
                    {
                        fF = matrix(i4, i3 + 1);
                        matrix(i4, i3 + 1) = fSin * matrix(i4, i3) + fCos * fF;
                        matrix(i4, i3) = fCos * matrix(i4, i3) - fSin * fF;
                    }
                }
                diag[i0] -= fP;
                subd[i0] = fG;
                subd[i2] = 0.0f;
            }
            if (i1 == iMaxIter)
                return false;
        }

        return true;
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::decreasing_sort(int n, T* eigval, MAT& eigvec)
    {
        // sort eigenvalues in decreasing order, e[0] >= ... >= e[n-1]
        for (int i0 = 0, i1; i0 <= n - 2; i0++)
        {
            // locate maximum eigenvalue
            i1 = i0;
            T fMax = eigval[i1];
            int i2;
            for (i2 = i0 + 1; i2 < n; i2++)
            {
                if (eigval[i2] > fMax)
                {
                    i1 = i2;
                    fMax = eigval[i1];
                }
            }

            if (i1 != i0)
            {
                // swap eigenvalues
                eigval[i1] = eigval[i0];
                eigval[i0] = fMax;

                // swap eigenvectors
                for (i2 = 0; i2 < n; i2++)
                {
                    T fTmp = eigvec(i2, i0);
                    eigvec(i2, i0) = eigvec(i2, i1);
                    eigvec(i2, i1) = fTmp;
                }
            }
        }
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::increasing_sort(int n, T* eigval, MAT& eigvec)
    {
        // sort eigenvalues in increasing order, e[0] <= ... <= e[n-1]
        for (int i0 = 0, i1; i0 <= n - 2; i0++)
        {
            // locate minimum eigenvalue
            i1 = i0;
            T fMin = eigval[i1];
            int i2;
            for (i2 = i0 + 1; i2 < n; i2++)
            {
                if (eigval[i2] < fMin)
                {
                    i1 = i2;
                    fMin = eigval[i1];
                }
            }

            if (i1 != i0)
            {
                // swap eigenvalues
                eigval[i1] = eigval[i0];
                eigval[i0] = fMin;

                // swap eigenvectors
                for (i2 = 0; i2 < n; i2++)
                {
                    T fTmp = eigvec(i2, i0);
                    eigvec(i2, i0) = eigvec(i2, i1);
                    eigvec(i2, i1) = fTmp;
                }
            }
        }
    }


    template <class MAT, typename T>
    void EigenSolver<MAT, T>::solve(const MAT& mat, SortingMethod sm /* = NO_SORTING*/)
    {
        matrix_ = mat;

        switch (size_)
        {
        case 2:
            tridiagonal_2(matrix_, diag_, subd_);
            break;
        case 3:
            tridiagonal_3(matrix_, diag_, subd_);
            break;
        case 4:
            tridiagonal_4(matrix_, diag_, subd_);
            break;
        default:
            tridiagonal_n(size_, matrix_, diag_, subd_);
            break;
        }

        ql_algorithm(size_, diag_, subd_, matrix_);

        switch (sm)
        {
        case INCREASING:
            increasing_sort(size_, diag_, matrix_);
            break;
        case DECREASING:
            decreasing_sort(size_, diag_, matrix_);
            break;
        default:
            break;
        }
    }
}

#endif  // EASY3D_CORE_EIGEN_SOLVER_H
