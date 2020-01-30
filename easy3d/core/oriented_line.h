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

#ifndef EASY3D_ORIENTED_LINE_H
#define EASY3D_ORIENTED_LINE_H

#include <easy3d/core/vec.h>


namespace easy3d {


    enum Sign {
        NEGATIVE = -1,
        ZERO = 0,
        POSITIVE = 1,
    };

    template<class T>
    inline Sign sign(T x) {
        return (x > 0) ? POSITIVE : ((x < 0) ? NEGATIVE : ZERO);
    }

    /**
     * OrientedLine implements plucker coordinates, which enables
     *  oriented lines to be compared. The comparison, implemented by the
     *  side() function, is a predicate similar to the right hand rule.
     * For instance, this class is used for the line-polygon intersection test.
     * (see D.M.Y. Sommerville, Analytical Geometry of Three Dimensions.
     *  Cambridge University Press, 1959).
     */
    template<class FT>
    class GenericOrientedLine {
    public:
        typedef Vec<3, FT> Point;

        GenericOrientedLine(const Point &p, const Point &q);

        GenericOrientedLine();

        /**
         * "right hand rule" like predicate.
         * @return +1 Positive side, -1 for negative side.
         */
        static Sign side(const GenericOrientedLine<FT> &a, const GenericOrientedLine<FT> &b);

    private:
        FT pi_[6];
    };

    //_________________________________________________________

    template<class FT>
    inline
    GenericOrientedLine<FT>::GenericOrientedLine(const Point &p, const Point &q) {
        // There are several conventions for defining plucker coordinates,
        // this one is introduced in : Marco Pellegrini, Stabbing and
        // ray-shooting in 3-dimensional space. In Proc. 6th ACM Symposium
        // on Computational Geometry, pages 177-186, 1990.
        // I think that it is possible to have a more symmetric formulation
        // of plucker coordinates, leading to a more symmetric side()
        // predicate, but I have no time to investigate this.
        pi_[0] = p.x * q.y - p.y * q.x;
        pi_[1] = p.x * q.z - p.z * q.x;
        pi_[2] = p.x - q.x;
        pi_[3] = p.y * q.z - p.z * q.y;
        pi_[4] = p.z - q.z;
        pi_[5] = q.y - p.y;
    }

    template<class FT>
    inline
    GenericOrientedLine<FT>::GenericOrientedLine() {
        pi_[0] = 0;
        pi_[1] = 0;
        pi_[2] = 0;
        pi_[3] = 0;
        pi_[4] = 0;
        pi_[5] = 0;
    }


    template<class FT>
    inline
    Sign GenericOrientedLine<FT>::side(const GenericOrientedLine<FT> &a, const GenericOrientedLine<FT> &b) {
        // Note: the order might seem strange, but product between
        //  lines in plucker coordinates is a permuted cross product.
        FT cross_product =
                a.pi_[0] * b.pi_[4] +
                a.pi_[1] * b.pi_[5] +
                a.pi_[2] * b.pi_[3] +
                a.pi_[4] * b.pi_[0] +
                a.pi_[5] * b.pi_[1] +
                a.pi_[3] * b.pi_[2];

        return sign(cross_product);
    }

}

#endif //EASY3D_ORIENTED_LINE_H
