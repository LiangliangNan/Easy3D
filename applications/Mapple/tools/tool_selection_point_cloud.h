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

#ifndef EASY3D_GUI_TOOL_SELECTION_POINT_CLOUD_H
#define EASY3D_GUI_TOOL_SELECTION_POINT_CLOUD_H

#include "tools/tool.h"
#include <easy3d/core/types.h>


namespace easy3d {

    class PointCloud;
    class PointCloudPicker;
    
    namespace tools {

        class ToolManager;

        class ToolPointCloudSelection : public Tool {
        public:
            ToolPointCloudSelection(ToolManager *mgr);
            virtual ~ToolPointCloudSelection() {}
            std::string name() const { return "PointCloudSelect"; }
        };
        

        // -------------------- Rect Select ----------------------

        class ToolPointCloudSelectionRect : public ToolPointCloudSelection {
        public:
            ToolPointCloudSelectionRect(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode = SM_SELECT);
            virtual void press(int x, int y);
            virtual void drag(int x, int y);
            virtual void release(int x, int y);

        protected:
            PointCloudPicker *picker_;
            SelectMode select_mode_;
            vec2 start_;
        };

        // -------------------- Lasso Select ----------------------

        class ToolPointCloudSelectionLasso : public ToolPointCloudSelection {
        public:
            ToolPointCloudSelectionLasso(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode = SM_SELECT);
            virtual void press(int x, int y);
            virtual void drag(int x, int y);
            virtual void release(int x, int y);

        protected:
            PointCloudPicker *picker_;
            SelectMode select_mode_;
            Polygon2 lasso_;
        };


        //_____________________________________________________

        // (message, "btn1: select point; btn3: deselect point");
        class MultitoolPointCloudSelectionRect : public MultiTool {
        public:
            MultitoolPointCloudSelectionRect(ToolManager *mgr);
            ~MultitoolPointCloudSelectionRect();

            virtual void set_hint() {} // point is the only thing to show
            void press(ToolButton button, int x, int y);
            void prepare_hint(ToolButton button, int x, int y);
            void clear_hint();
            void draw_hint() const;

            std::string instruction() const {
                return "Left: select; Right: deselect";
            }

        protected:
            PointCloudPicker *picker_;
            vec2 start_, end_;
        };


        // (message, "btn1: select point; btn3: deselect point");
        class MultitoolPointCloudSelectionLasso : public MultiTool {
        public:
            MultitoolPointCloudSelectionLasso(ToolManager *mgr);
            ~MultitoolPointCloudSelectionLasso();

            virtual void set_hint() {} // point is the only thing to show

            void press(ToolButton button, int x, int y);
            void prepare_hint(ToolButton button, int x, int y);
            void clear_hint();
            void draw_hint() const;

            std::string instruction() const {
                return "Left: select; Right: deselect";
            }

        protected:
            PointCloudPicker *picker_;
            Polygon2 lasso_;
        };

    }

}

#endif  // EASY3D_GUI_TOOL_SELECTION_POINT_CLOUD_H
