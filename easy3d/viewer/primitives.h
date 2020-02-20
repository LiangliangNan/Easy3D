/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_PRIMITIVES_H
#define EASY3D_PRIMITIVES_H

#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    namespace opengl
    {
        /**
         * Draw a quad defined in the screen space.
         * @param rect The quad.
         * @param color The color.
         * @param texture The texture.
         * @param width The width of the viewer.
         * @param height The height of the viewer.
         * @param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_quad_wire(const Rect& rect, const vec4& color, int width, int height, float depth);
        void draw_quad_filled(const Rect& rect, const vec4& color, int width, int height, float depth);
        void draw_quad_filled(const Rect& rect, int texture, int width, int height, float depth);

        /**
         * Draw a full screen textured quad.
         * @param rect The quad.
         * @param texture The texture.
         * @param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_full_screen_quad(int texture, float depth);

        /**
         * draw a quad visualizing a depth texture in a region. This is function is similar to
         * draw_quad_filled(... int texture...). The only difference is that a depth texture is rendered as a gray
         * scale image. For depth textures from off-screen rendering (usually from an FBO), the depth values are stored
         * in the R component of the texture.
         * @param rect The quad.
         * @param color The color.
         * @param texture The texture.
         * @param width The width of the viewer.
         * @param height The height of the viewer.
         * @param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_depth_texture(const Rect& rect, int texture, int width, int height, float depth);

        /**
         * Draw a quad  defined in the screen space using a bound shader. It binds the vertex position and UVs arrays
         * to the given attribute array indices and draws the quad.
         * @param position_attrib The vertex attribute array index that represents position in the current shader.
         * @param texcoord_attrib The vertex attribute array index that represents 2D UVs in the current shader.
         * @param (x, y) The position (i.e., min corner) of the quad.
         * @param (w, h) The size (i.e., width and height) of the quad.
         * @param (vpw, vph) The size (i.e., width and height) of the viewport (of the viewer).
         * @param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_quad(unsigned int position_attrib, unsigned int texcoord_attrib, int x, int y, int w, int h, int vpw, int vph, float depth);

        /**
         * Draw a full screen quad using the bound shader. It binds the vertex position and UVs arrays to the given
         * attribute array indices and draws the quad.
         * @param position_attrib The vertex attribute array index that represents position in the current shader.
         * @param texcoord_attrib The vertex attribute array index that represents 2D UVs in the current shader.
         * @param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_full_screen_quad(unsigned int position_attrib, unsigned int texcoord_attrib, float depth);


		//------  The following functions prepare data (points, normals, and colors) for rendering -----

		/**
		 * @brief Prepare data for a representing a grid as a set of line segments. The grid is centered at
		 *        vec3(0, 0, 0) and lies on the XOY plane.
		 *        and is
		 * @param x_steps The number of subdivisions along X direction.
		 * @param y_steps The number of subdivisions along Y direction.
		 * @param scale The scaling factor
		 * @param points The points to be returned.
		 */
        void prepare_grid(int x_steps, int y_steps, std::vector<vec3>& points, float scale = 0.5f);

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

		/**
		 * @brief Prepare data for representing a torus.
		 * @param major_radius The radius of major circle.
		 * @param minor_radius The radius of minor circle.
		 * @param major_slices The number of subdivisions along the major circle (suggested value 50).
		 * @param minor_slices The number of subdivisions along the minor circle (suggested value 20).
		 * @param points Returns the points.
		 * @param normals Returns the normals.
         */
        void prepare_torus(double major_radius, double minor_radius, int major_slices, int minor_slices,
                std::vector<vec3>& points, std::vector<vec3>& normals
        );

        /**
         * \brief Prepare data (points) for representing a camera in the 3D world as a set of lines.
         * \param size The size of the camera. A good value can be, e.g., 5% of the scene raidus.
         * \param hw_ratio The aspect ratio of the base quad defined as height/width (default 0.6).
         */
        void prepare_camera(std::vector<vec3>& points, float size, float hw_ratio = 0.6f);
    }

}


#endif
