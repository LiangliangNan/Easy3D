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


#include <easy3d/gui/tool.h>


namespace easy3d {

    namespace tools {

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

        void MultiTool::set_tool(ToolButton button, TaskTool *tool) {
            tools_[button] = tool;
        }

        TaskTool *MultiTool::get_tool(ToolButton button) {
            return tools_[button];
        }

    }

}
