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


#include <tools/tool_selection_point_cloud.h>
#include <tools/tool_manager.h>
#include <tools/canvas.h>

#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/buffers.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        // -------------------- ToolPointCloudSelection ----------------------


        ToolPointCloudSelection::ToolPointCloudSelection(ToolManager *mgr)
                : Tool(mgr) {
        }


        void ToolPointCloudSelection::update_render_buffer(PointCloud* cloud) const {
            auto d = cloud->renderer()->get_points_drawable("vertices");
            if ((d->coloring_method() != easy3d::State::SCALAR_FIELD) || (d->property_location() != State::VERTEX) || (d->property_name() != "v:select")) {
                if (!cloud->get_vertex_property<bool>("v:select"))
                    cloud->add_vertex_property<bool>("v:select", false);
                d->set_coloring(State::SCALAR_FIELD, State::VERTEX, "v:select");
                buffers::update(cloud, d);
            }
            else {
                auto select = cloud->vertex_property<bool>("v:select", false);
                // update the drawable's texcoord buffer
                std::vector<vec2> texcoords(d->num_vertices());
                for (auto v : cloud->vertices())
                    texcoords[v.idx()] = vec2(select[v], 0.5f);
                d->update_texcoord_buffer(texcoords);
                d->set_coloring(State::SCALAR_FIELD, State::VERTEX, "v:select");
            }
        }

        // -------------------- Rect Select ----------------------

        ToolPointCloudSelectionRect::ToolPointCloudSelectionRect(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : ToolPointCloudSelection(mgr), picker_(picker), select_mode_(mode) {
        }


        void ToolPointCloudSelectionRect::press(int x, int y) {
            start_ = vec2(x,y);
        }


        void ToolPointCloudSelectionRect::drag(int x, int y) {
        }


        void ToolPointCloudSelectionRect::release(int x, int y) {
            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model);
                if (cloud && cloud->renderer()->is_visible()) {
                    picker_->pick_vertices(cloud, Rect(start_, vec2(x, y)), select_mode_ != SM_SELECT);
                    update_render_buffer(cloud);
                }
            }
        }


        MultitoolPointCloudSelectionRect::MultitoolPointCloudSelectionRect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());

            set_tool(LEFT_BUTTON, new ToolPointCloudSelectionRect(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolPointCloudSelectionRect(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        MultitoolPointCloudSelectionRect::~MultitoolPointCloudSelectionRect() {
            delete picker_;
        }


        void MultitoolPointCloudSelectionRect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            start_ = vec2(x, y);
            end_ = vec2(x, y);
        }


        void MultitoolPointCloudSelectionRect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON && picker_) {
                end_ = vec2(x, y);
            }
        }

        void MultitoolPointCloudSelectionRect::clear_hint() {
            start_ = vec2(-1, -1);
            end_ = vec2(-1, -1);
        }

        void MultitoolPointCloudSelectionRect::draw_hint() const {
            draw_rect(Rect(start_, end_));
        }

        // ------------------ Lasso Select -----------------------

        ToolPointCloudSelectionLasso::ToolPointCloudSelectionLasso(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : ToolPointCloudSelection(mgr), picker_(picker), select_mode_(mode) {
        }

        void ToolPointCloudSelectionLasso::press(int x, int y) {
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void ToolPointCloudSelectionLasso::drag(int x, int y) {
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void ToolPointCloudSelectionLasso::release(int x, int y) {
            if (lasso_.size() < 3)
                return;

            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model);
                if (cloud && cloud->renderer()->is_visible()) {
                    picker_->pick_vertices(cloud, lasso_, select_mode_ != SM_SELECT);
                    update_render_buffer(cloud);
                }
            }
            lasso_.clear();
        }


        MultitoolPointCloudSelectionLasso::MultitoolPointCloudSelectionLasso(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolPointCloudSelectionLasso(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolPointCloudSelectionLasso(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        MultitoolPointCloudSelectionLasso::~MultitoolPointCloudSelectionLasso() {
            delete picker_;
        }


        void MultitoolPointCloudSelectionLasso::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void MultitoolPointCloudSelectionLasso::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON)
                lasso_.push_back(vec2(float(x), float(y)));
        }

        void MultitoolPointCloudSelectionLasso::clear_hint() {
            lasso_.clear();
        }

        void MultitoolPointCloudSelectionLasso::draw_hint() const {
            draw_lasso(lasso_);
        }

    }

}
