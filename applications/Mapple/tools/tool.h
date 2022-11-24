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

#ifndef EASY3D_GUI_TOOL_H
#define EASY3D_GUI_TOOL_H


#include <map>
#include <string>

#include <easy3d/core/types.h>


namespace easy3d {

    namespace tools {

        enum ToolButton {
            LEFT_BUTTON,
            MIDDLE_BUTTON,
            RIGHT_BUTTON,
            NO_BUTTON
        };

        enum SelectMode {
            SM_SELECT,
            SM_DESELECT
        };

        class ToolManager;

        class Tool {
        public:
            explicit Tool(ToolManager *mgr) : tool_manager_(mgr) {}
            virtual ~Tool() = default;

            ToolManager *tool_manager() const { return tool_manager_; }

            virtual void press(int x, int y) {}
            virtual void move(int x, int y) {}  // for dragging
            virtual void drag(int x, int y) {}  // for free moving
            virtual void release(int x, int y) {}
            virtual void reset() {}

        protected:
            ToolManager *tool_manager_;

            friend class ToolManager;
        };


        //___________________________________________________________________

        class MultiTool {
        public:
            explicit MultiTool(ToolManager *mgr) : tool_manager_(mgr) {}
            virtual ~MultiTool();

            ToolManager *tool_manager() const { return tool_manager_; }

            virtual void press(ToolButton button, int x, int y);
            virtual void move(ToolButton button, int x, int y);
            virtual void release(ToolButton button, int x, int y);
            virtual void reset();

            virtual std::string instruction() const = 0;

            // ------------- visual hint -----------
            // for visual hint during user interaction
            virtual void prepare_hint(ToolButton button, int x, int y) = 0;
            virtual void clear_hint() = 0;
            virtual void draw_hint() const = 0;

        protected:

            void set_tool(ToolButton button, Tool *tool);
            Tool *get_tool(ToolButton button);

            void draw_rect(const Rect& rect) const;
            void draw_lasso(const Polygon2& lasso) const;

        protected:
            ToolManager *tool_manager_;

            typedef std::map<ToolButton, Tool *> ToolMap;
            ToolMap tools_;

            friend class ToolManager;

        };

    }

}

#endif // EASY3D_GUI_TOOL_H
