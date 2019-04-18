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


#ifndef EASY3D_RECTANGLE_H
#define EASY3D_RECTANGLE_H

#include <algorithm>

#include <easy3d/core/vec.h>


namespace easy3d {

    // Seems duplicating 2D bounding box?

    template <typename FT>
    class GenericRect {
    public:
        GenericRect(const Vec<2, FT>& p, const Vec<2, FT>& q) {
            x_min_ = std::min(q.x, p.x);
            y_min_ = std::min(q.y, p.y);
            x_max_ = std::max(q.x, p.x);
            y_max_ = std::max(q.x, p.y);

        }

        GenericRect(FT xmin, FT xmax, FT ymin, FT ymax)
            : x_min_(xmin)
            , x_max_(xmax)
            , y_min_(ymin)
            , y_max_(ymax)
        {
        }

        FT x_min() const { return x_min_; }
        FT y_min() const { return y_min_; }
        FT x_max() const { return x_max_; }
        FT y_max() const { return y_max_; }

        FT x()  const { return x_min_; }
        FT y() const { return y_min_; }
        FT width()  const { return x_max() - x_min(); }
        FT height() const { return y_max() - y_min(); }

        FT left() const { return x_min_; }
        FT top() const  { return y_min_; }
        FT right() const  { return x_max_; }
        FT bottom() const { return y_max_; }

        Vec<2, FT> top_left() const  { return Vec<2, FT>(x_min_, y_min_); }
        Vec<2, FT> bottom_right() const  { return Vec<2, FT>(x_max_, y_max_); }
        Vec<2, FT> top_right() const  { return Vec<2, FT>(x_max_, y_min_); }
        Vec<2, FT> bottom_left() const  { return Vec<2, FT>(x_min_, y_max_); }

        Vec<2, FT> min() const { return Vec<2, FT>(x_min_, y_min_); }
        Vec<2, FT> max() const { return Vec<2, FT>(x_max_, y_max_); }
        Vec<2, FT> center() const {
            return Vec<2, FT>(FT(0.5) * (x_max() + x_min()), FT(0.5) * (y_max() + y_min()));
        }

    private:
        FT x_min_;
        FT y_min_;
        FT x_max_;
        FT y_max_;
    };

}


#endif  // EASY3D_RECTANGLE_H
