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

#ifndef EASY3D_GUI_TOOL_MANAGER_H
#define EASY3D_GUI_TOOL_MANAGER_H

#include <tools/tool.h>

#include <map>


namespace easy3d {

    class Canvas;

    namespace tools {

        class MultiTool;

        class ToolManager {
        public:
            enum ToolName {
                //------------------- Surface Mesh Face Selection Tools ---------------------
                SELECT_SURFACE_MESH_FACE_CLICK_TOOL,
                SELECT_SURFACE_MESH_FACE_RECT_TOOL,
                SELECT_SURFACE_MESH_FACE_LASSO_TOOL,

                //------------------- Point Cloud Vertices Select Tools ---------------------
                SELECT_POINT_CLOUD_CLICK_TOOL,
                SELECT_POINT_CLOUD_LASSO_TOOL,
                SELECT_POINT_CLOUD_RECT_TOOL,

                //------------------------------ empty tool ---------------------------------
                EMPTY_TOOL
            };

        public:
            explicit ToolManager(Canvas *cvs);
            virtual ~ToolManager();

            void set_tool(ToolName name);

            MultiTool *current_tool() { return current_tool_; }
            ToolName current_tool_name() const { return current_tool_name_; }

            void press(ToolButton button, int x, int y);
            void move(ToolButton button, int x, int y);
            void release(ToolButton button, int x, int y);

            // witch between tool function and scene manipulation
            void switch_mode();

            void draw_hint() const;
            void clear_hint();

            Canvas* viewer() const { return viewer_; }

        protected:
            virtual MultiTool *create_new_tool(ToolName name);

        private:
            Canvas *viewer_;

            MultiTool *current_tool_;
            ToolName current_tool_name_;

            MultiTool *last_tool_;
            ToolName last_tool_name_;

            typedef std::map<ToolName, MultiTool *> ToolMap;
            ToolMap tools_;
        };

    }

}


#endif  // EASY3D_GUI_TOOL_MANAGER_H
