/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/setting.h>


namespace easy3d {

    namespace shape {

        void draw_quad_wire(const Rect &rect, const vec4 &color, int width, int height, float depth) {
            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "ndc_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program) {
                LOG_N_TIMES(3, ERROR) << "shader doesn't exist: " << name << ". " << COUNTER;
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = static_cast<float>(height) - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / static_cast<float>(width) - 1.0f;
            const float min_y = 2.0f * y0 / static_cast<float>(height) - 1.0f;
            const float max_x = 2.0f * (x0 + w) / static_cast<float>(width) - 1.0f;
            const float max_y = 2.0f * (y0 + h) / static_cast<float>(height) - 1.0f;

            const std::vector<vec2> points = {
                    vec2(min_x, min_y),
                    vec2(max_x, min_y),
                    vec2(max_x, max_y),
                    vec2(min_x, max_y)
            };

            unsigned int vertex_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true); easy3d_debug_log_gl_error

            program->bind();    easy3d_debug_log_gl_error
            program->set_uniform("screen_color", color); easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            vao.bind(); easy3d_debug_log_gl_error
            glDrawArrays(GL_LINE_LOOP, 0, static_cast<int>(points.size()));     easy3d_debug_log_gl_error
            vao.release();  easy3d_debug_log_gl_error
            program->release();     easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
        }


        void draw_quad_filled(const Rect &rect, const vec4 &color, int width, int height, float depth) {
            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "ndc_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program) {
                LOG_N_TIMES(3, ERROR) << "shader doesn't exist: " << name << ". " << COUNTER;
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = static_cast<float>(height) - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / static_cast<float>(width) - 1.0f;
            const float min_y = 2.0f * y0 / static_cast<float>(height) - 1.0f;
            const float max_x = 2.0f * (x0 + w) / static_cast<float>(width) - 1.0f;
            const float max_y = 2.0f * (y0 + h) / static_cast<float>(height) - 1.0f;

            const std::vector<vec2> points = {
                    vec2(min_x, min_y),
                    vec2(max_x, min_y),
                    vec2(max_x, max_y),
                    vec2(min_x, max_y)
            };
            const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            unsigned int vertex_buffer = 0, element_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true);    easy3d_debug_log_gl_error
            vao.create_element_buffer(element_buffer, indices.data(), indices.size() * sizeof(unsigned int),
                                      true);    easy3d_debug_log_gl_error

            program->bind();      easy3d_debug_log_gl_error
            program->set_uniform("screen_color", color); easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            vao.bind(); easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);    easy3d_debug_log_gl_error
            glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);     easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    easy3d_debug_log_gl_error
            vao.release();   easy3d_debug_log_gl_error
            program->release();     easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
            VertexArrayObject::release_buffer(element_buffer);
        }


        void draw_quad_filled(const Rect &rect, unsigned int texture, int width, int height, float depth) {
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
                LOG_N_TIMES(3, ERROR) << "shader doesn't exist: " << name << ". " << COUNTER;
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = static_cast<float>(height) - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / static_cast<float>(width) - 1.0f;
            const float min_y = 2.0f * y0 / static_cast<float>(height) - 1.0f;
            const float max_x = 2.0f * (x0 + w) / static_cast<float>(width) - 1.0f;
            const float max_y = 2.0f * (y0 + h) / static_cast<float>(height) - 1.0f;

            const std::vector<vec2> points = {
                    vec2(min_x, min_y),
                    vec2(max_x, min_y),
                    vec2(max_x, max_y),
                    vec2(min_x, max_y)
            };

            static const std::vector<vec2> texcoords = {
                    vec2(0, 0),
                    vec2(1.0, 0),
                    vec2(1.0, 1.0),
                    vec2(0, 1.0),
            };

            static const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            unsigned int vertex_buffer = 0, texcoord_buffer = 0, element_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true);    easy3d_debug_log_gl_error
            vao.create_array_buffer(texcoord_buffer, ShaderProgram::TEXCOORD, texcoords.data(),
                                    texcoords.size() * sizeof(vec2), 2, true);    easy3d_debug_log_gl_error
            vao.create_element_buffer(element_buffer, indices.data(), indices.size() * sizeof(unsigned int),
                                      true);    easy3d_debug_log_gl_error

            program->bind();      easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            program->bind_texture("textureID", texture, 0);
            vao.bind(); easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);    easy3d_debug_log_gl_error
            glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);     easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    easy3d_debug_log_gl_error
            program->release_texture();
            vao.release();   easy3d_debug_log_gl_error
            program->release();     easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
            VertexArrayObject::release_buffer(texcoord_buffer);
            VertexArrayObject::release_buffer(element_buffer);
        }


        void draw_full_screen_quad(unsigned int texture, float depth) {
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
                LOG_N_TIMES(3, ERROR) << "shader doesn't exist: " << name << ". " << COUNTER;
                return;
            }

            // vertex positions in NDC (Normalized Device Coordinates)
            static const std::vector<vec2> points = {
                    vec2(-1.0f, -1.0f),
                    vec2(1.0f, -1.0f),
                    vec2(1.0f, 1.0f),
                    vec2(-1.0f, 1.0f)
            };

            static const std::vector<vec2> texcoords = {
                    vec2(0, 0),
                    vec2(1.0, 0),
                    vec2(1.0, 1.0),
                    vec2(0, 1.0),
            };

            static const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            unsigned int vertex_buffer = 0, texcoord_buffer = 0, element_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true);    easy3d_debug_log_gl_error
            vao.create_array_buffer(texcoord_buffer, ShaderProgram::TEXCOORD, texcoords.data(),
                                    texcoords.size() * sizeof(vec2), 2, true);    easy3d_debug_log_gl_error
            vao.create_element_buffer(element_buffer, indices.data(), indices.size() * sizeof(unsigned int),
                                      true);    easy3d_debug_log_gl_error

            program->bind();      easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            program->bind_texture("textureID", texture, 0);
            vao.bind(); easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);    easy3d_debug_log_gl_error
            glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);     easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    easy3d_debug_log_gl_error
            program->release_texture();
            vao.release();   easy3d_debug_log_gl_error
            program->release();     easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
            VertexArrayObject::release_buffer(texcoord_buffer);
            VertexArrayObject::release_buffer(element_buffer);
        }


        void draw_depth_texture(const Rect &rect, unsigned int texture, int width, int height, float depth) {
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
                LOG_N_TIMES(3, ERROR) << "shader doesn't exist: " << name << ". " << COUNTER;
                return;
            }

            const float x0 = rect.x_min();
            const float y0 = static_cast<float>(height) - rect.y_max() - 1;
            const float w = rect.width();
            const float h = rect.height();

            const float min_x = 2.0f * x0 / static_cast<float>(width) - 1.0f;
            const float min_y = 2.0f * y0 / static_cast<float>(height) - 1.0f;
            const float max_x = 2.0f * (x0 + w) / static_cast<float>(width) - 1.0f;
            const float max_y = 2.0f * (y0 + h) / static_cast<float>(height) - 1.0f;

            const std::vector<vec2> points = {
                    vec2(min_x, min_y),
                    vec2(max_x, min_y),
                    vec2(max_x, max_y),
                    vec2(min_x, max_y)
            };

            static const std::vector<vec2> texcoords = {
                    vec2(0, 0),
                    vec2(1.0, 0),
                    vec2(1.0, 1.0),
                    vec2(0, 1.0),
            };

            static const std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

            unsigned int vertex_buffer = 0, texcoord_buffer = 0, element_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true);    easy3d_debug_log_gl_error
            vao.create_array_buffer(texcoord_buffer, ShaderProgram::TEXCOORD, texcoords.data(),
                                    texcoords.size() * sizeof(vec2), 2, true);    easy3d_debug_log_gl_error
            vao.create_element_buffer(element_buffer, indices.data(), indices.size() * sizeof(unsigned int),
                                      true);    easy3d_debug_log_gl_error

            program->bind();      easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            program->bind_texture("textureID", texture, 0);
            vao.bind(); easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);    easy3d_debug_log_gl_error
            glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);     easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    easy3d_debug_log_gl_error
            program->release_texture();
            vao.release();   easy3d_debug_log_gl_error
            program->release();     easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
            VertexArrayObject::release_buffer(texcoord_buffer);
            VertexArrayObject::release_buffer(element_buffer);
        }


        void draw_quad(unsigned int positionAttrib, unsigned int texcoordAttrib, int x, int y, int w, int h,
                       int vpw, int vph, float depth)
       {
                // vertex positions in NDC (Normalized Device Coordinates)
                // I assume viewportX = 0 and viewportY = 0. Otherwise, use the following equation to
                // convert from screen coordinates to NDC.
                //Xndc = 2.0f * (x - viewportX) / vpw - 1.0f;
                //Yndc = 2.0f * (y - viewportY) / vph - 1.0f;
                const float min_x = 2.0f * static_cast<float>(x) / static_cast<float>(vpw) - 1.0f;
                const float min_y = 2.0f * static_cast<float>(y) / static_cast<float>(vph) - 1.0f;
                const float max_x = 2.0f * static_cast<float>(x + w) / static_cast<float>(vpw) - 1.0f;
                const float max_y = 2.0f * static_cast<float>(y + h) / static_cast<float>(vph) - 1.0f;

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

           // create vao and buffers, prepare data

            GLuint vao_handle = 0;
            glGenVertexArrays(1, &vao_handle);
            glBindVertexArray(vao_handle);

            GLuint vbo_positions = 0;
            glGenBuffers(1, &vbo_positions);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, positions, GL_STATIC_DRAW);
            glEnableVertexAttribArray(positionAttrib);
            glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            GLuint vbo_texcoords = 0;
            glGenBuffers(1, &vbo_texcoords);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, texcoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(texcoordAttrib);
            glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

           // draw

            glBindVertexArray(vao_handle);  easy3d_debug_log_gl_error
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  easy3d_debug_log_gl_error
            glBindVertexArray(0);

           glDeleteBuffers(1, &vbo_positions);     easy3d_debug_log_gl_error
           glDeleteBuffers(1, &vbo_texcoords);     easy3d_debug_log_gl_error
           glDeleteVertexArrays(1, &vao_handle);   easy3d_debug_log_gl_error
        }


        void draw_full_screen_quad(unsigned int positionAttrib, unsigned int texcoordAttrib, float depth) {
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

            // create vao and buffers, prepare data

            GLuint vao_handle = 0;
            glGenVertexArrays(1, &vao_handle);
            glBindVertexArray(vao_handle);

            GLuint vbo_positions = 0;
            glGenBuffers(1, &vbo_positions);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, positions, GL_STATIC_DRAW);
            glEnableVertexAttribArray(positionAttrib);
            glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            GLuint vbo_texcoords = 0;
            glGenBuffers(1, &vbo_texcoords);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, texcoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(texcoordAttrib);
            glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            // draw

            glBindVertexArray(vao_handle);  easy3d_debug_log_gl_error
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  easy3d_debug_log_gl_error
            glBindVertexArray(0);

            glDeleteBuffers(1, &vbo_positions);     easy3d_debug_log_gl_error
            glDeleteBuffers(1, &vbo_texcoords);     easy3d_debug_log_gl_error
            glDeleteVertexArrays(1, &vao_handle);   easy3d_debug_log_gl_error
        }


        void draw_polygon_wire(const Polygon2 &polygon, const vec4 &color, int width, int height, float depth) {
            if (polygon.size() < 3)
                return;

            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "ndc_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program)
                return;

            std::vector<vec2> points(polygon.size());
            for (std::size_t i = 0; i < polygon.size(); ++i) {
                const auto &p = polygon[i];
                // to use the screen space shaders, I need to convert the point coordinates into the NDC space.
                // also have to follow the OpenGL coordinates rule.
                points[i].x = {2.0f * p.x / static_cast<float>(width) - 1.0f};
                points[i].y = {2.0f * (static_cast<float>(height) - p.y - 1) / static_cast<float>(height) - 1.0f};
            }

            unsigned int vertex_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true); easy3d_debug_log_gl_error

            program->bind();    easy3d_debug_log_gl_error
            program->set_uniform("screen_color", color); easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            vao.bind(); easy3d_debug_log_gl_error
            glDrawArrays(GL_LINE_LOOP, 0, static_cast<int>(points.size()));     easy3d_debug_log_gl_error
            vao.release();  easy3d_debug_log_gl_error
            program->release();     easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
        }


        void draw_polygon_filled(const Polygon2 &polygon, const vec4 &color, int width, int height, float depth) {
            if (polygon.size() < 3)
                return;

            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "ndc_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program)
                return;

            // draw the face
            Tessellator tess;
            tess.begin_polygon(vec3(0, 0, 1));
            tess.begin_contour();
            for (std::size_t i = 0; i < polygon.size(); ++i) {
                const auto &p = polygon[i];
                // to use the screen space shaders, I need to convert the point coordinates into the NDC space.
                // also have to follow the OpenGL coordinates rule.
                float x = {2.0f * p.x / static_cast<float>(width) - 1.0f};
                float y = {2.0f * (static_cast<float>(height) - p.y - 1) / static_cast<float>(height) - 1.0f};
                tess.add_vertex(vec3(x, y, 0));
            }
            tess.end_contour();
            tess.end_polygon();
            const auto &vts = tess.vertices();
            std::vector<vec2> points(vts.size());
            for (std::size_t i = 0; i < vts.size(); ++i)
                points[i] = vec2(vts[i]->data());

            const auto &indices = tess.elements();
            std::vector<unsigned int> elements;
            for (const auto &array : indices)
                elements.insert(elements.end(), array.begin(), array.end());

            unsigned int vertex_buffer = 0, element_buffer = 0;
            VertexArrayObject vao; easy3d_debug_log_gl_error
            vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, points.data(), points.size() * sizeof(vec2),
                                    2, true);    easy3d_debug_log_gl_error
            vao.create_element_buffer(element_buffer, elements.data(), elements.size() * sizeof(unsigned int),
                                      true);easy3d_debug_log_gl_error

            program->bind();      easy3d_debug_log_gl_error
            program->set_uniform("screen_color", color); easy3d_debug_log_gl_error
            program->set_uniform("depth", depth);
            vao.bind(); easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);    easy3d_debug_log_gl_error
            glDrawElements(GL_TRIANGLES, static_cast<int>(elements.size()), GL_UNSIGNED_INT, nullptr); easy3d_debug_log_gl_error
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    easy3d_debug_log_gl_error
            vao.release();   easy3d_debug_log_gl_error
            program->release();easy3d_debug_log_gl_error

            VertexArrayObject::release_buffer(vertex_buffer);
            VertexArrayObject::release_buffer(element_buffer);
        }


        void draw_sphere_big_circles(LinesDrawable *drawable, const mat4 &mvp, const mat4 &m, bool axes) {
            if (!drawable)
                return;

            ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
                program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
            }
            if (!program)
                return;

            if (drawable->vertex_buffer() == 0) {
                std::vector<vec3> points, colors;
                std::vector<unsigned int> indices;

                std::vector<vec3> points_xoy;   // xoy
                create_circle(50, points_xoy, indices);

                for (auto &p : points_xoy) {
                    points.push_back(p);
                    colors.emplace_back(vec3(0, 0, 1));
                }
                colors.resize(points_xoy.size(), vec3(0, 0, 1));

                auto rot_x = mat4::rotation(vec3(1, 0, 0), static_cast<float>(M_PI * 0.5f));
                for (std::size_t i = 0; i < points_xoy.size(); ++i) {
                    points.push_back(rot_x * points_xoy[i]);
                    colors.emplace_back(vec3(0, 1, 0));
                    indices.push_back(static_cast<unsigned int>(points_xoy.size() + i));
                    indices.push_back(static_cast<unsigned int>(points_xoy.size() + (i + 1) % points_xoy.size()));
                }

                auto rot_y = mat4::rotation(vec3(0, 1, 0), static_cast<float>(M_PI * 0.5f));
                for (std::size_t i = 0; i < points_xoy.size(); ++i) {
                    points.push_back(rot_y * points_xoy[i]);
                    colors.emplace_back(vec3(1, 0, 0));
                    indices.push_back(static_cast<unsigned int>(points_xoy.size() * 2 + i));
                    indices.push_back(static_cast<unsigned int>(points_xoy.size() * 2 + (i + 1) % points_xoy.size()));
                }

                if (axes) {
                    // x axis
                    points.emplace_back(vec3(-1, 0, 0));
                    points.emplace_back(vec3(1, 0, 0));
                    colors.emplace_back(vec3(1, 0, 0));
                    colors.emplace_back(vec3(1, 0, 0));
                    indices.push_back(static_cast<unsigned int>(points.size() - 2));
                    indices.push_back(static_cast<unsigned int>(points.size() - 1));

                    // y axis
                    points.emplace_back(vec3(0, -1, 0));
                    points.emplace_back(vec3(0, 1, 0));
                    colors.emplace_back(vec3(0, 1, 0));
                    colors.emplace_back(vec3(0, 1, 0));
                    indices.push_back(static_cast<unsigned int>(points.size() - 2));
                    indices.push_back(static_cast<unsigned int>(points.size() - 1));

                    // z axis
                    points.emplace_back(vec3(0, 0, -1));
                    points.emplace_back(vec3(0, 0, 1));
                    colors.emplace_back(vec3(0, 0, 1));
                    colors.emplace_back(vec3(0, 0, 1));
                    indices.push_back(static_cast<unsigned int>(points.size() - 2));
                    indices.push_back(static_cast<unsigned int>(points.size() - 1));
                }

                drawable->update_vertex_buffer(points);
                drawable->update_color_buffer(colors);
                drawable->update_element_buffer(indices);
                drawable->set_property_coloring(State::VERTEX);
            }

            program->bind();
            program->set_uniform("MVP", mvp)
                    ->set_uniform("MANIP", m)
                    ->set_uniform("per_vertex_color", true)
                    ->set_uniform("clippingPlaneEnabled", false)
                    ->set_uniform("selected", false)
                    ->set_uniform("highlight_color", setting::highlight_color);

            drawable->gl_draw();
            program->release();
        }


        void draw_box_wire(LinesDrawable *drawable, const mat4 &mvp, const mat4 &m, bool abstracted) {
            if (!drawable)
                return;

            ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
                program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
            }
            if (!program)
                return;

            if (drawable->vertex_buffer() == 0) {
                std::vector<vec3> points, colors;
                create_box(points, colors, abstracted);
                drawable->update_vertex_buffer(points);
                drawable->update_color_buffer(colors);
            }

            program->bind();
            program->set_uniform("MVP", mvp)
                    ->set_uniform("MANIP", m)
                    ->set_uniform("per_vertex_color", true)
                    ->set_uniform("clippingPlaneEnabled", false)
                    ->set_uniform("selected", false)
                    ->set_uniform("highlight_color", setting::highlight_color);

            drawable->gl_draw();
            program->release();
        }


        // ---------------------------------------------------------------------------

    }
}
