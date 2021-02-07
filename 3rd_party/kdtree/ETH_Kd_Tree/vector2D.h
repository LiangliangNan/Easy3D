// Title:   Vector2D.h
// Created: Thu Sep 25 14:35:07 2003
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

#ifndef __VECTOR2D_H_
#define __VECTOR2D_H_

#pragma warning(disable:4201) // disable anonymous unions warning
#pragma warning(disable:4244) // disable warning about conversion from double to float
#pragma warning(disable:4305) // disable warning about truncation from double to float

#include <cmath>


namespace kdtree {

	////////////////////////////////////////////////////////////////////////////
	//
	// 2D vector
	//

	class Vector2D {

	public:

		/////////////////////////////////////////////////////////////////
		// constructors
		/////////////////////////////////////////////////////////////////	
		inline Vector2D (void)	{};


		/////////////////////////////////////////////////////////////////

		inline Vector2D (float p0, float p1)
		{ 
			p[0] = p0;
			p[1] = p1;
		};

		virtual ~Vector2D() {}


		/////////////////////////////////////////////////////////////////

		inline Vector2D (float* pp)
		{ 
			p[0] = pp[0];
			p[1] = pp[1];
		};


		/////////////////////////////////////////////////////////////////
		// overloaded operators
		/////////////////////////////////////////////////////////////////

		inline float& operator[] (int Index)	
		{
			return (p[Index]);
		};


		/////////////////////////////////////////////////////////////////

		inline const float& operator[] (int Index) const	
		{
			return (p[Index]);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator= (const Vector2D& P)
		{
			p[0] = P[0];
			p[1] = P[1];
			return (*this);
		};

		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator+= (Vector2D P)
		{
			p[0] += P[0];
			p[1] += P[1];
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator-= (Vector2D P)
		{
			p[0] -= P[0];
			p[1] -= P[1];
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator*= (Vector2D P)
		{
			p[0] *= P[0];
			p[1] *= P[1];
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator*= (float s)
		{
			p[0] *= s;
			p[1] *= s;
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator/= (Vector2D P)
		{
			p[0] /= P[0];
			p[1] /= P[1];
			return (*this);
		};

		/////////////////////////////////////////////////////////////////

		inline Vector2D& operator/= (float s)
		{
			p[0] /= s;
			p[1] /= s;
			return (*this);
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D operator+ (const Vector2D P) const
		{
			Vector2D res;
			res[0] = p[0] + P[0];
			res[1] = p[1] + P[1];
			return (res); 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D operator- (const Vector2D P) const
		{
			Vector2D res;
			res[0] = p[0] - P[0];
			res[1] = p[1] - P[1];
			return (res); 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D operator- () const
		{
			Vector2D res;
			res[0] = -p[0];
			res[1] = -p[1];
			return (res); 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector2D operator* (const Vector2D P) const
		{
			Vector2D res;
			res[0] = p[0] * P[0];
			res[1] = p[1] * P[1];
			return (res); 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector2D operator* (float s) const
		{
			Vector2D res;
			res[0] = p[0] * s;
			res[1] = p[1] * s;
			return (res); 
		};


		/////////////////////////////////////////////////////////////////

		inline Vector2D operator/ (const Vector2D P) const
		{
			Vector2D res;
			res[0] = p[0] / P[0];
			res[1] = p[1] / P[1];
			return (res); 
		};

		/////////////////////////////////////////////////////////////////

		inline Vector2D operator/ (float s) const
		{
			Vector2D res;
			res[0] = p[0] / s;
			res[1] = p[1] / s;
			return (res); 
		};



		/////////////////////////////////////////////////////////////////

		inline bool operator == (const Vector2D &a) const
		{
			return(p[0] == a[0] && p[1] == a[1]);
		};


		/////////////////////////////////////////////////////////////////

		inline bool operator != (const Vector2D &a) const
		{
			return(p[0] != a[0] || p[1] != a[1]);
		};


		/////////////////////////////////////////////////////////////////

		inline bool operator < (const Vector2D &a) const
		{
			return(p[0] < a[0] && p[1] < a[1]);
		};


		/////////////////////////////////////////////////////////////////

		inline bool operator >= (const Vector2D &a) const
		{
			return(p[0] >= a[0] && p[1] >= a[1]);
		};


		/////////////////////////////////////////////////////////////////
		inline float* source (void) { return p; };


		/////////////////////////////////////////////////////////////////

		inline Vector2D& makeZero()
		{
			p[0] = p[1] = 0;

			return (*this);
		}	






		/////////////////////////////////////////////////////////////////

		inline float getLength() const 
		{
			return (float)sqrt (p[0] * p[0] + p[1] * p[1]); 
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
			return length;
		};

		/////////////////////////////////////////////////////////////////
		// vector between the two points

		inline void fromTo (Vector2D& P1, Vector2D& P2)
		{
			p[0] = P2[0] - P1[0];
			p[1] = P2[1] - P1[1];
		};


		/////////////////////////////////////////////////////////////////

		static inline Vector2D scalarMult (float s, const Vector2D &P)
		{
			return (P * s);
		}


		static inline float dotProduct(const Vector2D &a, const Vector2D &b)
		{
			return(a[0] * b[0] + a[1] * b[1]);
		}


		static inline float squaredLength(const Vector2D &v)
		{
			return(dotProduct(v, v));
		}


		static inline float squaredDistance (const Vector2D &v1, const Vector2D &v2)
		{
			Vector2D tmp = v1 - v2;
			return (squaredLength(tmp));
		}


		static inline float distance (const Vector2D &v1, const Vector2D &v2)
		{
			Vector2D tmp = v1 - v2;
			return (tmp.getLength());
		}





		static inline Vector2D crossProduct(const Vector2D &a, const Vector2D &b)
		{
			Vector2D result;

			result[0] = a[1] * b[0] - a[0] * b[1];
			result[1] = a[0] * b[1] - a[1] * b[0];

			return(result);
		}


		/////////////////////////////////////////////////////////////////

		static inline Vector2D projectOntoVector (Vector2D &v1, Vector2D &v2)
		{
			return scalarMult (dotProduct(v1, v2), v2);
		}




	public:

		union
		{
			float p[2];
			struct
			{
				float x, y;
			};
		};

	};


	////////////////////////////////////////////////////////////////////////////

	inline Vector2D operator * (float s, const Vector2D &P)
	{
		return (P * s);
	}


}

#endif  // __VECTOR2D_H_

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
