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


#include <tools/tool_segmentation_surface_mesh.h>
#include <tools/tool_manager.h>
#include <tools/canvas.h>

#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        MultitoolSurfaceMeshFaceSegmentation::MultitoolSurfaceMeshFaceSegmentation(ToolManager *mgr)
                : MultiTool(mgr)
        {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolSurfaceMeshSegmentationMerge(mgr, picker_));
            set_tool(RIGHT_BUTTON, new ToolSurfaceMeshSegmentationSplit(mgr, picker_));
        }


        MultitoolSurfaceMeshFaceSegmentation::~MultitoolSurfaceMeshFaceSegmentation() {
            delete picker_;
        }


        void MultitoolSurfaceMeshFaceSegmentation::prepare_hint(ToolButton button, int x, int y) {
//            clear_hint();
//            if (button == NO_BUTTON && picker_) {
//                SurfaceMesh::Face face;
//                auto *mesh = dynamic_cast<ToolSurfaceMeshFaceSelectionClick *>(get_tool(LEFT_BUTTON))->multiple_pick(face, x, y);
//                if (mesh && face.is_valid()) {
//                    auto triangle_range = mesh->face_property<std::pair<int, int> >("f:triangle_range");
//                    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
//                    drawable->set_highlight(true);
//                    drawable->set_highlight_range(triangle_range[face]);
//                }
//            }
        }

        void MultitoolSurfaceMeshFaceSegmentation::clear_hint() {
            for (auto model : tool_manager()->viewer()->models()) {
                auto mesh = dynamic_cast<SurfaceMesh *>(model);
                if (mesh) {
                    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
                    drawable->set_highlight(false);
                    drawable->set_highlight_range({-1, -1});
                }
            }
        }

    }

}