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

#ifndef EASY3D_CORE_RECTANGLE_H
#define EASY3D_CORE_RECTANGLE_H

#include <algorithm>

#include <easy3d/core/vec.h>


namespace easy3d {

    /**
     * \brief The GenericRect class defines a rectangle in the 2D space, where the origin of the coordinate system
     *     is located in the top-left corner of the screen.
     * \details A rectangle is normally expressed as a top-left corner and a size (width and height).
     * \tparam FT The scalar type for rectangle coordinates.
     * \class GenericRect easy3d/core/rect.h
     */
    template <typename FT>
    class GenericRect {
    public:
        /**
         * \brief Constructs a rectangle from two points \p p and \p q.
         * \param p The first point.
         * \param q The second point.
         */
        GenericRect(const Vec<2, FT>& p, const Vec<2, FT>& q) {
            x_min_ = std::min(q.x, p.x);
            x_max_ = std::max(q.x, p.x);
            y_min_ = std::min(q.y, p.y);
            y_max_ = std::max(q.y, p.y);
        }

        /**
         * \brief Constructs a rectangle from its min coordinates (\p xmin and \p ymin)
         *      and max coordinates (\p xmax and \p ymax).
         * \param xmin The minimum x-coordinate.
         * \param xmax The maximum x-coordinate.
         * \param ymin The minimum y-coordinate.
         * \param ymax The maximum y-coordinate.
         */
        GenericRect(FT xmin, FT xmax, FT ymin, FT ymax)
            : x_min_(std::min(xmin, xmax))
            , y_min_(std::min(ymin, ymax))
            , x_max_(std::max(xmin, xmax))
            , y_max_(std::max(ymin, ymax))
        {
        }

        /**
         * \brief Returns the minimum x-coordinate.
         * \return The minimum x-coordinate.
         */
        FT& x_min() { return x_min_; }
        /**
         * \brief Returns the minimum y-coordinate.
         * \return The minimum y-coordinate.
         */
        FT& y_min() { return y_min_; }
        /**
         * \brief Returns the maximum x-coordinate.
         * \return The maximum x-coordinate.
         */
        FT& x_max() { return x_max_; }
        /**
         * \brief Returns the maximum y-coordinate.
         * \return The maximum y-coordinate.
         */
        FT& y_max() { return y_max_; }
        /**
         * \brief Returns the minimum x-coordinate (const version).
         * \return The minimum x-coordinate.
         */
        FT x_min() const { return x_min_; }
        /**
         * \brief Returns the minimum y-coordinate (const version).
         * \return The minimum y-coordinate.
         */
        FT y_min() const { return y_min_; }
        /**
         * \brief Returns the maximum x-coordinate (const version).
         * \return The maximum x-coordinate.
         */
        FT x_max() const { return x_max_; }
        /**
         * \brief Returns the maximum y-coordinate (const version).
         * \return The maximum y-coordinate.
         */
        FT y_max() const { return y_max_; }

        /**
         * \brief Returns the minimum x-coordinate (alias for x_min).
         * \return The minimum x-coordinate.
         */
        FT& x() { return x_min_; }
        /**
         * \brief Returns the minimum y-coordinate (alias for y_min).
         * \return The minimum y-coordinate.
         */
        FT& y() { return y_min_; }
        /**
         * \brief Returns the minimum x-coordinate (alias for x_min) (const version).
         * \return The minimum x-coordinate.
         */
        FT x() const { return x_min_; }
        /**
         * \brief Returns the minimum y-coordinate (alias for y_min) (const version).
         * \return The minimum y-coordinate.
         */
        FT y() const { return y_min_; }

        /**
         * \brief Returns the width of the rectangle.
         * \return The width of the rectangle.
         */
        FT width()  const { return x_max() - x_min(); }
        /**
         * \brief Returns the height of the rectangle.
         * \return The height of the rectangle.
         */
        FT height() const { return y_max() - y_min(); }

        /**
         * \brief Returns the left coordinate (alias for x_min).
         * \return The left coordinate.
         */
        FT& left() { return x_min_; }
        /**
         * \brief Returns the top coordinate (alias for y_min).
         * \return The top coordinate.
         */
        FT& top()  { return y_min_; }
        /**
         * \brief Returns the right coordinate (alias for x_max).
         * \return The right coordinate.
         */
        FT& right() { return x_max_; }
        /**
         * \brief Returns the bottom coordinate (alias for y_max).
         * \return The bottom coordinate.
         */
        FT& bottom() { return y_max_; }
        /**
         * \brief Returns the left coordinate (alias for x_min) (const version).
         * \return The left coordinate.
         */
        FT left() const { return x_min_; }
        /**
         * \brief Returns the top coordinate (alias for y_min) (const version).
         * \return The top coordinate.
         */
        FT top() const  { return y_min_; }
        /**
         * \brief Returns the right coordinate (alias for x_max) (const version).
         * \return The right coordinate.
         */
        FT right() const  { return x_max_; }
        /**
         * \brief Returns the bottom coordinate (alias for y_max) (const version).
         * \return The bottom coordinate.
         */
        FT bottom() const { return y_max_; }

        /**
         * \brief Returns the position of the rectangle's top-left corner.
         * \return The position of the top-left corner.
         */
        Vec<2, FT> top_left() const  { return Vec<2, FT>(x_min_, y_min_); }
        /**
         * \brief Returns the position of the rectangle's bottom-right corner.
         * \return The position of the bottom-right corner.
         */
        Vec<2, FT> bottom_right() const  { return Vec<2, FT>(x_max_, y_max_); }
        /**
         * \brief Returns the position of the rectangle's top-right corner.
         * \return The position of the top-right corner.
         */
        Vec<2, FT> top_right() const  { return Vec<2, FT>(x_max_, y_min_); }
        /**
         * \brief Returns the position of the rectangle's bottom-left corner.
         * \return The position of the bottom-left corner.
         */
        Vec<2, FT> bottom_left() const  { return Vec<2, FT>(x_min_, y_max_); }

        /**
         * \brief Returns the minimum point of the rectangle.
         * \return The minimum point.
         */
        Vec<2, FT> min_point() const { return Vec<2, FT>(x_min_, y_min_); }
        /**
         * \brief Returns the maximum point of the rectangle.
         * \return The maximum point.
         */
        Vec<2, FT> max_point() const { return Vec<2, FT>(x_max_, y_max_); }

        /**
         * \brief Returns the center point of the rectangle.
         * \return The center point.
         */
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


#endif  // EASY3D_CORE_RECTANGLE_H
