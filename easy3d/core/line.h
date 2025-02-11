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

#ifndef EASY3D_CORE_LINE_H
#define EASY3D_CORE_LINE_H

#include <easy3d/core/vec.h>
#include <easy3d/core/constant.h>
#include <easy3d/util/logging.h>


namespace easy3d {


	/**
	 * \brief A generic line representation, which supports both 2D and 3D lines.
	 * \tparam DIM The dimension of the line (2 or 3).
	 * \tparam FT The type of the floating-point number (float or double).
	 * \class GenericLine easy3d/core/line.h
	 */
    template <int DIM, typename FT>
    class GenericLine {
    public:
    	typedef Vec<DIM, FT> Point;				///< The type of the point.
    	typedef Vec<DIM, FT> Vector;			///< The type of the vector.
    	typedef GenericLine<DIM, FT> thisclass; ///< The type of this class.

    public:
    	/**
		 * \brief Constructs a line from a point \p p and its direction \p dir.
		 * \param p The point on the line.
		 * \param dir The direction of the line.
		 * \return A GenericLine object.
		 */
        static GenericLine from_point_and_direction(const Point& p, const Vector& dir) { return GenericLine(p, dir); }
    	/**
		 * \brief Constructs a line from two points \p p and \p q.
		 * \param p The first point on the line.
		 * \param q The second point on the line.
		 * \return A GenericLine object.
		 */
        static GenericLine from_two_points(const Point& p, const Point& q)   { return GenericLine(p, q - p); }

    	/**
		 * \brief Default constructor.
		 */
        GenericLine() = default;

    	/**
		 * \brief Sets a line from a point \p p and its direction \p dir.
		 * \param p The point on the line.
		 * \param dir The direction of the line.
		 */
        void set(const Point& p, const Vector& dir) {
            p_ = p;
            dir_ = normalize(dir);
        }

    	/**
		 * \brief Returns the direction of the line.
		 * \return The direction vector of the line.
		 */
        const Vector& direction() const { return dir_; }

    	/**
		 * \brief Returns an arbitrary point on the line.
		 * \return The point on the line.
		 */
        const Point& point() const { return p_; }

    	/**
		 * \brief Returns the projection of a point \p p on this line.
		 * \param p The point to project.
		 * \return The projection of the point on the line.
		 */
        Point projection(const Point &p) const { return p_ + dir_ * dot(p - p_, dir_); }

    	/**
		 * \brief Returns the squared distance of a point \p p to this line.
		 * \param p The point to measure the distance to.
		 * \return The squared distance of the point to the line.
		 */
        FT squared_distance(const Point &p) const { return length2(projection(p) - p); }

    	/**
		 * \brief Computes the perpendicular feet with another line.
		 * \param other Another line.
		 * \param p1 The perpendicular foot on the current line.
		 * \param p2 The perpendicular foot on the other line.
		 * \return True if the perpendicular feet exist, false if the two lines are parallel.
		 * \note This function is for 3D only.
		 */
        bool feet(const thisclass& other, Point& p1, Point& p2) const;

    private:  // Ambiguities exist for this one.
        GenericLine(const Point & p, const Vector & dir);

    private:
    	Point p_;		///< An arbitrary point on the line.
    	Vector dir_;	///< The direction of the line.
    };


    template <int DIM, typename FT>
    GenericLine<DIM, FT>::GenericLine(const Point & p, const Vector & dir) : p_(p) {
      dir_ = normalize(dir);
      LOG_IF(length(dir_) < 1e-15, ERROR)
              << "degenerate line constructed from point (" << p << ") and direction (" << dir << ")";
    }


	template <int DIM, typename FT>
	bool GenericLine<DIM, FT>::feet(const thisclass& other, Point& p1, Point& p2) const {
        // See the derivation: https://www.jianshu.com/p/34a7c4e1f3f5
		FT a = dot(dir_, other.direction());
		FT b = dot(dir_, dir_);
		FT c = dot(other.direction(), other.direction());
		if (std::abs(a * a - b * c) < epsilon<FT>()) // the two lines are colinear or parallel
			return false;
		FT d = dot(other.point() - p_, dir_);
		FT e = dot(other.point() - p_, other.direction());
		FT t1 = 0;
		FT t2 = 0;
		if (a == 0) {
			t1 = d / b;
			t2 = -e / c;
		}
		else {
			t1 = (a * e - c * d) / (a * a - b * c);
			t2 = b / a * t1 - d / a;
		}
		p1 = p_ + t1 * dir_;
		p2 = other.point() + t2 * other.direction();
		return true;
	}


	/**
	 * \brief Output stream support for GenericLine.
	 * \tparam DIM The dimension of the line.
	 * \tparam FT The type of the floating-point number.
	 * \param os The output stream.
	 * \param line The line to output.
	 * \return The output stream.
	 */
    template <int DIM, typename FT>
    std::ostream& operator<<(std::ostream& os, const GenericLine<DIM, FT>& line) {
        return os << line.point() << " " << line.direction();
    }

	/**
	 * \brief Input stream support for GenericLine.
	 * \tparam DIM The dimension of the line.
	 * \tparam FT The type of the floating-point number.
	 * \param is The input stream.
	 * \param line The line to input.
	 * \return The input stream.
	 */
    template <int DIM, typename FT>
    std::istream& operator>>(std::istream& is, GenericLine<DIM, FT>& line) {
        typename GenericLine<DIM, FT>::Point  p;
        typename GenericLine<DIM, FT>::Vector dir;
        is >> p >> dir;
        line.set(p, dir);
        return is;
    }

}

#endif  // EASY3D_CORE_LINE_H

