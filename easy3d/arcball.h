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


#ifndef EASY3D_ARCBALL_H
#define EASY3D_ARCBALL_H

#include <easy3d/math_types.h>


namespace easy3d {

	class Arcball
	{
	public:
		Arcball();
		Arcball(float radius);
		virtual ~Arcball() {}

		const mat3& current_rotation();
		const mat4& gl_current_rotation();
		const mat3& get_incremental_rotation() const { return m_incremental; }

		void reset();

	public:
		void begin_drag(float x, float y);
		void drag(float x, float y); 
		void end_drag(float x, float y);

	private:
		void project_onto_surface(vec3& p);
		void compute_incremental(const vec3& anchor, const vec3& current, mat3& transform);

	protected:
		float   m_radius;
		vec3    m_anchor_position;
		vec3    m_current_position;
		float   m_angle;
		vec3    m_axis;
		mat3    m_anchor;
		mat3    m_incremental;
		mat3    m_current;
		mat4	m_gl_current;
	};


}

#endif	// EASY3D_ARCBALL_H
