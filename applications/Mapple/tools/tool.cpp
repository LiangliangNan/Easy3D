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


#include "tools/tool.h"
#include "tools/tool_manager.h"
#include "tools/canvas.h"

#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/tessellator.h>


namespace easy3d {

    namespace tools {

        MultiTool::~MultiTool() {
            for (auto tool : tools_)
                delete tool.second;
            tools_.clear();
        }

        void MultiTool::press(ToolButton button, int x, int y) {
            auto pos = tools_.find(button);
            if (pos != tools_.end()) {
                pos->second->press(x, y);
            }
        }

        void MultiTool::drag(ToolButton button, int x, int y) {
            auto pos = tools_.find(button);
            if (pos != tools_.end()) {
                pos->second->drag(x, y);
            }
        }

        void MultiTool::release(ToolButton button, int x, int y) {
            auto pos = tools_.find(button);
            if (pos != tools_.end()) {
                pos->second->release(x, y);
            }
        }

        void MultiTool::reset() {
            auto pos = tools_.find(LEFT_BUTTON);
            if (pos != tools_.end()) {
                pos->second->reset();
            }

            pos = tools_.find(RIGHT_BUTTON);
            if (pos != tools_.end()) {
                pos->second->reset();
            }
        }

        void MultiTool::set_tool(ToolButton button, Tool *tool) {
            tools_[button] = tool;
        }

        Tool *MultiTool::get_tool(ToolButton button) {
            return tools_[button];
        }

        void MultiTool::draw_rect(const Rect& rect) const {
            int w = rect.width();
            int h = rect.height();
            if (w <= 0 || h <=0)
                return;

            int width = tool_manager()->viewer()->camera()->screenWidth();
            int height = tool_manager()->viewer()->camera()->screenHeight();

            // draw the boundary of the rect
            opengl::draw_quad_wire(rect, vec4(1.0f, 0.0f, 0.0f, 1.0f), width, height, -1.0f);

            // draw the transparent face
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            opengl::draw_quad_filled(rect, vec4(1.0f, 0.0f, 0.0f, 0.2f), width, height, -0.9f);
            glDisable(GL_BLEND);
        }


        void MultiTool::draw_lasso(const Polygon2& lasso) const {
            const std::string name = "screen_space/screen_space_color";
            auto program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program || lasso.size() < 3)
                return;

            int width = tool_manager()->viewer()->camera()->screenWidth();
            int height = tool_manager()->viewer()->camera()->screenHeight();

            std::vector<vec3> wire_points(lasso.size());
            std::vector<unsigned int> wire_indices(lasso.size() * 2);
            for (std::size_t i = 0; i < lasso.size(); ++i) {
                const auto& p = lasso[i];
                // to use the screen space shaders, I need to convert the point coordinates into the NDC space.
                // also have to follow the OpenGL coordinates rule.
                wire_points[i].x = {2.0f * p.x / width - 1.0f};
                wire_points[i].y = {2.0f * (height - p.y - 1)/ height - 1.0f};
                wire_points[i].z = 0.0f;

                wire_indices[i*2] = i;
                wire_indices[i*2+1] = (i+ 1) % lasso.size();
            }

            LinesDrawable drawable;
            drawable.update_vertex_buffer(wire_points);                                  easy3d_debug_log_gl_error;
            drawable.update_index_buffer(wire_indices);                                  easy3d_debug_log_gl_error;

            program->bind();                                                        easy3d_debug_log_gl_error;
            program->set_uniform("screen_color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
            drawable.gl_draw(false);                                                easy3d_debug_log_gl_error;
            program->release();                                                     easy3d_debug_log_gl_error;

            // draw the face
            Tessellator tess;
            tess.begin_polygon(vec3(0, 0, 1));
            tess.begin_contour();
            for (const auto& v : wire_points)
                tess.add_vertex(v);
            tess.end_contour();
            tess.end_polygon();
            const auto& vts = tess.vertices();
            std::vector<vec3> face_points(vts.size());
            for (std::size_t i=0; i<vts.size(); ++i)
                face_points[i] = vec3(vts[i]->data());
            const std::vector<unsigned int>& face_indices = tess.indices();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            TrianglesDrawable face;
            face.update_vertex_buffer(face_points);
            face.update_index_buffer(face_indices);
            program->bind();                                                        easy3d_debug_log_gl_error;
            program->set_uniform("screen_color", vec4(1.0f, 0.0f, 0.0f, 0.2f));
            face.gl_draw(false);                                                easy3d_debug_log_gl_error;
            program->release();
            glDisable(GL_BLEND);    easy3d_debug_log_gl_error;
        }

    }

}
