/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*
*	This file is part of Mapple: software for processing and rendering
*   meshes and point clouds.
*
*	Mapple is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Mapple is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MPL_OPENGL_PRIMITIVES_H_
#define _MPL_OPENGL_PRIMITIVES_H_

#include <3rd_party/glew/include/GL/glew.h>
#include <easy3d/model/math_types.h>


// TODO: add disk, cylinder, sphere, etc.

// Basic shape rendering.
// Functions to draw basic shapes, allowing the application to provide its own shader.

namespace easy3d
{

	// Draw a 2D quad with a single vertex attribute.
	// Renders a quad at (x, y) and of size (w, h) using the currently-bound shader
	// Binds the vertex position array to the given attribute array index and draws the object
	// using the bound shader
	// \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
	// \param[in] (x, y)		  the position (i.e., min corner) of the quad.
	// \param[in] (w, h)		  the size (i.e., width and height) of the quad.
	// \param[in] (vpw, vph)	  the size (i.e., width and height) of the viewport.
	void draw_quad(GLuint posIndex, int x, int y, int w, int h, int vpw, int vph, float depth);
	void draw_quad_wire(GLuint posIndex, int x, int y, int w, int h, int vpw, int vph, float depth);

	// Draw a 2D quad with positions and UVs.
	// Renders a quad at (x, y) and of size (w, h) using the currently-bound shader
	// Binds the vertex position and UVs arrays to the given attribute array indices and draws the object
	// using the bound shader
	// \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
	// \param[in] texcoordAttrib the vertex attribute array index that represents 2D UVs in the current shader
	// \param[in] (x, y)		  the position (i.e., min corner) of the quad.
	// \param[in] (w, h)		  the size (i.e., width and height) of the quad.
	// \param[in] (vpw, vph)	  the size (i.e., width and height) of the viewport.
	// \param[in] depth	the depth at which the quad will be drawn (in Normalized Device Coordinates, i.e., [-1.0, 1.0])
	// NOTE: After clipping and division by w, depth coordinates range from −1 to 1, corresponding to the near and far clipping planes.
	void draw_quad(GLuint posIndex, GLuint texcoordAttrib, int x, int y, int w, int h, int vpw, int vph, float depth);

	// Draw a 2D quad with a single vertex attribute.
	// Renders a normalized quad ([-1.0, 1.0] in X and Y) using the currently-bound shader
	// Binds the vertex position array to the given attribute array index and draws the object
	// using the bound shader
	// \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
	void draw_full_screen_quad(GLuint posIndex);

	// Draw a 2D quad with positions and UVs.
	// Renders a normalized quad ([-1.0, 1.0] in X and Y) with normalized UVs [0.0, 1.0]
	// using the currently-bound shader
	// Binds the vertex position and UVs arrays to the given attribute array indices and draws the object
	// using the bound shader
	// \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
	// \param[in] texcoordAttrib the vertex attribute array index that represents 2D UVs in the current shader
	// \param[in] depth	the depth at which the quad will be drawn (in Normalized Device Coordinates, i.e., [-1.0, 1.0])
	// NOTE: After clipping and division by w, depth coordinates range from −1 to 1, corresponding to the near and far clipping planes.
	void draw_full_screen_quad(GLuint positionAttrib, GLuint texcoordAttrib, float depth);

	// Draw a cube with positions.
	// Renders a normalized cube ([-1.0, 1.0] in X, Y, and Z) using the currently-bound shader
	// Binds the vertex position array to the given attribute array index and draws the object
	// using the bound shader
	// \param[in] posIndex the vertex attribute array index that represents position in the current shader
	void draw_cube(GLuint posIndex);

	// Draw a wireframe cube with positions.
	// Renders a normalized cube ([-1.0, 1.0] in X, Y, and Z) as lines using the currently-bound shader
	// Binds the vertex position array to the given attribute array index and draws the object
	// using the bound shader
	// \param[in] posIndex the vertex attribute array index that represents position in the current shader
	void draw_cube_wire(GLuint posIndex);

	// Draw a single point with given position.
	// Renders a single point (GL_POINTS) using the currently-bound shader
	// Binds the vertex position array to the given attribute array index and draws one point
	// using the bound shader
	// \param[in] pos the point to be drawn
	// \param[in] posIndex the vertex attribute array index that represents position in the current shader
	void draw_point(GLuint positionAttrib, const vec3 &pos);

}


#endif