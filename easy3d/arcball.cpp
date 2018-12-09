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

#include <easy3d/arcball.h>


namespace easy3d {

	Arcball::Arcball()
		: m_radius(1.0)
	{
		reset();
	}

	Arcball::Arcball(float radius)
		: m_radius(radius)
	{
		reset();
	}


	const mat3& Arcball::current_rotation() {
		// Compute the the rotation from Panchor to Pcurrent, i.e.
		// the rotation form (Xanchor, Yanchor, Zanchor) to
		// (Xcurrent, Ycurrent, Zcurrent) along a great circle.
		// Multiply the IncrementalTransformation and the AnchorTransformation
		// to get the CurrentTransformation.
		m_current = m_incremental * m_anchor;
		return m_current;
	}

	const mat4& Arcball::gl_current_rotation() {
		// Compute the the rotation from Panchor to Pcurrent, i.e.
		// the rotation form (Xanchor, Yanchor, Zanchor) to
		// (Xcurrent, Ycurrent, Zcurrent) along a great circle.
		// Multiply the IncrementalTransformation and the AnchorTransformation
		// to get the CurrentTransformation.
		m_current = m_incremental * m_anchor;

		m_gl_current[0] = m_current(0, 0);
		m_gl_current[1] = m_current(1, 0);
		m_gl_current[2] = m_current(2, 0);
		m_gl_current[3] = 0;
		m_gl_current[4] = m_current(0, 1);
		m_gl_current[5] = m_current(1, 1);
		m_gl_current[6] = m_current(2, 1);
		m_gl_current[7] = 0;
		m_gl_current[8] = m_current(0, 2);
		m_gl_current[9] = m_current(1, 2);
		m_gl_current[10] = m_current(2, 2);
		m_gl_current[11] = 0;
		m_gl_current[12] = 0;
		m_gl_current[13] = 0;
		m_gl_current[14] = 0;
		m_gl_current[15] = 1;

		return m_gl_current;
	}

	void Arcball::reset() {
		m_anchor_position = vec3(0.0);
		m_current_position = vec3(0.0);
		m_axis = vec3(0.0);
		m_angle = float(0.0);
		m_anchor = mat3(1.0);
		m_incremental = mat3(1.0);
		m_current = mat3(1.0);

		project_onto_surface(m_anchor_position);
		project_onto_surface(m_current_position);
	}

	void Arcball::begin_drag(float x, float y)
	{
		m_angle = 0.0;
		m_axis = vec3(0.0);

		m_anchor = m_current;
		m_incremental = mat3(1.0);
		m_current = mat3(1.0);

		m_anchor_position = vec3(x, y, 0);
		project_onto_surface(m_anchor_position);
		m_current_position = vec3(x, y, 0);
		project_onto_surface(m_current_position);
	}

	void Arcball::drag(float x, float y)
	{
		m_current_position = vec3(x, y, 0);
		project_onto_surface(m_current_position);
		compute_incremental(m_anchor_position, m_current_position, m_incremental);
	}

	void Arcball::end_drag(float x, float y)
	{
		m_current_position = vec3(x, y, 0);
		project_onto_surface(m_current_position);
		compute_incremental(m_anchor_position, m_current_position, m_incremental);
	}

	void Arcball::project_onto_surface(vec3& p) {
		using std::sqrt;
		const static float radius2 = m_radius * m_radius;
		float length2 = p[0] * p[0] + p[1] * p[1];
		if (length2 <= radius2 / 2.0)
			p[2] = sqrt(radius2 - length2);
		else {
			p[2] = radius2 / (2.0f * sqrt(length2));
			float length = sqrt(length2 + p[2] * p[2]);
			p /= length;
		}
		p.normalize();
	}

	void Arcball::compute_incremental(const vec3& anchor, const vec3& current, mat3& transform) {
		const vec3 axis = cross(anchor, current);
		m_axis = normalize(axis);
		m_angle = atan2(length(axis), dot(anchor, current));
		transform = mat3::rotation(m_axis, m_angle);
	}
}
