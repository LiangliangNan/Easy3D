// Title:   Vector3D.h
// Created: Thu Sep 25 14:35:05 2003
// Authors: Richard Keiser, Oliver Knoll, Mark Pauly, Matthias Zwicker
//
// Copyright (c) 2001, 2002, 2003 Computer Graphics Lab, ETH Zurich
//
// This file is part of the Pointshop3D system.
// See http://www.pointshop3d.com/ for more information.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.
//
// Contact info@pointshop3d.com if any conditions of this
// licensing are not clear to you.
//

#ifndef __VECTOR3D_H_
#define __VECTOR3D_H_

#pragma warning(disable:4201) // disable anonymous unions warning
#pragma warning (disable:4244) //disable warning about conversion from double to float
#pragma warning (disable:4305) // disable warning about truncation from double to float

#include <cmath>


namespace kdtree  {


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef SWAP
#define SWAP(x,y) \
	tmp = entry[x][y];	\
	entry[x][y] = entry[y][x];	\
	entry[y][x] = tmp;	
#endif

	////////////////////////////////////////////////////////////////////////////
	//
	// 3D vector
	//

	class Vector3D {

	public:

		/////////////////////////////////////////////////////////////////
		// constructors
		/////////////////////////////////////////////////////////////////	
		inline Vector3D (void)	{};

		/////////////////////////////////////////////////////////////////

		inline Vector3D (float p0, float p1, float p2)
		{ 
			p[0] = p0;
			p[1] = p1;
			p[2] = p2; 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D (float* pp)
		{ 
			p[0] = pp[0];
			p[1] = pp[1];
			p[2] = pp[2]; 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D (const Vector3D& v)
		{ 
			*this = v;
		};

		/////////////////////////////////////////////////////////////////
		// overloaded operators
		/////////////////////////////////////////////////////////////////

		//inline float& operator[] (int Index)	
		//{
		//	return (p[Index]);
		//};


		/////////////////////////////////////////////////////////////////

		//inline const float& operator[] (int Index) const	
		//{
		//	return (p[Index]);
		//};


		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator= (const Vector3D& P)
		{
			p[0] = P[0];
			p[1] = P[1];
			p[2] = P[2];
			return (*this);
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator= (const float * pp)
		{
			p[0] = pp[0];
			p[1] = pp[1];
			p[2] = pp[2];
			return (*this);
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator+= (Vector3D P)
		{
			p[0] += P[0];
			p[1] += P[1];
			p[2] += P[2];
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator-= (Vector3D P)
		{
			p[0] -= P[0];
			p[1] -= P[1];
			p[2] -= P[2];
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator*= (Vector3D P)
		{
			p[0] *= P[0];
			p[1] *= P[1];
			p[2] *= P[2];
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator*= (float s)
		{
			p[0] *= s;
			p[1] *= s;
			p[2] *= s;
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator/= (Vector3D P)
		{
			p[0] /= P[0];
			p[1] /= P[1];
			p[2] /= P[2];
			return (*this);
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D& operator/= (float s)
		{
			p[0] /= s;
			p[1] /= s;
			p[2] /= s;
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D operator+ (const Vector3D P) const
		{
			Vector3D res;
			res[0] = p[0] + P[0];
			res[1] = p[1] + P[1];
			res[2] = p[2] + P[2];
			return (res); 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D operator- (const Vector3D P) const
		{
			Vector3D res;
			res[0] = p[0] - P[0];
			res[1] = p[1] - P[1];
			res[2] = p[2] - P[2];
			return (res); 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D operator- () const
		{
			Vector3D res;
			res[0] = -p[0];
			res[1] = -p[1];
			res[2] = -p[2];
			return (res); 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D operator* (const Vector3D P) const
		{
			Vector3D res;
			res[0] = p[0] * P[0];
			res[1] = p[1] * P[1];
			res[2] = p[2] * P[2];
			return (res); 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D operator* (float s) const
		{
			Vector3D res;
			res[0] = p[0] * s;
			res[1] = p[1] * s;
			res[2] = p[2] * s;
			return (res); 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector3D operator/ (const Vector3D P) const
		{
			Vector3D res;
			res[0] = p[0] / P[0];
			res[1] = p[1] / P[1];
			res[2] = p[2] / P[2];
			return (res); 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector3D operator/ (float s) const
		{
			Vector3D res;
			float invS = 1.0f / s;
			res[0] = p[0] * invS;
			res[1] = p[1] * invS;
			res[2] = p[2] * invS;
			return (res); 
		};



		/////////////////////////////////////////////////////////////////

		inline bool operator == (const Vector3D &a) const
		{
			return(p[0] == a[0] && p[1] == a[1] && p[2] == a[2]);
		};


		/////////////////////////////////////////////////////////////////

		inline bool operator != (const Vector3D &a) const
		{
			return(p[0] != a[0] || p[1] != a[1] || p[2] != a[2]);
		};


		/////////////////////////////////////////////////////////////////

		inline bool operator < (const Vector3D &a) const
		{
			return(p[0] < a[0] && p[1] < a[1] && p[2] < a[2]);
		};


		/////////////////////////////////////////////////////////////////

		inline bool operator >= (const Vector3D &a) const
		{
			return(p[0] >= a[0] && p[1] >= a[1] && p[2] >= a[2]);
		};


		/////////////////////////////////////////////////////////////////
		inline float* source (void) { return p; };

		/////////////////////////////////////////////////////////////////
		inline operator float * () { return p; };

		/////////////////////////////////////////////////////////////////
		inline operator const float * () const { return p; };

		/////////////////////////////////////////////////////////////////

		inline void makeZero()
		{
			p[0] = p[1] = p[2] = 0;
		};

		/////////////////////////////////////////////////////////////////

		inline void makeNegative() {
			p[0] = -p[0];
			p[1] = -p[1];
			p[2] = -p[2];
		};

		/////////////////////////////////////////////////////////////////

		inline float getSquaredLength() const
		{
			return( p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
		};

		/////////////////////////////////////////////////////////////////

		inline float getLength() const
		{
			return (float)sqrt (getSquaredLength()); 
		};


		/////////////////////////////////////////////////////////////////
		// normalize and return length

		inline float normalize (void) 
		{
			float length = getLength();
			if (length == 0.0f)
				return 0;

			float rezLength = 1.0f / length;
			p[0] *= rezLength;
			p[1] *= rezLength;
			p[2] *= rezLength;
			return length;
		};

		/////////////////////////////////////////////////////////////////
		// vector between the two points

		inline void fromTo (Vector3D& P1, Vector3D& P2)
		{
			p[0] = P2[0] - P1[0];
			p[1] = P2[1] - P1[1];
			p[2] = P2[2] - P1[2];
		};

		/**
		* returns two orthogonal vectors u and v
		* attention: u and v are not normalized
		*/
		inline void getTwoOrthogonals (Vector3D &u, Vector3D &v) const
		{
			if (fabs(p[0]) < fabs(p[1])) {
				if (fabs(p[0]) < fabs(p[2])) {
					// p[0] is minimum
					u = Vector3D (0, -p[2], p[1]);
				}
				else {
					// p[2] is mimimum
					u = Vector3D (-p[1], p[0], 0);
				}
			}
			else { 
				if (fabs(p[1]) < fabs(p[2])) {

					// p[1] is minimum
					u = Vector3D (p[2], 0, -p[0]);
				}
				else {
					// p[2] is mimimum
					u = Vector3D(-p[1], p[0], 0);
				}
			}
			v = crossProduct (*this, u);
		}

		/**
		* returns two normalized orthogonal vectors u and v
		*/
		inline void getTwoNormalizedOrthogonals (Vector3D &u, Vector3D &v) const
		{
			if (fabs(p[0]) < fabs(p[1])) {
				if (fabs(p[0]) < fabs(p[2])) {
					// p[0] is minimum
					u = Vector3D (0, -p[2], p[1]);
				}
				else {
					// p[2] is mimimum
					u = Vector3D (-p[1], p[0], 0);
				}
			}
			else { 
				if (fabs(p[1]) < fabs(p[2])) {

					// p[1] is minimum
					u = Vector3D (p[2], 0, -p[0]);
				}
				else {
					// p[2] is mimimum
					u = Vector3D(-p[1], p[0], 0);
				}
			}
			v = crossProduct (*this, u);

			u.normalize();
			v.normalize();
		}


		static inline Vector3D crossProduct(const Vector3D &a, const Vector3D &b)
		{
			Vector3D result;

			result[0] = a[1] * b[2] - a[2] * b[1];
			result[1] = a[2] * b[0] - a[0] * b[2];
			result[2] = a[0] * b[1] - a[1] * b[0];

			return(result);
		}


		static inline float dotProduct(const Vector3D &a, const Vector3D &b)
		{
			return(a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
		}


		static inline float squaredDistance (const Vector3D &v1, const Vector3D &v2)
		{
			return ((v1 - v2).getSquaredLength());
		}


		static inline float distance (const Vector3D &v1, const Vector3D &v2)
		{
			return ((v1 - v2).getLength());
		}




		/**
		* converts cartesion to polar coordinates
		* result:
		* [0] = length
		* [1] = angle with z-axis
		* [2] = angle of projection into x,y, plane with x-axis
		*/	
		static inline Vector3D cartesianToPolar (Vector3D &v)
		{
			Vector3D polar;
			polar[0] = v.getLength();

			if (v[2] > 0.0f) {
				polar[1] = (float) atan (sqrt (v[0] * v[0] + v[1] * v[1]) / v[2]);
			}
			else if (v[2] < 0.0f) {
				polar[1] = (float) atan (sqrt (v[0] * v[0] + v[1] * v[1]) / v[2]) + M_PI;
			}
			else {
				polar[1] = M_PI * 0.5f;
			}


			if (v[0] > 0.0f) {
				polar[2] = (float) atan (v[1] / v[0]);
			}
			else if (v[0] < 0.0f) {
				polar[2] = (float) atan (v[1] / v[0]) + M_PI;
			}
			else if (v[1] > 0) {
				polar[2] = M_PI * 0.5f;
			}
			else {
				polar[2] = -M_PI * 0.5;
			}
			return polar;
		}



		/**
		* converts polar to cartesion coordinates
		* input:
		* [0] = length
		* [1] = angle with z-axis
		* [2] = angle of projection into x,y, plane with x-axis
		*/	
		static inline Vector3D polarToCartesian (Vector3D &v)
		{
			Vector3D cart;
			cart[0] = v[0] * (float) sin (v[1]) * (float) cos (v[2]);
			cart[1] = v[0] * (float) sin (v[1]) * (float) sin (v[2]);
			cart[2] = v[0] * (float) cos (v[1]);
			return cart;
		}


		/////////////////////////////////////////////////////////////////

		static inline Vector3D projectOntoVector (Vector3D &v1, Vector3D &v2)
		{
			return v2 * dotProduct (v1, v2);
		}


		static inline Vector3D projectVectorIntoPlane (Vector3D &v1, Vector3D &normal)
		{
			return v1 - projectOntoVector (v1, normal);
		}


		static inline Vector3D projectPointOntoPlane (Vector3D &point, Vector3D &anchor, Vector3D &normal) 
		{

			Vector3D temp = point - anchor;
			return point - projectOntoVector (temp, normal);
		}


	public:

		union
		{
			float p[3];
			struct
			{
				float x, y, z;
			};
		};
	};

	////////////////////////////////////////////////////////////////////////////

	inline Vector3D operator * (float s, const Vector3D &P)
	{
		return (P * s);
	}

}


#endif  // __VECTOR3D_H_

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
