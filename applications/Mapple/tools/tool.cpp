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


#include <tools/tool.h>
#include <tools/tool_manager.h>
#include <tools/canvas.h>

#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/opengl.h>


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

        void MultiTool::move(ToolButton button, int x, int y) {
            auto pos = tools_.find(button);
            if (pos != tools_.end()) {
                if (button == NO_BUTTON)
                    pos->second->move(x, y);
                else
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
            auto w = rect.width();
            auto h = rect.height();
            if (w <= 0 || h <=0)
                return;

            int width = tool_manager()->viewer()->camera()->screenWidth();
            int height = tool_manager()->viewer()->camera()->screenHeight();

            // draw the boundary of the rect
            shape::draw_quad_wire(rect, vec4(1.0f, 0.0f, 0.0f, 1.0f), width, height, -1.0f);

            // draw the transparent face
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            shape::draw_quad_filled(rect, vec4(1.0f, 0.0f, 0.0f, 0.2f), width, height, -0.9f);
            glDisable(GL_BLEND);
        }


        void MultiTool::draw_lasso(const Polygon2& lasso) const {
            int width = tool_manager()->viewer()->camera()->screenWidth();
            int height = tool_manager()->viewer()->camera()->screenHeight();

            // draw the boundary of the rect
            shape::draw_polygon_wire(lasso, vec4(1.0f, 0.0f, 0.0f, 1.0f), width, height, -1.0f);

            // draw the transparent face
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            shape::draw_polygon_filled(lasso, vec4(1.0f, 0.0f, 0.0f, 0.2f), width, height, -0.9f);
            glDisable(GL_BLEND);
        }

    }

}
