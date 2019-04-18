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


#ifndef EASY3D_PRIMITIVES_H
#define EASY3D_PRIMITIVES_H

#include <vector>

#include <easy3d/core/types.h>


// Functions to draw/create basic shapes, allowing the application to provide its own shader.

namespace easy3d {

    namespace opengl
    {

        // Draw a 2D quad with a single vertex attribute.
        // Renders a quad at (x, y) and of size (w, h) using the currently-bound shader
        // Binds the vertex position array to the given attribute array index and draws the object
        // using the bound shader
        // \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
        // \param[in] (x, y)		  the position (i.e., min corner) of the quad.
        // \param[in] (w, h)		  the size (i.e., width and height) of the quad.
        // \param[in] (vpw, vph)	  the size (i.e., width and height) of the viewport.
        void draw_quad(unsigned int posIndex, int x, int y, int w, int h, int vpw, int vph, float depth);
        void draw_quad_wire(unsigned int posIndex, int x, int y, int w, int h, int vpw, int vph, float depth);

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
        void draw_quad(unsigned int posIndex, unsigned int texcoordAttrib, int x, int y, int w, int h, int vpw, int vph, float depth);

        // Draw a 2D quad with a single vertex attribute.
        // Renders a normalized quad ([-1.0, 1.0] in X and Y) using the currently-bound shader
        // Binds the vertex position array to the given attribute array index and draws the object
        // using the bound shader
        // \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
        void draw_full_screen_quad(unsigned int posIndex);

        // Draw a 2D quad with positions and UVs.
        // Renders a normalized quad ([-1.0, 1.0] in X and Y) with normalized UVs [0.0, 1.0]
        // using the currently-bound shader
        // Binds the vertex position and UVs arrays to the given attribute array indices and draws the object
        // using the bound shader
        // \param[in] positionAttrib the vertex attribute array index that represents position in the current shader
        // \param[in] texcoordAttrib the vertex attribute array index that represents 2D UVs in the current shader
        // \param[in] depth	the depth at which the quad will be drawn (in Normalized Device Coordinates, i.e., [-1.0, 1.0])
        // NOTE: After clipping and division by w, depth coordinates range from −1 to 1, corresponding to the near and far clipping planes.
        void draw_full_screen_quad(unsigned int positionAttrib, unsigned int texcoordAttrib, float depth);

        // Draw a cube with positions.
        // Renders a normalized cube ([-1.0, 1.0] in X, Y, and Z) using the currently-bound shader
        // Binds the vertex position array to the given attribute array index and draws the object
        // using the bound shader
        // \param[in] posIndex the vertex attribute array index that represents position in the current shader
        void draw_cube(unsigned int posIndex);

        // Draw a wireframe cube with positions.
        // Renders a normalized cube ([-1.0, 1.0] in X, Y, and Z) as lines using the currently-bound shader
        // Binds the vertex position array to the given attribute array index and draws the object
        // using the bound shader
        // \param[in] posIndex the vertex attribute array index that represents position in the current shader
        void draw_cube_wire(unsigned int posIndex);

        // Draw a single point with given position.
        // Renders a single point (GL_POINTS) using the currently-bound shader
        // Binds the vertex position array to the given attribute array index and draws one point
        // using the bound shader
        // \param[in] pos the point to be drawn
        // \param[in] posIndex the vertex attribute array index that represents position in the current shader
        void draw_point(unsigned int positionAttrib, const vec3 &pos);



		//------  The following functions prepare data (points, normals, and colors) for rendering -----

		// Prepare data (points, normals, and colors) for a 3D sphere.
		// radius: the radius of the sphere.
		// slices: the number of subdivisions around the z axis (similar to lines of longitude).
		// stacks: the number of subdivisions along the z axis(similar to lines of latitude).
		void prepare_sphere(
			const vec3& center, double radius, int slices, int stacks, const vec3& color, 
			std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors
		);


		// Prepare data (points, normals, and colors) for a 3D checker sphere.
		// radius: the radius of the sphere.
		// slices: the number of subdivisions around the z axis (similar to lines of longitude).
		// stacks: the number of subdivisions along the z axis(similar to lines of latitude).
		// checker_size: how many tiles each checker will occupy
		// color1 and color2: the two interchanging colors
		void prepare_checker_sphere(
			const vec3& center, double radius, int slices, int stacks, int checker_size,
			const vec3& color1, const vec3& color2,
			std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors
		);


        // Prepare data (points, normals, and colors) for a 3D cylinder defined by two 3D points s and t.
        void prepare_cylinder(
			double radius, int slices, const vec3& s, const vec3& t, const vec3& color, 
			std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors
		);

		// Prepare data (points, normals, and colors) for a 3D cone defined by two 3D points b and t.
		// s is the base center and t is the tip.
		void prepare_cone(
			double radius, int slices, const vec3& s, const vec3& t, const vec3& color,
			std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors
		);


    }

}


#endif
