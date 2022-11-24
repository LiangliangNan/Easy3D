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

#ifndef EASY3D_GUI_TOOL_SELECTION_SURFACE_MESH_H
#define EASY3D_GUI_TOOL_SELECTION_SURFACE_MESH_H


#include <tools/tool.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class SurfaceMesh;
    class ModelPicker;
    class SurfaceMeshPicker;

    namespace tools {

        class ToolManager;

        class ToolSurfaceMeshFaceSelection : public Tool {
        public:
            explicit ToolSurfaceMeshFaceSelection(ToolManager *mgr);
            ~ToolSurfaceMeshFaceSelection() override = default;

            void update_render_buffer(SurfaceMesh* mesh) const;
        };

        // -------------------- Click Select ----------------------

        class ToolSurfaceMeshFaceSelectionClick : public ToolSurfaceMeshFaceSelection {
        public:
            ToolSurfaceMeshFaceSelectionClick(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode = SM_SELECT);
            ~ToolSurfaceMeshFaceSelectionClick() override;

            void press(int x, int y) override;

		    SurfaceMesh* multiple_pick(SurfaceMesh::Face& f, int x, int y);

        protected:
            ModelPicker *model_picker_;
            SurfaceMeshPicker *picker_;
            SelectMode select_mode_;
        };

        // -------------------- Rect Select ----------------------

        class ToolSurfaceMeshFaceSelectionRect : public ToolSurfaceMeshFaceSelection {
        public:
            ToolSurfaceMeshFaceSelectionRect(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode = SM_SELECT);

            void press(int x, int y) override;
            void drag(int x, int y) override;
            void release(int x, int y) override;

        protected:
            SurfaceMeshPicker *picker_;
            SelectMode select_mode_;
            vec2 start_;
        };

        // -------------------- Lasso Select ----------------------

        class ToolSurfaceMeshFaceSelectionLasso : public ToolSurfaceMeshFaceSelection {
        public:
            ToolSurfaceMeshFaceSelectionLasso(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode = SM_SELECT);

            void press(int x, int y) override;
            void drag(int x, int y) override;
            void release(int x, int y) override;

        protected:
            SurfaceMeshPicker *picker_;
            SelectMode select_mode_;
            Polygon2 lasso_;
        };


        //_____________________________________________________

        // (message, "btn1: select point; btn3: deselect point");
        class MultitoolSurfaceMeshFaceSelectionClick : public MultiTool {
        public:
            explicit MultitoolSurfaceMeshFaceSelectionClick(ToolManager *mgr);
            ~MultitoolSurfaceMeshFaceSelectionClick() override;

            void prepare_hint(ToolButton button, int x, int y) override;
            void clear_hint() override;
            void draw_hint() const override {} //I will draw the highlighted face using shader

            std::string instruction() const override {
                return "Left: select; Right: deselect";
            }

        protected:
            SurfaceMeshPicker *picker_;
        };


        // (message, "btn1: select point; btn3: deselect point");
        class MultitoolSurfaceMeshFaceSelectionRect : public MultiTool {
        public:
            explicit MultitoolSurfaceMeshFaceSelectionRect(ToolManager *mgr);
            ~MultitoolSurfaceMeshFaceSelectionRect() override;

            void press(ToolButton button, int x, int y) override;
            void release(ToolButton button, int x, int y) override;
            void prepare_hint(ToolButton button, int x, int y) override;
            void clear_hint() override;
            void draw_hint() const override;

            std::string instruction() const override{
                return "Left: select; Right: deselect";
            }

        protected:
            SurfaceMeshPicker *picker_;
            vec2 start_, end_;
        };


        // (message, "btn1: select point; btn3: deselect point");
        class MultitoolSurfaceMeshFaceSelectionLasso : public MultiTool {
        public:
            explicit MultitoolSurfaceMeshFaceSelectionLasso(ToolManager *mgr);
            ~MultitoolSurfaceMeshFaceSelectionLasso() override;

            void press(ToolButton button, int x, int y) override;
            void prepare_hint(ToolButton button, int x, int y) override;
            void clear_hint() override;
            void draw_hint() const override;

            std::string instruction() const override {
                return "Left: select; Right: deselect";
            }

        protected:
            SurfaceMeshPicker *picker_;
            Polygon2 lasso_;
        };

    }
}

#endif  // EASY3D_GUI_TOOL_SELECTION_SURFACE_MESH_H
