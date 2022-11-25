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

        void create_grid(int x_steps, int y_steps, std::vector<vec3> &points, float depth, float scale) {
            // Horizontal lines
            float x = scale * 0.5f * static_cast<float>(x_steps - 1);
            float y = -scale * 0.5f * static_cast<float>(y_steps - 1);
            for (int i = 0; i < y_steps; i++) {
                points.emplace_back(vec3(-x, y, depth));
                points.emplace_back(vec3(x, y, depth));
                y += scale;
            }

            // Vertical lines
            x = -scale * 0.5f * (float) (x_steps - 1);
            y = scale * 0.5f * (float) (y_steps - 1);
            for (int i = 0; i < x_steps; i++) {
                points.emplace_back(vec3(x, -y, depth));
                points.emplace_back(vec3(x, y, depth));
                x += scale;
            }
        }


        void create_box(std::vector<vec3> &points, std::vector<vec3> &colors, bool abstracted) {
            points.clear();
            colors.clear();

            const float min_coord = -0.5f;
            const float max_coord = 0.5f;
            vec3 red(1, 0, 0), green(0, 1, 0), blue(0, 0, 1);

            std::vector<vec3> vts = {
                    vec3(min_coord, min_coord, min_coord), // 0
                    vec3(max_coord, min_coord, min_coord), // 1
                    vec3(max_coord, max_coord, min_coord), // 2
                    vec3(min_coord, max_coord, min_coord), // 3
                    vec3(min_coord, min_coord, max_coord), // 4
                    vec3(max_coord, min_coord, max_coord), // 5
                    vec3(max_coord, max_coord, max_coord), // 6
                    vec3(min_coord, max_coord, max_coord)  // 7
            };

            if (abstracted) {
                const float ratio = 0.2f;
                points = {
                        vts[0], vts[0] + red * ratio, vts[1], vts[1] - red * ratio, vts[1], vts[1] + green * ratio,
                        vts[2], vts[2] - green * ratio, vts[2], vts[2] - red * ratio, vts[3], vts[3] + red * ratio,
                        vts[3], vts[3] - green * ratio, vts[0], vts[0] + green * ratio, vts[4], vts[4] + red * ratio,
                        vts[5], vts[5] - red * ratio, vts[5], vts[5] + green * ratio, vts[6], vts[6] - green * ratio,
                        vts[6], vts[6] - red * ratio, vts[7], vts[7] + red * ratio, vts[7], vts[7] - green * ratio,
                        vts[4], vts[4] + green * ratio, vts[0], vts[0] + blue * ratio, vts[1], vts[1] + blue * ratio,
                        vts[2], vts[2] + blue * ratio, vts[3], vts[3] + blue * ratio, vts[4], vts[4] - blue * ratio,
                        vts[5], vts[5] - blue * ratio, vts[6], vts[6] - blue * ratio, vts[7], vts[7] - blue * ratio
                };
                colors = {
                        red, red, red, red, green, green, green, green, red, red, red, red, green, green, green, green,
                        red, red, red, red, green, green, green, green, red, red, red, red, green, green, green, green,
                        blue, blue, blue, blue, blue, blue, blue, blue, blue, blue, blue, blue, blue, blue, blue, blue
                };
            } else {
                points = {
                        vts[0], vts[1], vts[1], vts[2], vts[2], vts[3], vts[3], vts[0],
                        vts[4], vts[5], vts[5], vts[6], vts[6], vts[7], vts[7], vts[4],
                        vts[0], vts[4], vts[1], vts[5], vts[2], vts[6], vts[3], vts[7]
                };
                colors = {
                        red, red, green, green, red, red, green, green,
                        red, red, green, green, red, red, green, green,
                        blue, blue, blue, blue, blue, blue, blue, blue
                };
            }
        }


        void create_circle(int slices, std::vector<vec3> &points, std::vector<unsigned int> &indices) {
            points.clear();
            indices.clear();

            // Compute vertex position of the body
            const auto step_theta = static_cast<float>(2.0 * M_PI / slices);
            for (int i = 0; i < slices; ++i) {
                const float angle = static_cast<float>(i) * step_theta;
                const float x = std::cos(angle);
                const float y = std::sin(angle);
                points.emplace_back(vec3(x, y, 0.f));

                indices.push_back(i);
                indices.push_back((i + 1) % slices);
            }
        }


        void create_sphere(
                const vec3 &center, double radius, int slices, int stacks, const vec3 &color,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors) {
            create_checker_sphere(center, radius, slices, stacks, 1, color, color, points, normals, colors);
        }


        void create_checker_sphere(
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
                    fan.emplace_back(vec3(static_cast<float>(x11), static_cast<float>(y11), static_cast<float>(z11)));
                    if (v != 0)
                        fan.emplace_back(vec3(static_cast<float>(x21), static_cast<float>(y21), static_cast<float>(z21)));
                    fan.emplace_back(vec3(static_cast<float>(x22), static_cast<float>(y22), static_cast<float>(z22)));
                    if (v != stacks - 1)
                        fan.emplace_back(vec3(static_cast<float>(x12), static_cast<float>(y12), static_cast<float>(z12)));

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


        void create_cylinder(double radius, int slices, const vec3 &s, const vec3 &t, const vec3 &color,
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


        void create_cone(double radius, int slices, const vec3 &s, const vec3 &t, const vec3 &color,
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


        void create_torus(double major_radius, double minor_radius, int major_slices, int minor_slices,
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
                        double x = (major_radius + minor_radius * std::cos(s * twopi / minor_slices)) *
                                std::cos(t * twopi / major_slices);
                        double y = minor_radius * std::sin(s * twopi / minor_slices);
                        double z = (major_radius + minor_radius * cos(s * twopi / minor_slices)) *
                                std::sin(t * twopi / major_slices);

                        // Calculate surface normal
                        double nx = x - major_radius * std::cos(t * twopi / major_slices);
                        double ny = y;
                        double nz = z - major_radius * std::sin(t * twopi / major_slices);
                        vec3 n(static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz));
                        n.normalize();

                        tmp_pts.emplace_back(vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
                        tmp_nms.push_back(n);

                        if (tmp_pts.size() <= 3) {
                            points.emplace_back(vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
                            normals.push_back(n);
                        } else {
                            points.emplace_back(vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
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


        void create_camera(std::vector<vec3> &points, float width, float fov, float hw_ratio) {
            const float halfWidth = width * 0.5f;
            const float halfHeight = halfWidth * hw_ratio;
            const auto dist = static_cast<float>(halfHeight / std::tan(fov * 0.5));

            const float arrowHeight = 2.0f * halfHeight;
            const float baseHeight = 1.2f * halfHeight;
            const float arrowHalfWidth = 0.5f * halfWidth;
            const float baseHalfWidth = 0.3f * halfWidth;

            //--------------
            // Frustum outline
            //--------------
            const vec3 c(0.0f, 0.0f, 0.0f);
            const vec3 p0(-halfWidth, -halfHeight, -dist);
            const vec3 p1(halfWidth, -halfHeight, -dist);
            const vec3 p2(halfWidth, halfHeight, -dist);
            const vec3 p3(-halfWidth, halfHeight, -dist);
            points.push_back(p0);
            points.push_back(p1);
            points.push_back(p1);
            points.push_back(p2);
            points.push_back(p2);
            points.push_back(p3);
            points.push_back(p3);
            points.push_back(p0);
            points.push_back(c);
            points.push_back(p0);
            points.push_back(c);
            points.push_back(p1);
            points.push_back(c);
            points.push_back(p2);
            points.push_back(c);
            points.push_back(p3);

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


        void create_camera(std::vector<vec3> &points, std::vector<unsigned int> &indices, float width, float fov,
                           float hw_ratio) {
            const float halfWidth = width * 0.5f;
            const float halfHeight = halfWidth * hw_ratio;
            const auto dist = static_cast<float>(halfHeight / std::tan(fov * 0.5));

            const float arrowHeight = 2.0f * halfHeight;
            const float baseHeight = 1.2f * halfHeight;
            const float arrowHalfWidth = 0.5f * halfWidth;
            const float baseHalfWidth = 0.3f * halfWidth;

            //--------------
            // Frustum outline
            //--------------
            const vec3 c(0.0f, 0.0f, 0.0f);
            const vec3 p0(-halfWidth, -halfHeight, -dist);
            const vec3 p1(halfWidth, -halfHeight, -dist);
            const vec3 p2(halfWidth, halfHeight, -dist);
            const vec3 p3(-halfWidth, halfHeight, -dist);

            points.push_back(c);
            points.push_back(p0);
            points.push_back(p1);
            points.push_back(p2);
            points.push_back(p3);

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
            points.push_back(r2);
            points.push_back(r3);

            // Arrow - TRIANGLE
            const vec3 a0(0.0f, arrowHeight, -dist);
            const vec3 a1(-arrowHalfWidth, baseHeight, -dist);
            const vec3 a2(arrowHalfWidth, baseHeight, -dist);

            points.push_back(a0);
            points.push_back(a1);
            points.push_back(a2);

            indices.insert(indices.end(), {0, 1, 2});
            indices.insert(indices.end(), {0, 2, 3});
            indices.insert(indices.end(), {0, 3, 4});
            indices.insert(indices.end(), {0, 4, 1});

            indices.insert(indices.end(), {5, 6, 7});
            indices.insert(indices.end(), {5, 7, 8});

            indices.insert(indices.end(), {9, 10, 11});
        }
    }

} // namespace easy3d
