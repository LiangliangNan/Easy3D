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

#ifndef EASY3D_GUI_TOOL_SEGMENTATION_SURFACE_MESH_H
#define EASY3D_GUI_TOOL_SEGMENTATION_SURFACE_MESH_H


#include <tools/tool.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class SurfaceMesh;
    class SurfaceMeshPicker;

    namespace tools {

        class ToolManager;

        class ToolSurfaceMeshSegmentation : public Tool {
        public:
            ToolSurfaceMeshSegmentation(ToolManager *mgr, SurfaceMeshPicker *picker) : Tool(mgr), picker_(picker) {
            }
            virtual ~ToolSurfaceMeshSegmentation() {}

            virtual void press(int x, int y) {}
            virtual void drag(int x, int y) {}
            virtual void release(int x, int y) {}

        protected:
            SurfaceMeshPicker *picker_;
        };


        // -------------------- Merge two patches ----------------------
        class ToolSurfaceMeshSegmentationMerge : public ToolSurfaceMeshSegmentation {
        public:
            ToolSurfaceMeshSegmentationMerge(ToolManager *mgr, SurfaceMeshPicker *picker) : ToolSurfaceMeshSegmentation(mgr, picker) {}
        };

        // -------------------- Split a patch ----------------------

        class ToolSurfaceMeshSegmentationSplit : public ToolSurfaceMeshSegmentation {
        public:
            ToolSurfaceMeshSegmentationSplit(ToolManager *mgr, SurfaceMeshPicker *picker): ToolSurfaceMeshSegmentation(mgr, picker) {}
        };

        //_____________________________________________________

        // (message, "Left: merge patches; Right: split patch");
        class MultitoolSurfaceMeshFaceSegmentation : public MultiTool {
        public:
            MultitoolSurfaceMeshFaceSegmentation(ToolManager *mgr);
            ~MultitoolSurfaceMeshFaceSegmentation();

            void press(ToolButton button, int x, int y) {}
            void prepare_hint(ToolButton button, int x, int y);
            void clear_hint();
            void draw_hint() const {}

            std::string instruction() const {
                return "Left: merge patches; Right: split patch";
            }

        protected:
            SurfaceMeshPicker *picker_;
        };
    }
}

#endif  // EASY3D_GUI_TOOL_SEGMENTATION_SURFACE_MESH_H
