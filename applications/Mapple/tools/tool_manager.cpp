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


#include <tools/tool_manager.h>
#include <tools/tool_selection_point_cloud.h>
#include <tools/tool_selection_surface_mesh.h>

#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        ToolManager::ToolManager(Canvas* cvs)
                : viewer_(cvs)
                , current_tool_(nullptr)
                , current_tool_name_(EMPTY_TOOL)
                , last_tool_(nullptr)
                , last_tool_name_(EMPTY_TOOL)
        {
        }


        ToolManager::~ToolManager() {
            tools_.clear();
        }


        MultiTool *ToolManager::create_new_tool(ToolName name) {
            MultiTool *tool = nullptr;
            switch (name) {
                //------------------- Surface Mesh Face Selection Tools ---------------------
                case SELECT_SURFACE_MESH_FACE_CLICK_TOOL:
                    tool = new MultitoolSurfaceMeshFaceSelectionClick(this);
                    break;
                case SELECT_SURFACE_MESH_FACE_RECT_TOOL:
                    tool = new MultitoolSurfaceMeshFaceSelectionRect(this);
                    break;
                case SELECT_SURFACE_MESH_FACE_LASSO_TOOL:
                    tool = new MultitoolSurfaceMeshFaceSelectionLasso(this);
                    break;

                    //------------------- Point Cloud Vertices Select Tools ---------------------
                case SELECT_POINT_CLOUD_CLICK_TOOL:
                    tool = new MultitoolPointCloudSelectionClick(this);
                    break;
                case SELECT_POINT_CLOUD_RECT_TOOL:
                    tool = new MultitoolPointCloudSelectionRect(this);
                    break;
                case SELECT_POINT_CLOUD_LASSO_TOOL:
                    tool = new MultitoolPointCloudSelectionLasso(this);
                    break;

                default:
                    break;
            }

            return tool;
        }


        void ToolManager::switch_mode() {
            if (current_tool_ == nullptr && last_tool_ != nullptr) {
                current_tool_ = last_tool_;
                current_tool_name_ = last_tool_name_;
                last_tool_ = nullptr;
                last_tool_name_ = EMPTY_TOOL;
            } else if (current_tool_ != nullptr) {
                last_tool_ = current_tool_;
                last_tool_name_ = current_tool_name_;
                last_tool_->clear_hint();
                current_tool_ = nullptr;
                current_tool_name_ = EMPTY_TOOL;
            }

            if (current_tool_)
                std::cout << current_tool_->instruction() << std::endl;
        }


        void ToolManager::set_tool(ToolName name) {
            auto it = tools_.find(name);
            if (it == tools_.end() && name != EMPTY_TOOL) {
                MultiTool *tool = create_new_tool(name);
                if (tool)
                    tools_[name] = tool;
            }

            it = tools_.find(name);
            if (it == tools_.end()) {
                last_tool_ = current_tool_;
                last_tool_name_ = current_tool_name_;
                current_tool_ = nullptr;
                current_tool_name_ = EMPTY_TOOL;
            } else {
                last_tool_ = current_tool_;
                last_tool_name_ = current_tool_name_;
                current_tool_ = it->second;
                current_tool_name_ = it->first;
            }

            if (last_tool_)
                last_tool_->clear_hint();
        }


        void ToolManager::press(ToolButton button, int x, int y) {
            if (current_tool_ != nullptr) {
                current_tool_->clear_hint();
                current_tool_->press(button, x, y);
            }
        }


        void ToolManager::release(ToolButton button, int x, int y) {
            if (current_tool_ != nullptr) {
                current_tool_->clear_hint();
                current_tool_->release(button, x, y);
            }
        }


        void ToolManager::move(ToolButton button, int x, int y) {
            if (current_tool_ != nullptr) {
                current_tool_->move(button, x, y);
                current_tool_->prepare_hint(button, x, y);
            }
        }


        void ToolManager::draw_hint() const {
            if (current_tool_)
                current_tool_->draw_hint();
        }


        void ToolManager::clear_hint() {
            if (current_tool_)
                current_tool_->clear_hint();
        }

    }

}
