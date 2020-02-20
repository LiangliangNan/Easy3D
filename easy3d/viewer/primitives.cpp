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

#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/opengl.h>


namespace easy3d {

    namespace opengl {


        void draw_quad_wire(const Rect &rect, const vec4 &color, int width, int height, float depth) {
            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program) {
                LOG_FIRST_N(ERROR, 1) << "shader doesn't exist: " << name << " (this is the first record)";
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = height - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / width - 1.0f;
            const float min_y = 2.0f * y0 / height - 1.0f;
            const float max_x = 2.0f * (x0 + w) / width - 1.0f;
            const float max_y = 2.0f * (y0 + h) / height - 1.0f;

            const std::vector<vec3> points = {
                    vec3(min_x, min_y, depth),
                    vec3(max_x, min_y, depth),
                    vec3(max_x, max_y, depth),
                    vec3(min_x, max_y, depth)
            };
            const std::vector<unsigned int> indices = {0, 1, 1, 2, 2, 3, 3, 0};

            LinesDrawable drawable;
            drawable.update_vertex_buffer(points);
            drawable.update_index_buffer(indices);

            program->bind();
            program->set_uniform("screen_color", color);
            drawable.gl_draw(false);
            program->release();
        }


        void draw_quad_filled(const Rect &rect, const vec4 &color, int width, int height, float depth) {
            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program) {
                LOG_FIRST_N(ERROR, 1) << "shader doesn't exist: " << name << " (this is the first record)";
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = height - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / width - 1.0f;
            const float min_y = 2.0f * y0 / height - 1.0f;
            const float max_x = 2.0f * (x0 + w) / width - 1.0f;
            const float max_y = 2.0f * (y0 + h) / height - 1.0f;

            const std::vector<vec3> points = {
                    vec3(min_x, min_y, depth),
                    vec3(max_x, min_y, depth),
                    vec3(max_x, max_y, depth),
                    vec3(min_x, max_y, depth)
            };
            const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            TrianglesDrawable drawable;
            drawable.update_vertex_buffer(points);
            drawable.update_index_buffer(indices);

            program->bind();
            program->set_uniform("screen_color", color);
            drawable.gl_draw(false);
            program->release();
        }


        void draw_quad_filled(const Rect &rect, int texture, int width, int height, float depth) {
            const std::string name = "screen_space/screen_space_texture";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                        ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tex_coord")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program) {
                LOG_FIRST_N(ERROR, 1) << "shader doesn't exist: " << name << " (this is the first record)";
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = height - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / width - 1.0f;
            const float min_y = 2.0f * y0 / height - 1.0f;
            const float max_x = 2.0f * (x0 + w) / width - 1.0f;
            const float max_y = 2.0f * (y0 + h) / height - 1.0f;

            const std::vector<vec3> points = {
                    vec3(min_x, min_y, depth),
                    vec3(max_x, min_y, depth),
                    vec3(max_x, max_y, depth),
                    vec3(min_x, max_y, depth)
            };

            const std::vector<vec2> texcoords = {
                    vec2(0, 0),
                    vec2(1.0, 0),
                    vec2(1.0, 1.0),
                    vec2(0, 1.0),
            };

            const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            TrianglesDrawable drawable;
            drawable.update_vertex_buffer(points);
            drawable.update_texcoord_buffer(texcoords);
            drawable.update_index_buffer(indices);

            program->bind();
            program->bind_texture("textureID", texture, 0);
            drawable.gl_draw(false);
            program->release_texture();
            program->release();
        }


        void draw_depth_texture(const Rect &rect, int texture, int width, int height, float depth) {
            const std::string name = "screen_space/screen_space_depth_texture";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                        ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tex_coord")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program) {
                LOG_FIRST_N(ERROR, 1) << "shader doesn't exist: " << name << " (this is the first record)";
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = height - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / width - 1.0f;
            const float min_y = 2.0f * y0 / height - 1.0f;
            const float max_x = 2.0f * (x0 + w) / width - 1.0f;
            const float max_y = 2.0f * (y0 + h) / height - 1.0f;

            const std::vector<vec3> points = {
                    vec3(min_x, min_y, depth),
                    vec3(max_x, min_y, depth),
                    vec3(max_x, max_y, depth),
                    vec3(min_x, max_y, depth)
            };

            const std::vector<vec2> texcoords = {
                    vec2(0, 0),
                    vec2(1.0, 0),
                    vec2(1.0, 1.0),
                    vec2(0, 1.0),
            };

            const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            TrianglesDrawable drawable;
            drawable.update_vertex_buffer(points);
            drawable.update_texcoord_buffer(texcoords);
            drawable.update_index_buffer(indices);

            program->bind();
            program->bind_texture("textureID", texture, 0);
            drawable.gl_draw(false);
            program->release_texture();
            program->release();
        }


        void draw_quad(GLuint positionAttrib, GLuint texcoordAttrib, int x, int y, int w, int h, int vpw, int vph, float depth) {
            static GLuint vao_handle = 0;
            static int last_x = x;
            static int last_y = y;
            static int last_w = w;
            static int last_h = h;
            static int last_vpw = vpw;
            static int last_vph = vph;
            static float last_depth = depth;
            if (vao_handle == 0 || depth != last_depth || last_x != x || last_y != y || last_w != w || last_h != h ||
                last_vpw != vpw || last_vph != vph) {
                // vertex positions in NDC (Normalized Device Coordinates)
                // I assume viewportX = 0 and viewportY = 0.  Otherwise use the following equation to
                // convert from screen coordinates to NDC.
                //Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;
                //Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
                const float min_x = 2.0f * x / vpw - 1.0f;
                const float min_y = 2.0f * y / vph - 1.0f;
                const float max_x = 2.0f * (x + w) / vpw - 1.0f;
                const float max_y = 2.0f * (y + h) / vph - 1.0f;

                const float max_yTexCoord = (h == 0) ? 0.0f : 1.0f;

                const float positions[] = {
                        min_x, min_y, depth,
                        max_x, min_y, depth,
                        min_x, max_y, depth,
                        max_x, max_y, depth
                };
                const float texcoords[] = {
                        0, 0,
                        1.0, 0,
                        0, max_yTexCoord,
                        1.0, max_yTexCoord
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
            glBindVertexArray(vao_handle);
            easy3d_debug_log_gl_error;
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            easy3d_debug_log_gl_error;
            glBindVertexArray(0);
        }


        void draw_full_screen_quad(GLuint positionAttrib, float depth) {
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


        void draw_full_screen_quad(GLuint positionAttrib, GLuint texcoordAttrib, float depth) {
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

            glBindVertexArray(vao_handle);
            easy3d_debug_log_gl_error;
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            easy3d_debug_log_gl_error;
            glBindVertexArray(0);
        }


        // ---------------------------------------------------------------------------

        void prepare_grid(int x_steps, int y_steps, std::vector<vec3> &points, float scale) {
            // Horizontal lines
            float x = scale * 0.5f * (x_steps - 1);
            float y = -scale * 0.5f * (y_steps - 1);
            for (int i = 0; i < y_steps; i++) {
                points.push_back(vec3(-x, y, 0.0f));
                points.push_back(vec3(x, y, 0.0f));
                y += scale;
            }

            // Vertical lines
            x = -scale * 0.5f * (float) (x_steps - 1);
            y = scale * 0.5f * (float) (y_steps - 1);
            for (int i = 0; i < x_steps; i++) {
                points.push_back(vec3(x, -y, 0.0f));
                points.push_back(vec3(x, y, 0.0f));
                x += scale;
            }
        }

        void prepare_sphere(
                const vec3 &center, double radius, int slices, int stacks, const vec3 &color,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors) {
            prepare_checker_sphere(center, radius, slices, stacks, 1, color, color, points, normals, colors);
        }


        void prepare_checker_sphere(
                const vec3 &center, double radius, int slices, int stacks, int checker_size,
                const vec3 &color1, const vec3 &color2,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors) {
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
                        points.push_back(fan[0] + center);
                        normals.push_back(normalize(fan[0]));
                        points.push_back(fan[1] + center);
                        normals.push_back(normalize(fan[1]));
                        points.push_back(fan[2] + center);
                        normals.push_back(normalize(fan[2]));
                        points.push_back(fan[0] + center);
                        normals.push_back(normalize(fan[0]));
                        points.push_back(fan[2] + center);
                        normals.push_back(normalize(fan[2]));
                        points.push_back(fan[3] + center);
                        normals.push_back(normalize(fan[3]));
                        colors.insert(colors.end(), 6, color);
                    } else { // a triangle
                        for (const auto &p : fan) {
                            points.push_back(p + center);
                            normals.push_back(normalize(p));
                            colors.push_back(color);
                        }
                    }
                }
            }
        }


        void prepare_cylinder(double radius, int slices, const vec3 &s, const vec3 &t, const vec3 &color,
                              std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors) {
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
                points.push_back(a);
                normals.push_back(na);
                colors.push_back(color);
                points.push_back(b);
                normals.push_back(nb);
                colors.push_back(color);
                points.push_back(c);
                normals.push_back(nc);
                colors.push_back(color);
                // triangle 2
                points.push_back(b);
                normals.push_back(nb);
                colors.push_back(color);
                points.push_back(d);
                normals.push_back(nd);
                colors.push_back(color);
                points.push_back(c);
                normals.push_back(nc);
                colors.push_back(color);
            }
        }


        void prepare_cone(double radius, int slices, const vec3 &s, const vec3 &t, const vec3 &color,
                          std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors) {
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
                vec3 dir = a - t;
                dir.normalize();
                vec3 na = t + dir * dot(s - t, dir) - s;
                na.normalize();

                // nb
                dir = b - t;
                dir.normalize();
                vec3 nb = t + dir * dot(s - t, dir) - s;
                nb.normalize();

                // nc
                vec3 nc = cross(a - t, b - t);
                nc.normalize();

                points.push_back(a);
                normals.push_back(na);
                colors.push_back(color);
                points.push_back(b);
                normals.push_back(nb);
                colors.push_back(color);
                points.push_back(c);
                normals.push_back(nc);
                colors.push_back(color);
            }
        }


        void prepare_torus(double major_radius, double minor_radius, int major_slices, int minor_slices,
                           std::vector<vec3> &points, std::vector<vec3> &normals) {
            bool flip = false;
            const double twopi = 2.0 * M_PI;
            for (int i = 0; i < minor_slices; i++) {
                // QUAD STRIPs;
                std::vector<vec3> tmp_pts, tmp_nms;
                for (int j = 0; j <= major_slices; j++) {
                    for (int k = 1; k >= 0; k--) {
                        double s = (i + k) % minor_slices + 0.5;
                        double t = j % major_slices;

                        // Calculate point on surface
                        double x = (major_radius + minor_radius * cos(s * twopi / minor_slices)) *
                                   cos(t * twopi / major_slices);
                        double y = minor_radius * sin(s * twopi / minor_slices);
                        double z = (major_radius + minor_radius * cos(s * twopi / minor_slices)) *
                                   sin(t * twopi / major_slices);

                        // Calculate surface normal
                        double nx = x - major_radius * cos(t * twopi / major_slices);
                        double ny = y;
                        double nz = z - major_radius * sin(t * twopi / major_slices);
                        vec3 n(nx, ny, nz);
                        n.normalize();

                        tmp_pts.push_back(vec3(x, y, z));
                        tmp_nms.push_back(n);

                        if (tmp_pts.size() <= 3) {
                            points.push_back(vec3(x, y, z));
                            normals.push_back(n);
                        } else {
                            points.push_back(vec3(x, y, z));
                            normals.push_back(n);
                            if (flip) {
                                points.push_back(tmp_pts[tmp_pts.size() - 3]);
                                normals.push_back(tmp_nms[tmp_nms.size() - 3]);
                                points.push_back(tmp_pts[tmp_pts.size() - 2]);
                                normals.push_back(tmp_nms[tmp_nms.size() - 2]);
                            } else {
                                points.push_back(tmp_pts[tmp_pts.size() - 2]);
                                normals.push_back(tmp_nms[tmp_nms.size() - 2]);
                                points.push_back(tmp_pts[tmp_pts.size() - 3]);
                                normals.push_back(tmp_nms[tmp_nms.size() - 3]);
                            }
                            flip = !flip;
                        }
                    }
                }
                flip = !flip;
            }
        }


        void prepare_camera(std::vector<vec3> &points, float size, float hw_ratio) {
            const float halfWidth = size;
            const float halfHeight = halfWidth * hw_ratio;
            const float dist = halfHeight / tan(float(M_PI) / 8.0f);

            const float arrowHeight = 2.0f * halfHeight;
            const float baseHeight = 1.2f * halfHeight;
            const float arrowHalfWidth = 0.5f * halfWidth;
            const float baseHalfWidth = 0.3f * halfWidth;

            //--------------
            // Frustum outline
            //--------------

            // LINE_STRIP
            const vec3 p0(-halfWidth, halfHeight, -dist);
            const vec3 p1(-halfWidth, -halfHeight, -dist);
            points.push_back(p0);
            points.push_back(p1);
            const vec3 p2(0.0f, 0.0f, 0.0f);
            points.push_back(p1);
            points.push_back(p2);
            const vec3 p3(halfWidth, -halfHeight, -dist);
            points.push_back(p2);
            points.push_back(p3);
            const vec3 p4(-halfWidth, -halfHeight, -dist);
            points.push_back(p3);
            points.push_back(p4);

            // LINE_STRIP
            const vec3 q0(halfWidth, -halfHeight, -dist);
            const vec3 q1(halfWidth, halfHeight, -dist);
            const vec3 q2(0.0f, 0.0f, 0.0f);
            const vec3 q3(-halfWidth, halfHeight, -dist);
            const vec3 q4(halfWidth, halfHeight, -dist);
            points.push_back(q0);
            points.push_back(q1);
            points.push_back(q1);
            points.push_back(q2);
            points.push_back(q2);
            points.push_back(q3);
            points.push_back(q3);
            points.push_back(q4);

            //------------------
            // Up arrow
            //------------------

            // Base - QUAD
            const vec3 r0(-baseHalfWidth, halfHeight, -dist);
            const vec3 r1(baseHalfWidth, halfHeight, -dist);
            const vec3 r2(baseHalfWidth, baseHeight, -dist);
            const vec3 r3(-baseHalfWidth, baseHeight, -dist);
            points.push_back(r0);
            points.push_back(r1);
            points.push_back(r1);
            points.push_back(r2);
            points.push_back(r2);
            points.push_back(r3);
            points.push_back(r3);
            points.push_back(r0);

            // Arrow - TRIANGLE
            const vec3 a0(0.0f, arrowHeight, -dist);
            const vec3 a1(-arrowHalfWidth, baseHeight, -dist);
            const vec3 a2(arrowHalfWidth, baseHeight, -dist);
            points.push_back(a0);
            points.push_back(a1);
            points.push_back(a1);
            points.push_back(a2);
            points.push_back(a2);
            points.push_back(a0);
        }


    }

} // namespace easy3d
