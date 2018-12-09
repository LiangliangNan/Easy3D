#ifndef OPENTISSUE_UTILITY_TRACKBALL_GENERIC_TRACKBALL_H
#define OPENTISSUE_UTILITY_TRACKBALL_GENERIC_TRACKBALL_H
//
// OpenTissue Template Library
// - A generic toolbox for physics-based modeling and simulation.
// Copyright (C) 2008 Department of Computer Science, University of Copenhagen.
//
// OTTL is licensed under zlib: http://opensource.org/licenses/zlib-license.php
//
#include "math_types.h"



template<typename real_type_ = double>
class GenericTrackball
{
public:

	typedef  real_type_						real_type;
	typedef  Vec<3,	real_type>				vector3_type;
	typedef  Mat3<real_type>				matrix3x3_type;
	typedef  Quat<real_type>				quaternion_type;
	typedef  real_type						gl_transform_type[16];

protected:

	real_type         m_radius;                 ///< TBD
	vector3_type      m_anchor_position;        ///< TBD
	vector3_type      m_current_position;       ///< TBD
	real_type         m_angle;                  ///< TBD
	vector3_type      m_axis;                   ///< TBD
	matrix3x3_type    m_xform_anchor;           ///< TBD
	matrix3x3_type    m_xform_incremental;      ///< TBD
	matrix3x3_type    m_xform_current;          ///< TBD
	gl_transform_type m_gl_xform_current;       ///< TBD

public:

	real_type          & radius() { return m_radius; }
	real_type const    & radius()  const { return m_radius; }
	vector3_type       & anchor() { m_anchor_position = normalize(m_anchor_position); return m_anchor_position; }
	vector3_type const & anchor()  const { m_anchor_position = unit(m_anchor_position); return m_anchor_position; }
	vector3_type       & current() { m_current_position = normalize(m_current_position); return m_current_position; }
	vector3_type const & current() const { m_current_position = unit(m_current_position); return m_current_position; }
	real_type          & angle() { return m_angle; }
	real_type const    & angle()   const { return m_angle; }
	vector3_type       & axis() { m_axis = unit(m_axis); return m_axis; }
	vector3_type const & axis()    const { m_axis = unit(m_axis); return m_axis; }

	matrix3x3_type const & get_current_rotation()
	{
		// Compute the the rotation from Panchor to Pcurrent, i.e.
		// the rotation form (Xanchor, Yanchor, Zanchor) to
		// (Xcurrent, Ycurrent, Zcurrent) along a great circle.
		// Multiply the IncrementalTransformation and the AnchorTransformation
		// to get the CurrentTransformation.
		m_xform_current = m_xform_incremental * m_xform_anchor;
		return m_xform_current;
	}

	gl_transform_type const & get_gl_current_rotation()
	{
		// Compute the the rotation from Panchor to Pcurrent, i.e.
		// the rotation form (Xanchor, Yanchor, Zanchor) to
		// (Xcurrent, Ycurrent, Zcurrent) along a great circle.
		// Multiply the IncrementalTransformation and the AnchorTransformation
		// to get the CurrentTransformation.
		m_xform_current = m_xform_incremental * m_xform_anchor;

		m_gl_xform_current[0] = m_xform_current(0, 0);
		m_gl_xform_current[1] = m_xform_current(1, 0);
		m_gl_xform_current[2] = m_xform_current(2, 0);
		m_gl_xform_current[3] = 0;		
		m_gl_xform_current[4] = m_xform_current(0, 1);
		m_gl_xform_current[5] = m_xform_current(1, 1);
		m_gl_xform_current[6] = m_xform_current(2, 1);
		m_gl_xform_current[7] = 0;			
		m_gl_xform_current[8] = m_xform_current(0, 2);
		m_gl_xform_current[9] = m_xform_current(1, 2);
		m_gl_xform_current[10] =m_xform_current(2, 2);
		m_gl_xform_current[11] = 0;
		m_gl_xform_current[12] = 0;
		m_gl_xform_current[13] = 0;
		m_gl_xform_current[14] = 0;
		m_gl_xform_current[15] = 1;

		return m_gl_xform_current;
	}

	matrix3x3_type const & get_incremental_rotation() const { return m_xform_incremental; }

public:

	GenericTrackball()
		: m_radius(1.0)
	{}

	GenericTrackball(real_type radius)
		: m_radius(radius)
	{}

	virtual ~GenericTrackball() {}

	virtual void reset()
	{
		m_anchor_position = vector3_type(0.0);
		m_current_position = vector3_type(0.0);
		m_axis = vector3_type(0.0);
		m_angle = real_type(0.0);
		m_xform_anchor = matrix3x3_type(1.0);
		m_xform_incremental = matrix3x3_type(1.0);
		m_xform_current = matrix3x3_type(1.0);
	}

public:

	virtual void begin_drag(real_type const & x, real_type const & y) = 0;
	virtual void drag(real_type const & x, real_type const & y) = 0;
	virtual void end_drag(real_type const & x, real_type const & y) = 0;

private:

	virtual void compute_incremental(vector3_type const & anchor, vector3_type const & current, matrix3x3_type & transform) = 0;

};


// OPENTISSUE_UTILITY_TRACKBALL_GENERIC_TRACKBALL_H
#endif
