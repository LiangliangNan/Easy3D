/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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


#ifndef _MATH_LINE_H_
#define _MATH_LINE_H_

#include "vec.h"


template <int DIM, class FT>
class GenericLine {
public:
	typedef Vec<DIM, FT>		 Point;
	typedef Vec<DIM, FT>		 Vector;
	typedef GenericLine<DIM, FT> thisclass;

public:
	static GenericLine from_point_and_direction(const Point& p, const Vector& dir) { return GenericLine(p, dir); }
	static GenericLine from_two_points(const Point& p, const Point& q)   { return GenericLine(p, q - p); }

	GenericLine() {}
	void set(const Point& p, const Vector& dir) { 
		p_ = p; 
		dir_ = normalize(dir);
	}

	const Vector& direction() const { return dir_; }

	const Point&  point() const { return p_; }

	GenericLine opposite() const { return GenericLine(p_, -dir_); }

	// the projection of p on this line
	Point  projection(const Point &p) const { return p_ + dir_ * dot(p - p_, dir_); }

	FT	   squared_ditance(const Point &p) const { return length2(projection(p) - p); }

private:  // Ambiguities exist for this one.
	GenericLine(const Point & p, const Vector & dir);

private:
	Point	p_;
	Vector  dir_;
};


template <int DIM, class FT> inline
GenericLine<DIM, FT>::GenericLine(const Point & p, const Vector & dir) : p_(p) {
	dir_ = normalize(dir);

#ifndef NDEBUG // degenerate case
	if (length(dir_) < 1e-15) {
		std::cerr << "degenerate line constructed from point ("
			<< p << ") and direction (" << dir << ")" << std::endl;
	}
#endif
}


template <int DIM, class FT> inline
std::ostream& operator<<(std::ostream& os, const GenericLine<DIM, FT>& line) {
	return os << line.point() << " " << line.direction();
}


template <int DIM, class FT> inline
std::istream& operator>>(std::istream& is, GenericLine<DIM, FT>& line) {
    typename GenericLine<DIM, FT>::Point	p;
    typename GenericLine<DIM, FT>::Vector dir;
	is >> p >> dir;
	line.set(p, dir);
	return is;
}

#endif

