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


#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/opengl.h>


namespace easy3d {

	namespace opengl {

		void draw_quad(GLuint positionAttrib, int x, int y, int w, int h, int vpw, int vph, float depth)
		{
			// vertex positions in NDC (Normalized Device Coordinates)
			// I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to
			// convert from screen coordinates to NDC.
			//Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;
			//Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
			const float minX = 2.0f * x / vpw - 1.0f;
			const float minY = 2.0f * y / vph - 1.0f;
			const float maxX = 2.0f * (x + w) / vpw - 1.0f;
			const float maxY = 2.0f * (y + h) / vph - 1.0f;

			const float positions[] = {
				minX, minY, depth,
				maxX, minY, depth,
				minX, maxY, depth,
				maxX, maxY, depth
			};

			glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
			glEnableVertexAttribArray(positionAttrib);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisableVertexAttribArray(positionAttrib);
		}


		void draw_quad_wire(GLuint positionAttrib, int x, int y, int w, int h, int vpw, int vph, float depth)
		{
			static GLuint vao_handle = 0;
			static int last_x = x;
			static int last_y = y;
			static int last_w = w;
			static int last_h = h;
			static int last_vpw = vpw;
			static int last_vph = vph;
			static float last_depth = depth;
			if (vao_handle == 0 || depth != last_depth || last_x != x || last_y != y || last_w != w || last_h != h || last_vpw != vpw || last_vph != vph) {
				// vertex positions in NDC (Normalized Device Coordinates)
				// I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to
				// convert from screen coordinates to NDC.
				//Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;
				//Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
				const float minX = 2.0f * x / vpw - 1.0f;
				const float minY = 2.0f * y / vph - 1.0f;
				const float maxX = 2.0f * (x + w) / vpw - 1.0f;
				const float maxY = 2.0f * (y + h) / vph - 1.0f;

				const float positions[] = {
					minX, minY, depth,
					maxX, minY, depth,
					maxX, maxY, depth,
					minX, maxY, depth
				};
				glGenVertexArrays(1, &vao_handle);			easy3d_debug_gl_error;
				glBindVertexArray(vao_handle);			easy3d_debug_gl_error;

				GLuint vbo_positions = 0;
				glGenBuffers(1, &vbo_positions);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, positions, GL_STATIC_DRAW);
				glEnableVertexAttribArray(positionAttrib);
				glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, 0);			easy3d_debug_gl_error;
				glBindVertexArray(0);			easy3d_debug_gl_error;

				last_x = x;
				last_y = y;
				last_w = w;
				last_h = h;
				last_vpw = vpw;
				last_vph = vph;
				last_depth = depth;
			}

			glBindVertexArray(vao_handle);			easy3d_debug_gl_error;
			glDrawArrays(GL_LINE_LOOP, 0, 4);		easy3d_debug_gl_error;
			glBindVertexArray(0);
		}


		void draw_quad(GLuint positionAttrib, GLuint texcoordAttrib, int x, int y, int w, int h, int vpw, int vph, float depth)
		{
			static GLuint vao_handle = 0;
			static int last_x = x;
			static int last_y = y;
			static int last_w = w;
			static int last_h = h;
			static int last_vpw = vpw;
			static int last_vph = vph;
			static float last_depth = depth;
			if (vao_handle == 0 || depth != last_depth || last_x != x || last_y != y || last_w != w || last_h != h || last_vpw != vpw || last_vph != vph) {
				// vertex positions in NDC (Normalized Device Coordinates)
				// I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to
				// convert from screen coordinates to NDC.
				//Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;
				//Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
				const float minX = 2.0f * x / vpw - 1.0f;
				const float minY = 2.0f * y / vph - 1.0f;
				const float maxX = 2.0f * (x + w) / vpw - 1.0f;
				const float maxY = 2.0f * (y + h) / vph - 1.0f;

				const float maxYTexCoord = (h == 0) ? 0.0f : 1.0f;

				const float positions[] = {
					minX, minY, depth,
					maxX, minY, depth,
					minX, maxY, depth,
					maxX, maxY, depth
				};
				const float texcoords[] = {
					 0, 0,
					 1.0, 0,
					 0, maxYTexCoord,
					 1.0, maxYTexCoord
				};

				glGenVertexArrays(1, &vao_handle);
				glBindVertexArray(vao_handle);

				GLuint vbo_positions = 0;
				glGenBuffers(1, &vbo_positions);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, positions, GL_STATIC_DRAW);
				glEnableVertexAttribArray(positionAttrib);
				glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

				GLuint vbo_texcoords = 0;
				glGenBuffers(1, &vbo_texcoords);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, texcoords, GL_STATIC_DRAW);
				glEnableVertexAttribArray(texcoordAttrib);
				glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				last_x = x;
				last_y = y;
				last_w = w;
				last_h = h;
				last_vpw = vpw;
				last_vph = vph;
				last_depth = depth;
			}
			glBindVertexArray(vao_handle);				easy3d_debug_gl_error;
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		easy3d_debug_gl_error;
			glBindVertexArray(0);
		}


		void draw_full_screen_quad(GLuint positionAttrib, float depth)
		{
			// vertex positions in NDC (Normalized Device Coordinates)
			static const float positions[] = {
				-1.0f, -1.0f, depth,
				1.0f, -1.0f, depth,
				-1.0f, 1.0f, depth,
				1.0f, 1.0f, depth
			};

			glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
			glEnableVertexAttribArray(positionAttrib);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisableVertexAttribArray(positionAttrib);
		}


		void draw_full_screen_quad(GLuint positionAttrib, GLuint texcoordAttrib, float depth)
		{
			static GLuint vao_handle = 0;
			static float last_depth = depth;
			if (vao_handle == 0 || depth != last_depth) {
				// vertex positions in NDC (Normalized Device Coordinates)
				const float positions[] = {
					-1.0f, -1.0f, depth,
					1.0f, -1.0f, depth,
					-1.0f, 1.0f, depth,
					1.0f, 1.0f, depth
				};

				// texture coordinates
				const float texcoords[] = {
					0.0f, 0.0f,
					1.0f, 0.0f,
					0.0f, 1.0f,
					1.0f, 1.0f
				};

				glGenVertexArrays(1, &vao_handle);
				glBindVertexArray(vao_handle);

				GLuint vbo_positions = 0;
				glGenBuffers(1, &vbo_positions);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, positions, GL_STATIC_DRAW);
				glEnableVertexAttribArray(positionAttrib);
				glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
				
				GLuint vbo_texcoords = 0;
				glGenBuffers(1, &vbo_texcoords);		
				glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, texcoords, GL_STATIC_DRAW);
				glEnableVertexAttribArray(texcoordAttrib);
				glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				last_depth = depth;
			}

			glBindVertexArray(vao_handle);				easy3d_debug_gl_error;
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		easy3d_debug_gl_error;
			glBindVertexArray(0);
		}


		void draw_cube(GLuint positionAttrib)
		{
			const float positions[] = {
				-1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,
				-1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				-1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				-1.0f, 1.0f, -1.0f,
				1.0f, 1.0f, -1.0f,
			};

			const GLushort indices[] = {
				0, 1, 2, 2, 1, 3,   // f
				4, 6, 5, 5, 6, 7,   // b
				4, 0, 6, 6, 0, 2,   // l
				1, 5, 3, 3, 5, 7,   // r
				2, 3, 6, 6, 3, 7,   // t
				4, 5, 0, 0, 5, 1,   // b
			};

			glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
			glEnableVertexAttribArray(positionAttrib);

			glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLushort), GL_UNSIGNED_SHORT, indices);

			glDisableVertexAttribArray(positionAttrib);

			//checkGlError("drawCube");
		}

		void draw_cube_wire(GLuint positionAttrib)
		{
			const float positions[] = {
				-1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, 1.0f,
				-1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f, 1.0f, -1.0f,
				-1.0f, 1.0f, -1.0f,
			};

			const GLushort indices[] = {
				0, 1, 1, 2, 2, 3, 3, 0,    // f
				4, 5, 5, 6, 6, 7, 7, 4, // b
				0, 4, 1, 5, 2, 6, 3, 7
			};

			glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), positions);
			glEnableVertexAttribArray(positionAttrib);

			glDrawElements(GL_LINES, sizeof(indices) / sizeof(GLushort), GL_UNSIGNED_SHORT, indices);

			glDisableVertexAttribArray(positionAttrib);
		}

		void draw_point(GLuint positionAttrib, const vec3 &pos)
		{
			glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, false, 3 * sizeof(float), pos);
			glEnableVertexAttribArray(positionAttrib);

			glDrawArrays(GL_POINTS, 0, 1);

			glDisableVertexAttribArray(positionAttrib);
		}



		// ---------------------------------------------------------------------------

		void prepare_sphere(
			const vec3& center, double radius, int slices, int stacks, const vec3& color,
			std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors)
		{
			prepare_checker_sphere(center, radius, slices, stacks, 1, color, color, points, normals, colors);
		}


		void prepare_checker_sphere(
			const vec3& center, double radius, int slices, int stacks, int checker_size,
			const vec3& color1, const vec3& color2,
			std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors)
		{
			for (int u = 0; u < slices; u += 1) {
				double theta1 = u * 2.0 * M_PI / slices;
				double theta2 = (u + 1) * 2.0 * M_PI / slices;
				for (int v = 0; v < stacks; v++) {
					double phi1 = v * M_PI / stacks - M_PI / 2.0;
					double phi2 = (v + 1) * M_PI / stacks - M_PI / 2.0;

					double x11 = radius * std::cos(theta1) * std::cos(phi1);
					double y11 = radius * std::sin(theta1) * std::cos(phi1);
					double z11 = radius * std::sin(phi1);

					double x12 = radius * std::cos(theta1) * std::cos(phi2);
					double y12 = radius * std::sin(theta1) * std::cos(phi2);
					double z12 = radius * std::sin(phi2);

					double x21 = radius * std::cos(theta2) * std::cos(phi1);
					double y21 = radius * std::sin(theta2) * std::cos(phi1);
					double z21 = radius * std::sin(phi1);

					double x22 = radius * std::cos(theta2) * std::cos(phi2);
					double y22 = radius * std::sin(theta2) * std::cos(phi2);
					double z22 = radius * std::sin(phi2);

					// we generate triangle fans, but the renderer accept triangles only.
					// so we collect vertices first, then we split the fan to triangles.
					std::vector<vec3> fan;
					fan.push_back(vec3(static_cast<float>(x11), static_cast<float>(y11), static_cast<float>(z11)));
					if (v != 0)
						fan.push_back(vec3(static_cast<float>(x21), static_cast<float>(y21), static_cast<float>(z21)));
					fan.push_back(vec3(static_cast<float>(x22), static_cast<float>(y22), static_cast<float>(z22)));
					if (v != stacks - 1)
						fan.push_back(vec3(static_cast<float>(x12), static_cast<float>(y12), static_cast<float>(z12)));

					int toggle = ((u / checker_size) ^ (v / checker_size)) & 1;
					vec3 color = toggle ? color1 : color2;
					if (fan.size() == 4) { // a quad
						points.push_back(fan[0] + center);   normals.push_back(normalize(fan[0]));
						points.push_back(fan[1] + center);   normals.push_back(normalize(fan[1]));
						points.push_back(fan[2] + center);   normals.push_back(normalize(fan[2]));
						points.push_back(fan[0] + center);   normals.push_back(normalize(fan[0]));
						points.push_back(fan[2] + center);   normals.push_back(normalize(fan[2]));
						points.push_back(fan[3] + center);   normals.push_back(normalize(fan[3]));
						colors.insert(colors.end(), 6, color);
					}
					else { // a triangle
						for (const auto& p : fan) {
							points.push_back(p + center);
							normals.push_back(normalize(p));
							colors.push_back(color);
						}
					}
				}
			}
		}


		void prepare_cylinder( double radius, int slices, const vec3& s, const vec3& t, const vec3& color, std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors )
		{
			// find a vector perpendicular to the direction
			const vec3 offset = t - s;
			const vec3 axis = normalize(offset);
			vec3 perp = geom::orthogonal(axis);
			perp.normalize();
			const vec3 p = s + perp * radius;
			const double angle_interval = 2.0 * M_PI / slices;
			for (int i = 0; i < slices; ++i) {
				double angle_a = i * angle_interval;
				double angle_b = (i + 1) * angle_interval;
                // the rotation axis is just the direction (i.e., passing through the original)
                const vec3 a = s + mat4::rotation(axis, static_cast<float>(angle_a)) * (p - s);
                const vec3 b = s + mat4::rotation(axis, static_cast<float>(angle_b)) * (p - s);
				const vec3 c = a + offset;
				const vec3 d = b + offset;
				const vec3 na = normalize(a - s);
				const vec3 nb = normalize(b - s);
				const vec3 nc = normalize(c - t);
				const vec3 nd = normalize(d - t);
				// triangle 1
				points.push_back(a);	normals.push_back(na);	colors.push_back(color);
				points.push_back(b);	normals.push_back(nb);	colors.push_back(color);
				points.push_back(c);	normals.push_back(nc);	colors.push_back(color);
				// triangle 2
				points.push_back(b);	normals.push_back(nb);	colors.push_back(color);
				points.push_back(d);	normals.push_back(nd);	colors.push_back(color);
				points.push_back(c);	normals.push_back(nc);	colors.push_back(color);
			}
		}


		void prepare_cone(double radius, int slices, const vec3& s, const vec3& t, const vec3& color, std::vector<vec3>& points, std::vector<vec3>& normals, std::vector<vec3>& colors)
		{
			// find a vector perpendicular to the direction
			const vec3 offset = t - s;
			const vec3 axis = normalize(offset);
			vec3 perp = geom::orthogonal(axis);
			perp.normalize();
			const vec3 p = s + perp * radius;
			const double angle_interval = 2.0 * M_PI / slices;
			for (int i = 0; i < slices; ++i) {
				double angle_a = i * angle_interval;
				double angle_b = (i + 1) * angle_interval;
                // the rotation axis is just the direction (i.e., passing through the original)
                const vec3 a = s + mat4::rotation(axis, static_cast<float>(angle_a)) * (p - s);
                const vec3 b = s + mat4::rotation(axis, static_cast<float>(angle_b)) * (p - s);
				const vec3 c = t;

				// na
				vec3 dir = a - t;	dir.normalize();
				vec3 na = t + dir * dot(s - t, dir) - s;
				na.normalize();

				// nb
				dir = b - t;	dir.normalize();
				vec3 nb = t + dir * dot(s - t, dir) - s;
				nb.normalize();

				// nc
				vec3 nc = cross(a - t, b - t); 
				nc.normalize();

				points.push_back(a);	normals.push_back(na);	colors.push_back(color);
				points.push_back(b);	normals.push_back(nb);	colors.push_back(color);
				points.push_back(c);	normals.push_back(nc);	colors.push_back(color);
			}
		}


	}

} // namespace easy3d
